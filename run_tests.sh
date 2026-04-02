#!/usr/bin/env bash
set -euo pipefail

echo "==> Building..."
make

mkdir -p tests/my-output

declare -A TESTS=(
    ["input_rectangle_with_two_holes.csv"]=11
    ["input_cushion_with_hexagonal_hole.csv"]=13
    ["input_blob_with_two_holes.csv"]=17
    ["input_wavy_with_three_holes.csv"]=21
    ["input_lake_with_two_islands.csv"]=17
    ["input_original_01.csv"]=99
    ["input_original_02.csv"]=99
    ["input_original_03.csv"]=99
    ["input_original_04.csv"]=99
    ["input_original_05.csv"]=99
    ["input_original_06.csv"]=99
    ["input_original_07.csv"]=99
    ["input_original_08.csv"]=99
    ["input_original_09.csv"]=99
    ["input_original_10.csv"]=99
)

PASS=0
FAIL=0
TOTAL=0

for INPUT in "${!TESTS[@]}"; do
    TARGET="${TESTS[$INPUT]}"
    BASE="${INPUT#input_}"
    BASE="${BASE%.csv}"

    INPUT_FILE="tests/input/$INPUT"
    EXPECTED_FILE="tests/expected-output/output_${BASE}.txt"
    ACTUAL_FILE="tests/my-output/my_output_${BASE}.txt"

    if [[ ! -f "$INPUT_FILE" ]]; then
        echo "SKIP  $INPUT (missing input)"
        continue
    fi

    if [[ ! -f "$EXPECTED_FILE" ]]; then
        echo "SKIP  $INPUT (missing expected output)"
        continue
    fi

    TOTAL=$((TOTAL + 1))

    echo "RUN   $INPUT (target=$TARGET)"
    if ./simplify "$INPUT_FILE" "$TARGET" > "$ACTUAL_FILE"; then
        if diff -u "$EXPECTED_FILE" "$ACTUAL_FILE" > /tmp/simplify_diff.txt; then
            echo "PASS  $INPUT"
            PASS=$((PASS + 1))
        else
            echo "FAIL  $INPUT"
            head -40 /tmp/simplify_diff.txt
            FAIL=$((FAIL + 1))
        fi
    else
        echo "FAIL  $INPUT (program error)"
        FAIL=$((FAIL + 1))
    fi

    echo ""
done

echo "==============================="
echo "Passed: $PASS"
echo "Failed: $FAIL"
echo "Total : $TOTAL"
echo "==============================="

if [[ $FAIL -ne 0 ]]; then
    exit 1
fi