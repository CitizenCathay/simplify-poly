# simplify-poly

Repository for the CSD2183 Assignment 2.

## Overview

This project implements an area- and topology-preserving polygon simplification
program. The executable reads a polygon in CSV format, simplifies it to a target
maximum number of vertices, and prints the simplified polygon and summary
statistics to standard output.

## Dependencies

This project uses standard C++ and does not require any third-party libraries or
proprietary software.

- A C++17-compatible compiler (`g++`)
- The C++ standard library
- `make`
- Bash (for the test script)

No external geometry library such as CGAL is used.

## Build

```bash
make
```

## Usage

```bash
./simplify <input-file> <target-vertices>
```

| Argument          | Description                                         |
| ----------------- | --------------------------------------------------- |
| `input-file`      | Path to the input polygon in CSV format             |
| `target-vertices` | Maximum number of vertices in the simplified output |

The simplified polygon and summary statistics are written to standard output.
To save the result to a file, redirect stdout:

```bash
./simplify input.csv 100 > output.csv
```

### Example

```bash
./simplify data/exp02_noisy_circle_10000.csv 500
```

## Automated Tests

The repository includes `run_tests.sh`, which builds the project and runs all
provided test cases automatically.

```bash
bash run_tests.sh
```

## Visualizing results

The repository includes `run_overlay.sh`, which builds SVGs that overlay the generated simplifications against the expected outcomes dataset. It also generates
an overlayed SVG for the experimental data sets. You must run run_tests.sh before running this script.

```bash
bash run_overlay.sh
```

## Troubleshooting

### 1. Fix Script Error: `invalid option name: pipefail`
If you encounter an error like `invalid option name set: pipefail` when running the test script in WSL, it is caused by **Windows-style line endings (CRLF)**. This happens if the script was edited in a Windows text editor.

**To fix this, run the following command in your WSL terminal:**

```bash
sed -i 's/\r$//' <filename>.sh
```
