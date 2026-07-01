#!/usr/bin/env bash
# ============================================================================
#  run_all.sh - regression matrix: compares ft_ls against the real ls over
#  many invocations and prints a PASS/FAIL summary.
#
#  Each case checks: identical stdout, identical stderr, same exit code.
#  Everything runs under LC_ALL=C (as during evaluation).
#
#  Usage:
#     ./tests/run_all.sh            # compact summary
#     ./tests/run_all.sh -v         # show the diff of failing cases
#     ./tests/run_all.sh -l         # also run a valgrind leak check (if present)
# ============================================================================

cd "$(dirname "$0")/.." || exit 1
BIN=./ft_ls

VERBOSE=0
DOLEAKS=0
for a in "$@"; do
	[ "$a" = "-v" ] && VERBOSE=1
	[ "$a" = "-l" ] && DOLEAKS=1
done

if [ ! -x "$BIN" ]; then
	echo "ft_ls not built -> run 'make' first." >&2
	exit 1
fi

# Booby-trapped tree (symlinks, permissions, sticky/setuid, old file, ...)
FIX=/tmp/ft_ls_test
sh tests/setup_tree.sh >/dev/null 2>&1

PASS=0
FAIL=0
FAILED=()

# Reference ls: the REAL GNU coreutils (the 42 evaluation target). On some
# machines /bin/ls is uutils/Rust, which diverges on edge cases (e.g. a
# non-traversable directory -> no error, no exit code 1). So we prefer
# /usr/bin/gnuls when present, invoked with argv[0]="ls" so its error prefix
# is "ls:" like ours. On a standard machine (where `ls` == GNU) we simply fall
# back to `command ls`.
if [ -x /usr/bin/gnuls ]; then
	refls() { bash -c 'exec -a ls /usr/bin/gnuls --color=never "$@"' ls "$@"; }
else
	refls() { command ls --color=never "$@"; }
fi

om=$(mktemp); om2=$(mktemp); em=$(mktemp); er=$(mktemp)

# run "description" args...
# Compares stdout BYTE FOR BYTE (files, not $(...)) so a trailing-newline
# difference is caught. Also compares stderr and the exit code.
run()
{
	desc="$1"
	shift
	LC_ALL=C "$BIN" "$@" >"$om"  2>"$em"; c_m=$?
	LC_ALL=C refls  "$@" >"$om2" 2>"$er"; c_r=$?
	if diff -q "$om" "$om2" >/dev/null 2>&1 && [ "$c_m" = "$c_r" ] \
		&& diff -q "$em" "$er" >/dev/null 2>&1; then
		PASS=$((PASS + 1))
	else
		FAIL=$((FAIL + 1))
		FAILED+=("$desc  [args: $*]")
		if [ "$VERBOSE" = "1" ]; then
			echo "------------------------------------------------------------"
			echo "FAIL: $desc   (args: $*)"
			[ "$c_m" != "$c_r" ] && echo "  exit: ft_ls=$c_m  ls=$c_r"
			if ! diff -q "$om" "$om2" >/dev/null 2>&1; then
				echo "  --- stdout diff (left=ft_ls) ---"
				diff "$om" "$om2" | head -20
			fi
			if ! diff -q "$em" "$er" >/dev/null 2>&1; then
				echo "  --- stderr diff (left=ft_ls) ---"
				diff "$em" "$er" | head -10
			fi
		fi
	fi
}

# ---- 1. targets / arguments -----------------------------------------------
run "no args"
run "dot"                 .
run "dotdot"              ..
run "single file"         Makefile
run "two files"           Makefile README.md
run "one dir"             src
run "two dirs"            src includes
run "file + dir"          Makefile src
run "absolute"            /etc
run "trailing slash"      src/
run "empty dir"           "$FIX/empty_dir"

# ---- 2. errors ------------------------------------------------------------
run "nonexistent"         nope_xyz
run "valid + invalid"     nope_xyz src
run "invalid option"      -z

# ---- 3. simple options ----------------------------------------------------
run "-a"   -a
run "-r"   -r
run "-t"   -t
run "-l"   -l
run "-R"   -R

# ---- 4. combinations ------------------------------------------------------
for o in -la -lr -lt -lR -ar -at -rt -tr -laR -lart -laRt; do
	run "$o (cwd)"  "$o"
	run "$o /etc"   "$o" /etc
done

# ---- 5. booby-trapped fixture (symlinks, perms, sticky, old file) ---------
run "fixture -la"   -la  "$FIX"
run "fixture -lR"   -lR  "$FIX"
run "fixture -laR"  -laR "$FIX"
run "fixture -lt"   -lt  "$FIX"

# ---- 5b. readable but non-traversable directory (chmod 600) ---------------
# readdir returns the names but lstat of each entry fails (EACCES). GNU then
# prints '?' in every -l/-i column, one "cannot access" error per entry on
# stderr, and exits 1. A short listing (no -l/-i) stats nothing -> no error,
# exit 0.
LOCK=/tmp/ft_ls_locked
rm -rf "$LOCK"; mkdir -p "$LOCK"; : >"$LOCK/alpha"; : >"$LOCK/beta"; printf 'x\n' >"$LOCK/gamma"
chmod 600 "$LOCK"
run "locked -l"      -l   "$LOCK"
run "locked -i"      -i   "$LOCK"
run "locked -li"     -li  "$LOCK"
run "locked -lir"    -lir "$LOCK"
run "locked -lt"     -lt  "$LOCK"
run "locked -lR"     -lR  "$LOCK"
run "locked short"        "$LOCK"
run "locked -R"      -R   "$LOCK"
chmod 700 "$LOCK"; rm -rf "$LOCK"

# ---- 6. big directories ---------------------------------------------------
run "big /usr/lib"  /usr/lib
run "big -la /usr/bin" -la /usr/bin

# ---- summary --------------------------------------------------------------
echo "============================================================"
echo "  PASS=$PASS   FAIL=$FAIL"
if [ "$FAIL" -gt 0 ]; then
	echo "  Failing cases:"
	for f in "${FAILED[@]}"; do
		echo "    - $f"
	done
	[ "$VERBOSE" = "0" ] && echo "  (re-run with -v to see the diffs)"
fi

# ---- optional valgrind ----------------------------------------------------
if [ "$DOLEAKS" = "1" ]; then
	echo "============================================================"
	if command -v valgrind >/dev/null 2>&1; then
		for args in "-laR $FIX" "-laR /etc"; do
			LC_ALL=C valgrind -q --leak-check=full --error-exitcode=42 \
				$BIN $args >/dev/null 2>/tmp/vg && \
				echo "  [OK leaks] $args" || echo "  [KO leaks] $args (see /tmp/vg)"
		done
	else
		echo "  valgrind not installed -> leak check skipped"
	fi
fi

rm -f "$om" "$om2" "$em" "$er"
[ "$FAIL" -eq 0 ]
