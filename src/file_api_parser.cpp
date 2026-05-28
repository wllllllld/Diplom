#include "anaport/file_api_parser.hpp"
#include "anaport/logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>
#include <regex>

using json = nlohmann::json;

namespace anaport {

static json load_json(const std::filesystem::path& p) {
    std::ifstream ifs(p);
    if (!ifs) {
        throw std::runtime_error("Cannot open JSON file: " + p.string());
    }
    try {
        return json::parse(ifs);
    } catch (const json::exception& e) {
        throw std::runtime_error("JSON parse error in " + p.string() + ": " + e.what());
    }
}

/// Find the index-*.json file in reply dir
static std::filesystem::path find_index_file(const std::filesystem::path& reply_dir) {
    namespace fs = std::filesystem;
    for (auto& entry : fs::directory_iterator(reply_dir)) {
        auto name = entry.path().filename().string();
        if (name.starts_with("index-") && name.ends_with(".json")) {
            return entry.path();
        }
    }
    throw std::runtime_error("No index-*.json found in: " + reply_dir.string());
}

/// Find the codemodel-v2-*.json file referenced in the index
static std::filesystem::path find_codemodel_file(
    const std::filesystem::path& reply_dir,
    const json& index_json)
{
    // index JSON: { "reply": { "codemodel-v2": { "kind": "codemodel", "version": {..}, "jsonFile": "<name>" } } }
    // Also can be array. Check both structures.
    try {
        if (index_json.contains("reply")) {
            for (auto& [key, val] : index_json["reply"].items()) {
                if (key.starts_with("codemodel-v2") || key == "codemodel") {
                    if (val.contains("jsonFile")) {
                        return reply_dir / val["jsonFile"].get<std::string>();
                    }
                }
            }
        }
        // Try stateless API: replies is an array
        if (index_json.contains("replies")) {
            for (auto& rep : index_json["replies"]) {
                if (rep.value("kind", "") == "codemodel") {
                    return reply_dir / rep["jsonFile"].get<std::string>();
                }
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error parsing index JSON: ") + e.what());
    }

    // Fallback: search for codemodel-v2-*.json directly
    namespace fs = std::filesystem;
    for (auto& entry : fs::directory_iterator(reply_dir)) {
        auto name = entry.path().filename().string();
        if (name.starts_with("codemodel-v2-") && name.ends_with(".json")) {
            log_verbose("Found codemodel by filesystem scan: " + name);
            return entry.path();
        }
    }

    throw std::runtime_error("No codemodel-v2 reply file found in: " + reply_dir.string());
}

static CompileGroup parse_compile_group(const json& cg_json) {
    CompileGroup cg;
    cg.language = cg_json.value("language", "");

    if (cg_json.contains("compileCommandFragments")) {
        for (auto& frag : cg_json["compileCommandFragments"]) {
            CompileFragment cf;
            cf.fragment = frag.value("fragment", "");
            cf.role = "flags";
            cg.fragments.push_back(cf);
        }
    }

    if (cg_json.contains("includes")) {
        for (auto& inc : cg_json["includes"]) {
            IncludePath ip;
            ip.path = inc.value("path", "");
            ip.is_system = inc.value("isSystem", false);
            cg.includes.push_back(ip);
        }
    }

    if (cg_json.contains("defines")) {
        for (auto& def : cg_json["defines"]) {
            cg.defines.push_back(def.value("define", ""));
        }
    }

    return cg;
}

static RawTarget parse_target_file(const std::filesystem::path& target_file) {
    auto tj = load_json(target_file);
    RawTarget t;

    t.id   = tj.value("id", "");
    t.name = tj.value("name", "");
    t.type = tj.value("type", "");

    if (tj.contains("paths")) {
        t.source_dir = tj["paths"].value("source", "");
        t.build_dir  = tj["paths"].value("build", "");
    }

    // artifacts
    if (tj.contains("artifacts")) {
        for (auto& art : tj["artifacts"]) {
            Artifact a;
            a.path = art.value("path", "");
            t.artifacts.push_back(a);
        }
    }

    // sources
    if (tj.contains("sources")) {
        for (auto& src : tj["sources"]) {
            t.source_files.push_back(src.value("path", ""));
        }
    }

    // compileGroups
    if (tj.contains("compileGroups")) {
        for (auto& cg_json : tj["compileGroups"]) {
            t.compile_groups.push_back(parse_compile_group(cg_json));
        }
    }

    // link
    if (tj.contains("link")) {
        auto& link = tj["link"];
        if (link.contains("commandFragments")) {
            for (auto& frag : link["commandFragments"]) {
                LinkFragment lf;
                lf.fragment = frag.value("fragment", "");
                lf.role     = frag.value("role", "");
                t.link_fragments.push_back(lf);
            }
        }
    }

    // dependencies
    if (tj.contains("dependencies")) {
        for (auto& dep : tj["dependencies"]) {
            TargetDependency td;
            td.id = dep.value("id", "");
            t.dependencies.push_back(td);
        }
    }

    log_verbose("  Parsed target: " + t.name + " [" + t.type + "]"
        + " compileGroups=" + std::to_string(t.compile_groups.size())
        + " linkFrags=" + std::to_string(t.link_fragments.size())
        + " deps=" + std::to_string(t.dependencies.size()));

    return t;
}

RawProject parse_file_api(const std::filesystem::path& build_dir, bool verbose) {
    namespace fs = std::filesystem;

    auto reply_dir = build_dir / ".cmake" / "api" / "v1" / "reply";
    if (!fs::exists(reply_dir)) {
        throw std::runtime_error("CMake File API reply dir not found: " + reply_dir.string()
            + "\n  Did cmake configure succeed?");
    }

    log_verbose("Reading File API reply dir: " + reply_dir.string());

    // Load index
    auto index_path = find_index_file(reply_dir);
    log_verbose("Index file: " + index_path.filename().string());
    auto index_json = load_json(index_path);

    // Extract cmake version from index
    std::string cmake_version;
    if (index_json.contains("cmake") && index_json["cmake"].contains("version")) {
        auto& ver = index_json["cmake"]["version"];
        cmake_version = ver.value("string", "");
    }

    // Load codemodel
    auto codemodel_path = find_codemodel_file(reply_dir, index_json);
    log_verbose("Codemodel file: " + codemodel_path.filename().string());
    auto codemodel = load_json(codemodel_path);

    RawProject project;
    project.cmake_version = cmake_version;

    // Extract project source dir from codemodel paths
    if (codemodel.contains("paths")) {
        project.source_dir = codemodel["paths"].value("source", "");
    }

    // Project name from codemodel
    if (codemodel.contains("configurations")) {
        auto& configs = codemodel["configurations"];
        if (!configs.empty()) {
            auto& cfg = configs[0];

            // Project name
            if (cfg.contains("projects") && !cfg["projects"].empty()) {
                project.name = cfg["projects"][0].value("name", "unknown");
            }

            // Targets
            if (cfg.contains("targets")) {
                for (auto& tref : cfg["targets"]) {
                    std::string tname = tref.value("name", "");
                    std::string jf    = tref.value("jsonFile", "");
                    if (jf.empty()) continue;

                    auto target_path = reply_dir / jf;
                    if (!fs::exists(target_path)) {
                        log_warn("Target file not found: " + target_path.string());
                        continue;
                    }

                    try {
                        auto t = parse_target_file(target_path);
                        project.targets.push_back(std::move(t));
                    } catch (const std::exception& e) {
                        log_warn("Error parsing target " + tname + ": " + e.what());
                    }
                }
            }
        }
    }

    if (project.name.empty()) project.name = "unknown";

    log_info("Parsed project: " + project.name
        + " (cmake " + project.cmake_version + ")"
        + ", targets: " + std::to_string(project.targets.size()));

    return project;
}

} // namespace anaport
