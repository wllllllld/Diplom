cmake_minimum_required(VERSION 3.16)
project(MultiTarget VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── lib_a: utility library (no external deps) ─────────────────────────────────
add_subdirectory(lib_a)

# ── lib_b: depends on lib_a ───────────────────────────────────────────────────
add_subdirectory(lib_b)

# ── app: main executable ──────────────────────────────────────────────────────
add_subdirectory(app)
