# analyze-portability

`analyze-portability` is a C++ CLI prototype for analyzing C/C++ CMake projects before porting them between Unix-like operating systems.

The tool uses CMake-generated build metadata to extract targets, dependencies, include paths, link fragments, compiler flags and configuration variables, then produces portability-oriented findings and reports.


## Requirements

- C++20 compiler
- CMake
- nlohmann/json
- Unix-like environment

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Usage

```bash
./build/analyze-portability /path/to/cmake/project --out reports
```

Generate selected report formats:

```bash
./build/analyze-portability /path/to/project --out reports -jmg
```

Available format flags:

```text
-j    generate report.json
-m    generate report.md
-g    generate dependencies.dot
```

If no format flag is specified, all three reports are generated.


`report.json` is intended for automated processing.

`report.md` is intended for human-readable review.

`dependencies.dot` can be rendered with Graphviz:



## Example

```bash
./build/analyze-portability tests/fixtures/test_problematic_paths \
  --out examples/out \
  -jm \
  --no-info
```
