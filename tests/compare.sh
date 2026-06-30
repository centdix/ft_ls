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
#
#  NB: on compare stdout OCTET POUR OCTET (via fichiers temporaires), donc une
#  difference de newline final est detectee. On force `command ls` sans couleur
#  pour ignorer un eventuel alias `ls --color=...` du shell interactif.
# ============================================================================

BIN=./ft_ls

if [ ! -x "$BIN" ]; then
	echo "ft_ls n'est pas compile (lance 'make' d'abord)." >&2
	exit 1
fi

# ls de reference : binaire reel (pas d'alias) + couleur desactivee.
REF_LS="command ls --color=never"

out_mine=$(mktemp); err_mine=$(mktemp)
out_real=$(mktemp); err_real=$(mktemp)

LC_ALL=C "$BIN"   "$@" >"$out_mine" 2>"$err_mine"; code_mine=$?
LC_ALL=C $REF_LS  "$@" >"$out_real" 2>"$err_real"; code_real=$?

echo "===== STDOUT diff (gauche=ft_ls, droite=ls) ====="
diff "$out_mine" "$out_real" && echo "  [OK] stdout identique"

echo "===== STDERR diff (gauche=ft_ls, droite=ls) ====="
diff "$err_mine" "$err_real" && echo "  [OK] stderr identique"

echo "===== CODE RETOUR ====="
echo "  ft_ls=$code_mine   ls=$code_real"
[ "$code_mine" = "$code_real" ] && echo "  [OK]" || echo "  [DIFF]"

rm -f "$out_mine" "$err_mine" "$out_real" "$err_real"
