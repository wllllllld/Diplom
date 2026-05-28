#include "anaport/compile_commands_parser.hpp"
#include "anaport/logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

namespace anaport {

// ─── Flag extraction helpers ──────────────────────────────────────────────────

/// Tokenise a command string respecting simple quoting (no full shell).
static std::vector<std::string> tokenise_command(const std::string& cmd) {
    std::vector<std::string> tokens;
    std::string cur;
    bool in_single = false;
    bool in_double = false;
    for (size_t i = 0; i < cmd.size(); ++i) {
        char c = cmd[i];
        if (c == '\'' && !in_double) {
            in_single = !in_single;
        } else if (c == '"' && !in_single) {
            in_double = !in_double;
        } else if (c == ' ' && !in_single && !in_double) {
            if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) tokens.push_back(cur);
    return tokens;
}

/// Arch/portability flags that may indicate platform-specific compilation
static const std::vector<std::string> ARCH_FLAGS = {
    "-march=", "-mcpu=", "-mtune=", "-mavx", "-msse", "-mfpu=",
    "-m32", "-m64", "-mfloat-abi=", "-mabi=", "-mthumb", "-marm"
};

/// Compiler-specific/non-portable flags
static const std::vector<std::string> COMPILER_FLAGS = {
    "-stdlib=", "-fno-", "-fPIC", "-fPIE", "-pthread",
    "-static-libstdc++", "-static-libgcc", "-static",
    "-fopenmp", "-fsanitize=", "-fstack-", "-fprofile-",
    "-flto", "-ffast-math", "-fomit-frame-pointer"
};

static bool starts_with(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static bool is_absolute_path_token(const std::string& tok) {
    return !tok.empty() && tok[0] == '/';
}

/// Extract portability observations from a list of tokens
static void extract_observations(const std::string& file,
                                  const std::vector<std::string>& tokens,
                                  std::vector<CompileCommandsObservation>& obs)
{
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& tok = tokens[i];

        // Include flags: -I/path, -isystem /path, -iquote /path
        if (starts_with(tok, "-I") && tok.size() > 2) {
            obs.push_back({file, tok.substr(2), "include"});
            continue;
        }
        if (tok == "-I" && i + 1 < tokens.size()) {
            obs.push_back({file, tokens[++i], "include"});
            continue;
        }
        if ((tok == "-isystem" || tok == "-iquote" || tok == "-idirafter") &&
            i + 1 < tokens.size()) {
            obs.push_back({file, tokens[++i], "include"});
            continue;
        }
        if (starts_with(tok, "-isystem") && tok.size() > 8) {
            obs.push_back({file, tok.substr(8), "include"});
            continue;
        }

        // Defines: -DFOO or -DFOO=bar
        if (starts_with(tok, "-D") && tok.size() > 2) {
            obs.push_back({file, tok.substr(2), "define"});
            continue;
        }

        // Sysroot
        if (tok == "--sysroot" && i + 1 < tokens.size()) {
            obs.push_back({file, tokens[++i], "sysroot"});
            continue;
        }
        if (starts_with(tok, "--sysroot=")) {
            obs.push_back({file, tok.substr(10), "sysroot"});
            continue;
        }
        if (tok == "-isysroot" && i + 1 < tokens.size()) {
            obs.push_back({file, tokens[++i], "sysroot"});
            continue;
        }

        // Architecture flags
        for (auto& af : ARCH_FLAGS) {
            if (starts_with(tok, af) || tok == af.substr(0, af.size()-1)) {
                obs.push_back({file, tok, "arch_flag"});
                break;
            }
        }

        // Compiler-specific flags
        for (auto& cf : COMPILER_FLAGS) {
            if (starts_with(tok, cf) || tok == cf) {
                obs.push_back({file, tok, "compiler_flag"});
                break;
            }
        }

        // Absolute path tokens (not flags, not compiler)
        if (is_absolute_path_token(tok) && !starts_with(tok, "-")) {
            // Only flag interesting non-standard paths
            if (!starts_with(tok, "/usr/include") &&
                !starts_with(tok, "/usr/lib") &&
                !starts_with(tok, "/usr/bin") &&
                tok.size() > 5) {
                obs.push_back({file, tok, "absolute_path"});
            }
        }
    }
}

// ─── Public API ───────────────────────────────────────────────────────────────

CompileCommandsData parse_compile_commands(const std::filesystem::path& build_dir,
                                            bool verbose)
{
    CompileCommandsData data;
    auto cc_path = build_dir / "compile_commands.json";

    if (!std::filesystem::exists(cc_path)) {
        log_verbose("compile_commands.json not found at: " + cc_path.string());
        data.found = false;
        return data;
    }

    data.found = true;
    log_info("Found compile_commands.json: " + cc_path.string());

    std::ifstream ifs(cc_path);
    if (!ifs) {
        data.parse_error = "Cannot open file: " + cc_path.string();
        log_warn("compile_commands.json: " + data.parse_error);
        return data;
    }

    json doc;
    try {
        doc = json::parse(ifs);
    } catch (const json::exception& e) {
        data.parse_error = std::string("JSON parse error: ") + e.what();
        log_warn("compile_commands.json: " + data.parse_error);
        return data;
    }

    if (!doc.is_array()) {
        data.parse_error = "Root element is not an array";
        log_warn("compile_commands.json: " + data.parse_error);
        return data;
    }

    data.parsed = true;
    int skipped = 0;

    for (auto& entry : doc) {
        if (!entry.is_object()) { ++skipped; continue; }

        CompileCommandEntry ce;
        ce.directory = entry.value("directory", "");
        ce.file      = entry.value("file", "");

        // Prefer arguments array; fall back to command string
        std::vector<std::string> tokens;
        if (entry.contains("arguments") && entry["arguments"].is_array()) {
            for (auto& arg : entry["arguments"]) {
                if (arg.is_string()) tokens.push_back(arg.get<std::string>());
            }
        } else if (entry.contains("command") && entry["command"].is_string()) {
            tokens = tokenise_command(entry["command"].get<std::string>());
        } else {
            ++skipped;
            continue;
        }

        // Skip first token (compiler executable)
        if (!tokens.empty()) tokens.erase(tokens.begin());

        extract_observations(ce.file, tokens, data.observations);
        ++data.entry_count;
    }

    if (skipped > 0) {
        log_verbose("compile_commands.json: skipped " + std::to_string(skipped) + " malformed entries");
    }

    // Deduplicate observations by (file, flag, kind)
    std::sort(data.observations.begin(), data.observations.end(),
        [](const CompileCommandsObservation& a, const CompileCommandsObservation& b) {
            if (a.flag != b.flag) return a.flag < b.flag;
            if (a.kind != b.kind) return a.kind < b.kind;
            return a.file < b.file;
        });
    data.observations.erase(
        std::unique(data.observations.begin(), data.observations.end(),
            [](const CompileCommandsObservation& a, const CompileCommandsObservation& b) {
                return a.flag == b.flag && a.kind == b.kind && a.file == b.file;
            }),
        data.observations.end());

    log_info("compile_commands.json: parsed " + std::to_string(data.entry_count)
             + " entries, " + std::to_string(data.observations.size()) + " observations");

    if (verbose) {
        for (auto& obs : data.observations) {
            log_verbose("  cc_obs [" + obs.kind + "] " + obs.flag + " (from " + obs.file + ")");
        }
    }

    return data;
}

} // namespace anaport
