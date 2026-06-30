#!/usr/bin/env bash
# ============================================================================
#  run_all.sh — matrice de regression : compare ft_ls au vrai ls sur plein
#  d'invocations, et affiche un resume PASS/FAIL.
#
#  Pour chaque cas on verifie : stdout identique, stderr identique, meme code
#  retour. Tout est lance en LC_ALL=C (comme a l'evaluation).
#
#  Usage:
#     ./tests/run_all.sh            # resume compact
#     ./tests/run_all.sh -v         # affiche le diff des cas qui echouent
#     ./tests/run_all.sh -l         # lance aussi un check valgrind (si dispo)
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
	echo "ft_ls pas compile -> 'make' d'abord." >&2
	exit 1
fi

# Arborescence piegeuse
FIX=/tmp/ft_ls_test
sh tests/setup_tree.sh >/dev/null 2>&1

PASS=0
FAIL=0
FAILED=()

# ls de reference : binaire reel (pas d'alias du shell) + couleur desactivee.
REF_LS="command ls --color=never"

om=$(mktemp); om2=$(mktemp); em=$(mktemp); er=$(mktemp)

# run "description" args...
# Compare stdout OCTET POUR OCTET (fichiers, pas $(...)) -> detecte les
# differences de newline final. Compare aussi stderr et le code retour.
run()
{
	desc="$1"
	shift
	LC_ALL=C "$BIN"  "$@" >"$om"  2>"$em"; c_m=$?
	LC_ALL=C $REF_LS "$@" >"$om2" 2>"$er"; c_r=$?
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
				echo "  --- stdout diff (gauche=ft_ls) ---"
				diff "$om" "$om2" | head -20
			fi
			if ! diff -q "$em" "$er" >/dev/null 2>&1; then
				echo "  --- stderr diff (gauche=ft_ls) ---"
				diff "$em" "$er" | head -10
			fi
		fi
	fi
}

# ---- 1. cibles / arguments -------------------------------------------------
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

# ---- 2. erreurs ------------------------------------------------------------
run "nonexistent"         nope_xyz
run "valid + invalid"     nope_xyz src
run "invalid option"      -z

# ---- 3. options simples ----------------------------------------------------
run "-a"   -a
run "-r"   -r
run "-t"   -t
run "-l"   -l
run "-R"   -R

# ---- 4. combinaisons -------------------------------------------------------
for o in -la -lr -lt -lR -ar -at -rt -tr -laR -lart -laRt; do
	run "$o (cwd)"  "$o"
	run "$o /etc"   "$o" /etc
done

# ---- 5. fixture piegeuse (symlinks, droits, sticky, vieux fichier) ---------
run "fixture -la"   -la  "$FIX"
run "fixture -lR"   -lR  "$FIX"
run "fixture -laR"  -laR "$FIX"
run "fixture -lt"   -lt  "$FIX"

# ---- 6. gros dossiers ------------------------------------------------------
run "big /usr/lib"  /usr/lib
run "big -la /usr/bin" -la /usr/bin

# ---- resume ----------------------------------------------------------------
echo "============================================================"
echo "  PASS=$PASS   FAIL=$FAIL"
if [ "$FAIL" -gt 0 ]; then
	echo "  Cas en echec :"
	for f in "${FAILED[@]}"; do
		echo "    - $f"
	done
	[ "$VERBOSE" = "0" ] && echo "  (relance avec -v pour voir les diffs)"
fi

# ---- valgrind optionnel ----------------------------------------------------
if [ "$DOLEAKS" = "1" ]; then
	echo "============================================================"
	if command -v valgrind >/dev/null 2>&1; then
		for args in "-laR $FIX" "-laR /etc"; do
			LC_ALL=C valgrind -q --leak-check=full --error-exitcode=42 \
				$BIN $args >/dev/null 2>/tmp/vg && \
				echo "  [OK leaks] $args" || echo "  [KO leaks] $args (voir /tmp/vg)"
		done
	else
		echo "  valgrind non installe -> check fuites saute"
	fi
fi

rm -f "$om" "$om2" "$em" "$er"
[ "$FAIL" -eq 0 ]
