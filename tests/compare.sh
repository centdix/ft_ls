#!/bin/sh
# ============================================================================
#  compare.sh - compares the output of ./ft_ls with the real ls
#
#  Usage:
#     ./tests/compare.sh [OPTIONS_AND_PATHS...]
#
#  Examples:
#     ./tests/compare.sh -l /etc
#     ./tests/compare.sh -laR .
#     ./tests/compare.sh -lt /tmp /usr
#
#  Prints a diff. If nothing is printed -> outputs are identical (perfect).
#  Everything runs under LC_ALL=C, as during evaluation.
#
#  NB: stdout is compared BYTE FOR BYTE (via temp files), so a trailing-newline
#  difference is caught. The reference is the REAL GNU coreutils (the 42
#  evaluation target): we prefer /usr/bin/gnuls when present (invoked with
#  argv[0]="ls" so its error prefix is "ls:" like ours), otherwise we fall back
#  to `command ls` (which is GNU on a standard machine). --color=never avoids a
#  shell `ls --color=...` alias.
# ============================================================================

BIN=./ft_ls

if [ ! -x "$BIN" ]; then
	echo "ft_ls is not built (run 'make' first)." >&2
	exit 1
fi

if [ -x /usr/bin/gnuls ]; then
	refls() { bash -c 'exec -a ls /usr/bin/gnuls --color=never "$@"' ls "$@"; }
else
	refls() { command ls --color=never "$@"; }
fi

out_mine=$(mktemp); err_mine=$(mktemp)
out_real=$(mktemp); err_real=$(mktemp)

LC_ALL=C "$BIN" "$@" >"$out_mine" 2>"$err_mine"; code_mine=$?
LC_ALL=C refls  "$@" >"$out_real" 2>"$err_real"; code_real=$?

echo "===== STDOUT diff (left=ft_ls, right=ls) ====="
diff "$out_mine" "$out_real" && echo "  [OK] identical stdout"

echo "===== STDERR diff (left=ft_ls, right=ls) ====="
diff "$err_mine" "$err_real" && echo "  [OK] identical stderr"

echo "===== EXIT CODE ====="
echo "  ft_ls=$code_mine   ls=$code_real"
[ "$code_mine" = "$code_real" ] && echo "  [OK]" || echo "  [DIFF]"

rm -f "$out_mine" "$err_mine" "$out_real" "$err_real"
