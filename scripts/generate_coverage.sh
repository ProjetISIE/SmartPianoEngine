#!/bin/sh
LCOV="$1" # LCOV executable
GENHTML="$2" # GENHTML executable
GCOV_WRAPPER="$3" # GCOV_WRAPPER script
# SOURCE_DIR="$4" # PROJECT_SOURCE_DIR (to filter files correctly)
echo "LCOV: $LCOV"
echo "GCOV_WRAPPER: $GCOV_WRAPPER"

# Capture données couverture
echo "Capturing coverage data..."
"$LCOV" --directory . --capture --output-file coverage.info \
    --gcov-tool "$GCOV_WRAPPER" \
    --ignore-errors inconsistent,unsupported,format,empty,gcov || echo "Warning: LCOV capture failed (but continuing)"

# Nettoyage données
if [ -f coverage.info ]; then
    echo "Clean coverage data"
    "$LCOV" --remove coverage.info \
        '*/test/*' '/nix/store/*' '/usr/*' '*/.cache/*' \
        --output-file coverage_cleaned.info \
        --ignore-errors inconsistent,unsupported,format,unused,empty,gcov || echo "Warning: LCOV remove failed"
else
    echo "No coverage.info found"
fi

# Génération HTML
if [ -f coverage_cleaned.info ]; then
    echo "Generating HTML report"
    "$GENHTML" coverage_cleaned.info \
        --output-directory coverage_html \
        --ignore-errors inconsistent,unsupported,format,category,empty,gcov || echo "Warning: GENHTML failed"
else
    echo "No coverage_cleaned.info found, placeholder"
    mkdir -p coverage_html
    echo "<h1>No coverage data generated</h1><p>Check build logs for errors" > coverage_html/index.html
fi
