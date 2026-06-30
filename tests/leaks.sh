#!/bin/sh
# ============================================================================
#  leaks.sh — verifie l'absence de fuites memoire avec valgrind
#
#  Usage:
#     ./tests/leaks.sh [OPTIONS_ET_CHEMINS...]
#
#  Exemple:
#     ./tests/leaks.sh -laR /etc
#
#  Exige 0 fuite ET 0 erreur (--error-exitcode=42).
# ============================================================================

BIN=./ft_ls

if ! command -v valgrind >/dev/null 2>&1; then
	echo "valgrind n'est pas installe." >&2
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
	echo "  [KO] fuites ou erreurs memoire detectees"
else
	echo "  [OK] aucune fuite / erreur memoire"
fi
