#include "anaport/report_generator.hpp"
#include "anaport/logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <map>

using json = nlohmann::json;

namespace anaport {

static std::string now_iso8601() {
    auto now = std::chrono::system_clock::now();
    auto tt  = std::chrono::system_clock::to_time_t(now);
    std::tm tm_utc{};
    gmtime_r(&tt, &tm_utc);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm_utc);
    return buf;
}

static bool passes_filter(FindingSeverity sev, SeverityFilter min_sev) {
    return static_cast<int>(sev) >= static_cast<int>(min_sev);
}

/// Count findings that pass the severity filter
static void count_filtered(const AnalysisResult& result, SeverityFilter min_sev,
                             int& errors, int& warnings, int& infos) {
    errors = warnings = infos = 0;
    for (auto& f : result.findings) {
        if (!passes_filter(f.severity, min_sev)) continue;
        switch (f.severity) {
            case FindingSeverity::Error:   ++errors;   break;
            case FindingSeverity::Warning: ++warnings; break;
            case FindingSeverity::Info:    ++infos;    break;
        }
    }
}

// ─── JSON Report ──────────────────────────────────────────────────────────────

void write_json_report(const AnalysisResult& result,
                        const std::filesystem::path& output_dir,
                        SeverityFilter min_severity)
{
    json doc;
    doc["generated_at"]  = now_iso8601();
    doc["tool"]          = "analyze-portability";
    doc["tool_version"]  = "1.1.0";
    doc["project_name"]  = result.project_name;
    doc["source_dir"]    = result.source_dir.empty() ? "" : result.source_dir.string();
    doc["cmake_version"] = result.cmake_version;

    // Service target names
    json service_target_names = json::array();
    for (auto& st : result.service_targets) service_target_names.push_back(st.name);

    // Count filtered findings
    int errors, warnings, infos;
    count_filtered(result, min_severity, errors, warnings, infos);
    int emitted = errors + warnings + infos;

    // Build category and source counts
    std::map<std::string, int> cat_count, src_count;
    for (auto& f : result.findings) {
        if (!passes_filter(f.severity, min_severity)) continue;
        cat_count[category_str(f.category)]++;
        src_count[source_str(f.source)]++;
    }

    json cat_json = json::object();
    for (auto& [k,v] : cat_count) cat_json[k] = v;
    json src_json = json::object();
    for (auto& [k,v] : src_count) src_json[k] = v;

    doc["summary"] = {
        {"total_targets",            result.targets.size()},
        {"hidden_service_targets",   result.service_targets.size()},
        {"total_findings",           result.findings.size()},
        {"emitted_findings",         emitted},
        {"errors",                   errors},
        {"warnings",                 warnings},
        {"info",                     infos},
        {"dependency_edges",         result.edges.size()},
        {"findings_by_category",     cat_json},
        {"findings_by_source",       src_json},
        {"severity_filter",          [&]() -> std::string {
            switch (min_severity) {
                case SeverityFilter::Info:    return "info";
                case SeverityFilter::Warning: return "warning";
                case SeverityFilter::Error:   return "error";
            }
            return "info";
        }()}
    };
    if (!result.service_targets.empty()) {
        doc["summary"]["hidden_service_target_names"] = service_target_names;
    }

    // Additional data sources
    json sources_doc = json::object();
    {
        json cc_doc;
        cc_doc["found"]       = result.compile_commands.found;
        cc_doc["parsed"]      = result.compile_commands.parsed;
        cc_doc["entry_count"] = result.compile_commands.entry_count;
        cc_doc["observations"] = result.compile_commands.observations.size();
        if (!result.compile_commands.parse_error.empty())
            cc_doc["parse_error"] = result.compile_commands.parse_error;
        sources_doc["compile_commands"] = cc_doc;

        json cache_doc;
        cache_doc["found"]          = result.cmake_cache.found;
        cache_doc["parsed"]         = result.cmake_cache.parsed;
        cache_doc["variable_count"] = result.cmake_cache.variable_count;
        cache_doc["interesting_variables"] = result.cmake_cache.variables.size();
        if (!result.cmake_cache.parse_error.empty())
            cache_doc["parse_error"] = result.cmake_cache.parse_error;
        sources_doc["cmake_cache"] = cache_doc;
    }
    doc["data_sources"] = sources_doc;

    // Targets
    json targets_arr = json::array();
    for (auto& t : result.targets) {
        json tj;
        tj["id"]   = t.id;
        tj["name"] = t.name;
        tj["type"] = t.type;
        tj["source_dir"] = t.source_dir.string();

        json artifacts = json::array();
        for (auto& a : t.artifacts) artifacts.push_back(a.path.string());
        tj["artifacts"] = artifacts;

        json sources = json::array();
        for (auto& s : t.source_files) sources.push_back(s);
        tj["sources"] = sources;

        json cgs = json::array();
        for (auto& cg : t.compile_groups) {
            json cgj;
            cgj["language"] = cg.language;
            json incs = json::array();
            for (auto& inc : cg.includes) {
                incs.push_back({{"path", inc.path.string()}, {"is_system", inc.is_system}});
            }
            cgj["includes"] = incs;
            json defs = json::array();
            for (auto& d : cg.defines) defs.push_back(d);
            cgj["defines"] = defs;
            cgs.push_back(cgj);
        }
        tj["compile_groups"] = cgs;

        json lfrags = json::array();
        for (auto& lf : t.link_fragments) {
            lfrags.push_back({{"fragment", lf.fragment}, {"role", lf.role}});
        }
        tj["link_fragments"] = lfrags;

        json deps = json::array();
        for (auto& dep : t.dependencies) deps.push_back(dep.id);
        tj["dependencies"] = deps;

        targets_arr.push_back(tj);
    }
    doc["targets"] = targets_arr;

    // Findings (filtered)
    json findings_arr = json::array();
    for (auto& f : result.findings) {
        if (!passes_filter(f.severity, min_severity)) continue;
        json fj;
        fj["severity"]       = severity_str(f.severity);
        fj["category"]       = category_str(f.category);
        fj["source"]         = source_str(f.source);
        fj["target"]         = f.target_name;
        fj["field"]          = f.field;
        fj["value"]          = f.value;
        fj["message"]        = f.message;
        fj["recommendation"] = f.recommendation;
        findings_arr.push_back(fj);
    }
    doc["findings"] = findings_arr;

    // Dependency edges
    json edges_arr = json::array();
    for (auto& e : result.edges) {
        edges_arr.push_back({{"from", e.from_target}, {"to", e.to_target}, {"kind", e.kind}});
    }
    doc["dependency_edges"] = edges_arr;

    auto out_path = output_dir / "report.json";
    std::ofstream ofs(out_path);
    if (!ofs) throw std::runtime_error("Cannot write: " + out_path.string());
    ofs << doc.dump(2) << "\n";
    log_info("Written: " + out_path.string());
}

// ─── Markdown Report ──────────────────────────────────────────────────────────

static std::string severity_badge(FindingSeverity s) {
    switch (s) {
        case FindingSeverity::Error:   return "**ERROR**";
        case FindingSeverity::Warning: return "**WARNING**";
        case FindingSeverity::Info:    return "*INFO*";
    }
    return "?";
}

static std::string filter_label(SeverityFilter f) {
    switch (f) {
        case SeverityFilter::Info:    return "info (all)";
        case SeverityFilter::Warning: return "warning and above";
        case SeverityFilter::Error:   return "error only";
    }
    return "info";
}

void write_markdown_report(const AnalysisResult& result,
                            const std::filesystem::path& output_dir,
                            SeverityFilter min_severity)
{
    std::ostringstream md;

    md << "# Portability Analysis Report\n\n";
    md << "**Project:** " << result.project_name << "  \n";
    md << "**CMake version:** " << result.cmake_version << "  \n";
    md << "**Generated at:** " << now_iso8601() << "  \n";
    md << "**Tool:** analyze-portability v1.1.0  \n";
    if (min_severity != SeverityFilter::Info) {
        md << "**Severity filter:** " << filter_label(min_severity) << "  \n";
    }
    md << "\n---\n\n";

    // Count totals and filtered
    int errors, warnings, infos;
    count_filtered(result, min_severity, errors, warnings, infos);
    int emitted = errors + warnings + infos;
    int hidden  = static_cast<int>(result.findings.size()) - emitted;

    // Summary table
    md << "## Summary\n\n";
    md << "| Metric | Count |\n";
    md << "|--------|-------|\n";
    md << "| Targets analyzed | " << result.targets.size() << " |\n";
    if (!result.service_targets.empty()) {
        md << "| CTest service targets excluded | " << result.service_targets.size() << " |\n";
    }
    md << "| Total findings | " << result.findings.size() << " |\n";
    md << "| Emitted findings | " << emitted << " |\n";
    if (hidden > 0) {
        md << "| Hidden by severity filter | " << hidden << " |\n";
    }
    md << "| Errors | " << errors << " |\n";
    md << "| Warnings | " << warnings << " |\n";
    md << "| Info | " << infos << " |\n";
    md << "| Dependency edges | " << result.edges.size() << " |\n\n";

    // Findings by category
    if (emitted > 0) {
        std::map<std::string, int> cat_count, src_count;
        for (auto& f : result.findings) {
            if (!passes_filter(f.severity, min_severity)) continue;
            cat_count[category_str(f.category)]++;
            src_count[source_str(f.source)]++;
        }

        md << "### Findings by Category\n\n";
        md << "| Category | Count |\n|----------|-------|\n";
        for (auto& [cat, cnt] : cat_count) md << "| " << cat << " | " << cnt << " |\n";
        md << "\n";

        md << "### Findings by Source\n\n";
        md << "| Source | Count |\n|--------|-------|\n";
        for (auto& [src, cnt] : src_count) md << "| " << src << " | " << cnt << " |\n";
        md << "\n";
    }

    // Data sources status
    md << "### Data Sources\n\n";
    md << "| Source | Found | Parsed | Details |\n|--------|-------|--------|---------|\n";
    md << "| CMake File API | yes | yes | — |\n";
    {
        auto& cc = result.compile_commands;
        md << "| compile_commands.json | " << (cc.found ? "yes" : "no") << " | "
           << (cc.found ? (cc.parsed ? "yes" : "no") : "—") << " | ";
        if (cc.found && cc.parsed)
            md << cc.entry_count << " entries, " << cc.observations.size() << " observations";
        else if (cc.found && !cc.parsed)
            md << "Parse error: " << cc.parse_error;
        else
            md << "Not generated";
        md << " |\n";
    }
    {
        auto& cache = result.cmake_cache;
        md << "| CMakeCache.txt | " << (cache.found ? "yes" : "no") << " | "
           << (cache.found ? (cache.parsed ? "yes" : "no") : "—") << " | ";
        if (cache.found && cache.parsed)
            md << cache.variable_count << " total variables, "
               << cache.variables.size() << " interesting";
        else if (cache.found && !cache.parsed)
            md << "Parse error: " << cache.parse_error;
        else
            md << "Not found";
        md << " |\n";
    }
    md << "\n";

    if (!result.service_targets.empty()) {
        md << "> **Note:** " << result.service_targets.size()
           << " CTest dashboard service target(s) were excluded from this analysis "
           << "(Continuous\\*, Experimental\\*, Nightly\\*). "
           << "Use `--include-utility-targets` to include them.\n\n";
    }

    // Targets table
    md << "## Targets\n\n";
    if (result.targets.empty()) {
        md << "_No targets found._\n\n";
    } else {
        md << "| Name | Type | Language(s) | Sources |\n";
        md << "|------|------|-------------|--------|\n";
        for (auto& t : result.targets) {
            std::string langs;
            for (auto& cg : t.compile_groups) {
                if (!langs.empty()) langs += ", ";
                langs += cg.language;
            }
            if (langs.empty()) langs = "—";
            md << "| `" << t.name << "` | " << t.type << " | " << langs
               << " | " << t.source_files.size() << " |\n";
        }
        md << "\n";
    }

    // Dependency edges
    if (!result.edges.empty()) {
        md << "## Dependency Graph (edges)\n\n";
        md << "| From | To | Kind |\n|------|----|------|\n";
        for (auto& e : result.edges) {
            md << "| `" << e.from_target << "` | `" << e.to_target << "` | " << e.kind << " |\n";
        }
        md << "\n";
    }

    // Portability Findings
    md << "## Portability Findings\n\n";

    // Collect filtered findings
    std::vector<const Finding*> emitted_findings;
    for (auto& f : result.findings) {
        if (passes_filter(f.severity, min_severity)) emitted_findings.push_back(&f);
    }

    if (emitted_findings.empty()) {
        if (hidden > 0) {
            md << "_No portability issues at the current severity level "
               << "(" << filter_label(min_severity) << ")._  \n";
            md << "_(" << hidden << " lower-severity findings hidden by filter.)_\n\n";
        } else {
            md << "_No portability issues detected._\n\n";
        }
    } else {
        // Group by source, then by target
        // Show File API and analyzer findings first (per-target), then compile_commands and cache
        std::map<std::string, std::map<std::string, std::vector<const Finding*>>> by_src_target;
        for (auto* f : emitted_findings) {
            std::string src = source_str(f->source);
            std::string tgt = f->target_name.empty() ? "(project-level)" : f->target_name;
            by_src_target[src][tgt].push_back(f);
        }

        // Emit per-source sections
        static const std::vector<std::pair<std::string,std::string>> SOURCE_ORDER = {
            {"file_api",         "File API (CMake Targets)"},
            {"compile_commands", "compile_commands.json"},
            {"cmake_cache",      "CMakeCache.txt"},
            {"analyzer",         "Analyzer"}
        };

        for (auto& [src_key, src_label] : SOURCE_ORDER) {
            auto it = by_src_target.find(src_key);
            if (it == by_src_target.end()) continue;

            md << "### " << src_label << "\n\n";
            for (auto& [tname, flist] : it->second) {
                md << "#### Target: `" << tname << "`\n\n";
                for (auto* f : flist) {
                    md << "- " << severity_badge(f->severity) << " ["
                       << category_str(f->category) << "] "
                       << "**Field:** `" << f->field << "` — **Value:** `" << f->value << "`\n";
                    md << "  - " << f->message << "\n";
                    md << "  - _Recommendation:_ " << f->recommendation << "\n\n";
                }
            }
        }
    }

    if (hidden > 0) {
        md << "> **Note:** " << hidden << " finding(s) were hidden by the severity filter "
           << "(`" << filter_label(min_severity) << "`). "
           << "Run without `--no-info` or with `--min-severity info` to see all findings.\n\n";
    }

    md << "---\n\n";
    md << "*Generated by analyze-portability v1.1.0 — a CMake portability analysis tool for ВКР.*\n";

    auto out_path = output_dir / "report.md";
    std::ofstream ofs(out_path);
    if (!ofs) throw std::runtime_error("Cannot write: " + out_path.string());
    ofs << md.str();
    log_info("Written: " + out_path.string());
}

// ─── DOT/Graphviz Report ──────────────────────────────────────────────────────

static std::string dot_color(const std::string& type) {
    if (type == "EXECUTABLE")                              return "lightblue";
    if (type.find("SHARED") != std::string::npos)         return "lightyellow";
    if (type.find("STATIC") != std::string::npos)         return "lightgreen";
    if (type == "INTERFACE_LIBRARY")                       return "lightsalmon";
    if (type == "UTILITY")                                 return "lightgray";
    return "white";
}

static std::string dot_escape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

void write_dot_report(const AnalysisResult& result,
                       const std::filesystem::path& output_dir,
                       SeverityFilter min_severity)
{
    std::ostringstream dot;

    dot << "// analyze-portability: dependency graph for project '" << result.project_name << "'\n";
    dot << "digraph dependencies {\n";
    dot << "  graph [rankdir=LR, label=\"" << dot_escape(result.project_name)
        << " — Dependency Graph\", fontsize=14];\n";
    dot << "  node [shape=box, style=filled, fontname=\"Helvetica\"];\n\n";

    for (auto& t : result.targets) {
        int warn_count = 0;
        int err_count  = 0;
        for (auto& f : result.findings) {
            if (f.target_name != t.name) continue;
            if (!passes_filter(f.severity, min_severity)) continue;
            if (f.severity == FindingSeverity::Warning) ++warn_count;
            if (f.severity == FindingSeverity::Error)   ++err_count;
        }
        std::string color = dot_color(t.type);
        std::string label = dot_escape(t.name) + "\\n[" + t.type + "]";
        if (err_count > 0) {
            label += "\\n✗ " + std::to_string(err_count) + " errors";
            color = "lightcoral";
        } else if (warn_count > 0) {
            label += "\\n⚠ " + std::to_string(warn_count) + " warnings";
            color = "lightyellow";
        }
        dot << "  \"" << dot_escape(t.name) << "\" [label=\"" << label
            << "\", fillcolor=\"" << color << "\"];\n";
    }

    dot << "\n";

    for (auto& e : result.edges) {
        std::string style;
        if (e.kind == "interface") style = " [style=dashed, color=gray]";
        else if (e.kind == "utility") style = " [style=dotted]";
        dot << "  \"" << dot_escape(e.from_target) << "\" -> \""
            << dot_escape(e.to_target) << "\"" << style << ";\n";
    }

    dot << "}\n";

    auto out_path = output_dir / "dependencies.dot";
    std::ofstream ofs(out_path);
    if (!ofs) throw std::runtime_error("Cannot write: " + out_path.string());
    ofs << dot.str();
    log_info("Written: " + out_path.string());
}

// ─── write_reports (format-flag aware) ───────────────────────────────────────

void write_reports(const AnalysisResult& result,
                   const std::filesystem::path& output_dir,
                   const Options& opts)
{
    bool write_all = !opts.out_json && !opts.out_markdown && !opts.out_graphviz;

    if (write_all || opts.out_json)     write_json_report(result, output_dir, opts.min_severity);
    if (write_all || opts.out_markdown) write_markdown_report(result, output_dir, opts.min_severity);
    if (write_all || opts.out_graphviz) write_dot_report(result, output_dir, opts.min_severity);
}

// ─── Failure Reports (cmake configure failed / File API unavailable) ───────────────────────────────

void write_failure_json(const FailureContext& ctx,
                        const std::filesystem::path& output_dir)
{
    json doc;
    doc["generated_at"]  = now_iso8601();
    doc["tool"]          = "analyze-portability";
    doc["tool_version"]  = "1.1.0";
    doc["status"]        = "configure_failed";
    doc["project_name"]  = "unknown";
    doc["source_dir"]    = ctx.source_dir.empty() ? "" : ctx.source_dir.string();
    doc["build_dir"]     = ctx.build_dir.empty()  ? "" : ctx.build_dir.string();

    doc["configure_status"] = {
        {"success",   false},
        {"exit_code", ctx.configure_exit_code},
        {"error",     ctx.error_message}
    };

    doc["file_api_note"] = "CMake File API reply was unavailable "
                           "(configure failed before CMake could write reply files). "
                           "Portability analysis could not be performed.";

    // Data sources status
    json src_doc;
    src_doc["file_api"] = false;
    {
        json cc_doc;
        cc_doc["found"]  = ctx.compile_commands.found;
        cc_doc["parsed"] = ctx.compile_commands.parsed;
        if (!ctx.compile_commands.parse_error.empty())
            cc_doc["parse_error"] = ctx.compile_commands.parse_error;
        src_doc["compile_commands"] = cc_doc;
    }
    {
        json cache_doc;
        cache_doc["found"]          = ctx.cmake_cache.found;
        cache_doc["parsed"]         = ctx.cmake_cache.parsed;
        cache_doc["variable_count"] = ctx.cmake_cache.variable_count;
        if (!ctx.cmake_cache.parse_error.empty())
            cache_doc["parse_error"] = ctx.cmake_cache.parse_error;
        src_doc["cmake_cache"] = cache_doc;
    }
    doc["data_sources"] = src_doc;

    doc["summary"] = {
        {"total_targets",   0},
        {"total_findings",  1},
        {"errors",          1},
        {"warnings",        0},
        {"info",            0},
        {"findings_by_category", {{"configure_failure", 1}}},
        {"findings_by_source",   {{"analyzer", 1}}}
    };

    doc["targets"] = json::array();

    json finding;
    finding["severity"]       = "error";
    finding["category"]       = "configure_failure";
    finding["source"]         = "analyzer";
    finding["target"]         = "";
    finding["field"]          = "cmake_configure";
    finding["value"]          = "exit_code=" + std::to_string(ctx.configure_exit_code);
    finding["message"]        = ctx.error_message;
    finding["recommendation"] = "Inspect the configure log for the root cause. "
                                "Check for platform-specific CMake dependencies "
                                "(Windows SDK, MSVC flags, platform-only find_package). "
                                "Run on the supported OS or add platform guards (if(WIN32), "
                                "if(APPLE), etc.) to the CMakeLists.txt.";
    doc["findings"] = json::array({finding});
    doc["dependency_edges"] = json::array();

    auto out_path = output_dir / "report.json";
    std::ofstream ofs(out_path);
    if (!ofs) throw std::runtime_error("Cannot write: " + out_path.string());
    ofs << doc.dump(2) << "\n";
    log_info("Written (failure report): " + out_path.string());
}

void write_failure_markdown(const FailureContext& ctx,
                            const std::filesystem::path& output_dir)
{
    std::ostringstream md;

    md << "# Portability Analysis Report — Configure Failed\n\n";
    md << "**Status:** cmake configure FAILED (exit code " << ctx.configure_exit_code << ")  \n";
    md << "**Generated at:** " << now_iso8601() << "  \n";
    md << "**Tool:** analyze-portability v1.1.0  \n";
    md << "\n---\n\n";

    md << "## Configure Failure Details\n\n";
    md << "| Field | Value |\n|-------|-------|\n";
    md << "| Source dir | `" << ctx.source_dir.string() << "` |\n";
    md << "| Build dir  | `" << ctx.build_dir.string()  << "` |\n";
    md << "| Exit code  | " << ctx.configure_exit_code << " |\n\n";

    md << "**Error:** " << ctx.error_message << "\n\n";

    md << "## What This Means for Portability Analysis\n\n";
    md << "CMake configure failed before producing the File API reply. "
          "Because the File API reply is the primary data source for this tool, "
          "a full portability analysis could not be completed.\n\n";
    md << "This typically happens when the project has **platform-specific CMake dependencies** "
          "that cannot be satisfied on the current OS. Examples:\n\n";
    md << "- Windows-only packages required unconditionally (`find_package(DirectX REQUIRED)`)\n";
    md << "- Windows SDK paths hard-coded in CMake scripts\n";
    md << "- FetchContent downloading Windows-only binaries\n";
    md << "- MSVC-only compiler flags set without platform guards\n\n";

    // Data sources status
    md << "## Data Sources Status\n\n";
    md << "| Source | Available |\n|--------|-----------|\n";
    md << "| CMake File API reply | **NO** — configure failed |\n";
    {
        auto& cc = ctx.compile_commands;
        md << "| compile_commands.json | " << (cc.found ? (cc.parsed ? "yes (partial)" : "found, parse failed") : "no") << " |\n";
    }
    {
        auto& cache = ctx.cmake_cache;
        md << "| CMakeCache.txt | ";
        if (cache.found && cache.parsed)
            md << "yes — " << cache.variable_count << " variables (partial configure)";
        else if (cache.found)
            md << "found, parse failed";
        else
            md << "no";
        md << " |\n";
    }
    md << "\n";

    md << "## Recommendations\n\n";
    md << "1. **Inspect the configure log** — check `configure_or_tool.log` (if saved with `-v`) "
          "for the cmake error messages.\n";
    md << "2. **Check platform-specific CMake dependencies** — look for `find_package`, `FetchContent`, "
          "and Windows SDK paths used without `if(WIN32)` guards.\n";
    md << "3. **Run on the supported OS** — if the project targets Windows only, run analyze-portability "
          "in a Windows environment to get a full report.\n";
    md << "4. **Add platform guards** — add `if(WIN32)` / `if(APPLE)` / `if(UNIX)` conditions to "
          "CMakeLists.txt to make configure succeed on all platforms (even if features are disabled).\n";
    md << "5. **Abstract platform-specific dependencies** — use CMake's cross-platform mechanisms "
          "(`find_library`, `find_package` with `QUIET` + fallback) rather than hard-coding SDK paths.\n\n";

    md << "---\n\n";
    md << "*Generated by analyze-portability v1.1.0 — configure failure diagnostic mode.*\n";

    auto out_path = output_dir / "report.md";
    std::ofstream ofs(out_path);
    if (!ofs) throw std::runtime_error("Cannot write: " + out_path.string());
    ofs << md.str();
    log_info("Written (failure report): " + out_path.string());
}

void write_failure_dot(const FailureContext& ctx,
                       const std::filesystem::path& output_dir)
{
    std::ostringstream dot;
    dot << "// analyze-portability: configure FAILED — dependency graph unavailable\n";
    dot << "// cmake exited with code " << ctx.configure_exit_code << "\n";
    dot << "// CMake File API reply was not produced.\n";
    dot << "digraph dependencies {\n";
    dot << "  graph [label=\"Configure Failed — no dependency data\", fontsize=14];\n";
    dot << "  // No targets or edges available (File API reply unavailable).\n";
    dot << "}\n";

    auto out_path = output_dir / "dependencies.dot";
    std::ofstream ofs(out_path);
    if (!ofs) throw std::runtime_error("Cannot write: " + out_path.string());
    ofs << dot.str();
    log_info("Written (failure report): " + out_path.string());
}

void write_failure_reports(const FailureContext& ctx,
                           const std::filesystem::path& output_dir,
                           const Options& opts)
{
    bool write_all = !opts.out_json && !opts.out_markdown && !opts.out_graphviz;

    if (write_all || opts.out_json)     write_failure_json(ctx, output_dir);
    if (write_all || opts.out_markdown) write_failure_markdown(ctx, output_dir);
    if (write_all || opts.out_graphviz) write_failure_dot(ctx, output_dir);
}

} // namespace anaport
