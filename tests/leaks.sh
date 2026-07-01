#!/bin/sh
# ============================================================================
#  leaks.sh - checks for memory leaks with valgrind
#
#  Usage:
#     ./tests/leaks.sh [OPTIONS_AND_PATHS...]
#
#  Example:
#     ./tests/leaks.sh -laR /etc
#
#  Requires 0 leaks AND 0 errors (--error-exitcode=42).
# ============================================================================

BIN=./ft_ls

if ! command -v valgrind >/dev/null 2>&1; then
	echo "valgrind is not installed." >&2
	exit 1
fi

LC_ALL=C valgrind \
	--leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	--error-exitcode=42 \
	"$BIN" "$@" >/dev/null

status=$?
echo "-----------------------------------------"
if [ "$status" = "42" ]; then
	echo "  [KO] memory leaks or errors detected"
else
	echo "  [OK] no memory leak / error"
fi
