#!/bin/sh
# ============================================================================
#  compare.sh — compare la sortie de ./ft_ls avec le vrai ls
#
#  Usage:
#     ./tests/compare.sh [OPTIONS_ET_CHEMINS...]
#
#  Exemples:
#     ./tests/compare.sh -l /etc
#     ./tests/compare.sh -laR .
#     ./tests/compare.sh -lt /tmp /usr
#
#  Affiche un diff. Si rien ne s'affiche -> sorties identiques (parfait).
#  Tout est lance en LC_ALL=C, comme a l'evaluation.
# ============================================================================

BIN=./ft_ls

if [ ! -x "$BIN" ]; then
	echo "ft_ls n'est pas compile (lance 'make' d'abord)." >&2
	exit 1
fi

# Compare stdout ET le code de retour.
out_mine=$(LC_ALL=C "$BIN" "$@" 2>/tmp/ft_ls_err_mine); code_mine=$?
out_real=$(LC_ALL=C ls    "$@" 2>/tmp/ft_ls_err_real); code_real=$?

echo "===== STDOUT diff (gauche=ft_ls, droite=ls) ====="
diff <(printf '%s' "$out_mine") <(printf '%s' "$out_real") \
	&& echo "  [OK] stdout identique"

echo "===== STDERR diff ====="
diff /tmp/ft_ls_err_mine /tmp/ft_ls_err_real \
	&& echo "  [OK] stderr identique"

echo "===== CODE RETOUR ====="
echo "  ft_ls=$code_mine   ls=$code_real"
[ "$code_mine" = "$code_real" ] && echo "  [OK]" || echo "  [DIFF]"
