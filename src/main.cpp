#include "anaport/options.hpp"
#include "anaport/workspace.hpp"
#include "anaport/cmake_invoker.hpp"
#include "anaport/file_api_parser.hpp"
#include "anaport/compile_commands_parser.hpp"
#include "anaport/cmake_cache_parser.hpp"
#include "anaport/analyzer.hpp"
#include "anaport/report_generator.hpp"
#include "anaport/logger.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main(int argc, char* argv[]) {
    using namespace anaport;

    // ── Parse options ────────────────────────────────────────────────────────
    Options opts;
    try {
        opts = parse_options(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    g_verbose = opts.verbose;

    log_info("=== analyze-portability v1.1.0 ===");
    log_info("Source: " + opts.source_dir.string());
    log_info("Output: " + opts.output_dir.string());

    std::filesystem::path build_dir;
    InvokeResult cmake_result;
    bool cmake_was_invoked = false;

    try {
        // ── Prepare workspace ────────────────────────────────────────────────
        build_dir = prepare_workspace(opts);

        // ── Write File API query before cmake configure ──────────────────────
        write_file_api_query(build_dir);

        // ── Run cmake configure ──────────────────────────────────────────────
        cmake_result = invoke_cmake_configure(opts.source_dir, build_dir, opts.verbose);
        cmake_was_invoked = true;
        if (!cmake_result.success()) {
            log_error("cmake configure failed (exit code " + std::to_string(cmake_result.exit_code) + ")");
            log_error("Analysis will proceed with partial File API data if available.");
        }

        // ── Parse File API reply ─────────────────────────────────────────────
        auto raw_project = parse_file_api(build_dir, opts.verbose);

        // ── Parse additional data sources ────────────────────────────────────
        auto cc_data    = parse_compile_commands(build_dir, opts.verbose);
        auto cache_data = parse_cmake_cache(build_dir, opts.verbose);

        // ── Run portability analysis ─────────────────────────────────────────
        if (raw_project.source_dir.empty()) {
            raw_project.source_dir = opts.source_dir;
        }
        auto result = analyze(raw_project, cc_data, cache_data,
                              build_dir,
                              opts.verbose, opts.include_utility_targets);
        result.source_dir = opts.source_dir;

        // ── Generate reports (format-flag aware) ─────────────────────────────
        write_reports(result, opts.output_dir, opts);

        // ── Print short summary ──────────────────────────────────────────────
        bool write_all = !opts.out_json && !opts.out_markdown && !opts.out_graphviz;

        std::cout << "\n=== Analysis complete ===\n";
        std::cout << "Project: " << result.project_name << "\n";
        std::cout << "Targets: " << result.targets.size() << "\n";
        if (!result.service_targets.empty()) {
            std::cout << "  (" << result.service_targets.size()
                      << " CTest service targets excluded; use --include-utility-targets to see them)\n";
        }

        // Severity filter info
        std::string sev_label;
        switch (opts.min_severity) {
            case SeverityFilter::Info:    sev_label = "info (all)"; break;
            case SeverityFilter::Warning: sev_label = "warning+"; break;
            case SeverityFilter::Error:   sev_label = "error only"; break;
        }

        // Count emitted vs total
        int emitted_errors = 0, emitted_warnings = 0, emitted_infos = 0;
        for (auto& f : result.findings) {
            if (static_cast<int>(f.severity) < static_cast<int>(opts.min_severity)) continue;
            switch (f.severity) {
                case FindingSeverity::Error:   ++emitted_errors;   break;
                case FindingSeverity::Warning: ++emitted_warnings; break;
                case FindingSeverity::Info:    ++emitted_infos;    break;
            }
        }
        int emitted_total = emitted_errors + emitted_warnings + emitted_infos;
        int hidden        = static_cast<int>(result.findings.size()) - emitted_total;

        std::cout << "Findings: " << result.findings.size() << " total"
                  << " (" << result.error_count << " errors, "
                  << result.warning_count << " warnings, "
                  << result.info_count << " info)\n";
        if (opts.min_severity != SeverityFilter::Info) {
            std::cout << "  Emitted (" << sev_label << "): " << emitted_total
                      << " (" << emitted_errors << " errors, "
                      << emitted_warnings << " warnings, "
                      << emitted_infos << " info)"
                      << (hidden > 0 ? " [" + std::to_string(hidden) + " hidden by filter]" : "")
                      << "\n";
        }

        // Data sources status
        std::cout << "Data sources:\n";
        std::cout << "  CMake File API: parsed\n";
        if (result.compile_commands.found) {
            std::cout << "  compile_commands.json: "
                      << (result.compile_commands.parsed
                            ? std::to_string(result.compile_commands.entry_count) + " entries, "
                              + std::to_string(result.compile_commands.observations.size()) + " observations"
                            : "found but not parsed: " + result.compile_commands.parse_error)
                      << "\n";
        } else {
            std::cout << "  compile_commands.json: not found (configure with CMAKE_EXPORT_COMPILE_COMMANDS=ON)\n";
        }
        if (result.cmake_cache.found) {
            std::cout << "  CMakeCache.txt: "
                      << (result.cmake_cache.parsed
                            ? std::to_string(result.cmake_cache.variable_count) + " variables parsed"
                            : "found but not parsed: " + result.cmake_cache.parse_error)
                      << "\n";
        }

        std::cout << "Reports written to: " << opts.output_dir.string() << "\n";
        if (write_all || opts.out_json)
            std::cout << "  report.json       — machine-readable portability analysis\n";
        if (write_all || opts.out_markdown)
            std::cout << "  report.md         — Markdown report\n";
        if (write_all || opts.out_graphviz)
            std::cout << "  dependencies.dot  — Graphviz dependency graph\n";

        // ── Cleanup ──────────────────────────────────────────────────────────
        cleanup_build_dir(build_dir, opts.keep_build_dir);

    } catch (const std::exception& e) {
        // ── If cmake configure failed and File API reply is absent, write
        //    diagnostic failure reports instead of just exiting. ──────────────
        if (cmake_was_invoked && !cmake_result.success() && !build_dir.empty()) {
            log_warn("File API reply unavailable after configure failure — generating diagnostic failure report.");
            try {
                std::filesystem::create_directories(opts.output_dir);
                FailureContext ctx;
                ctx.source_dir           = opts.source_dir;
                ctx.build_dir            = build_dir;
                ctx.configure_exit_code  = cmake_result.exit_code;
                ctx.error_message        = e.what();
                // Try to gather partial data sources even after configure failure
                try { ctx.compile_commands = parse_compile_commands(build_dir, false); } catch (...) {}
                try { ctx.cmake_cache      = parse_cmake_cache(build_dir, false);      } catch (...) {}
                write_failure_reports(ctx, opts.output_dir, opts);

                bool write_all = !opts.out_json && !opts.out_markdown && !opts.out_graphviz;
                std::cout << "\n=== Configure failed — diagnostic failure report written ===\n";
                std::cout << "configure exit code: " << cmake_result.exit_code << "\n";
                std::cout << "error: " << e.what() << "\n";
                std::cout << "Reports written to: " << opts.output_dir.string() << "\n";
                if (write_all || opts.out_json)
                    std::cout << "  report.json       — configure failure diagnostic\n";
                if (write_all || opts.out_markdown)
                    std::cout << "  report.md         — configure failure diagnostic\n";
                if (write_all || opts.out_graphviz)
                    std::cout << "  dependencies.dot  — empty graph (no data)\n";
            } catch (const std::exception& write_ex) {
                log_error("Failed to write failure reports: " + std::string(write_ex.what()));
            }
            cleanup_build_dir(build_dir, opts.keep_build_dir);
            return EXIT_FAILURE;
        }
        log_error(std::string(e.what()));
        if (!build_dir.empty()) {
            cleanup_build_dir(build_dir, opts.keep_build_dir);
        }
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
