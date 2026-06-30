#!/bin/sh
# ============================================================================
#  setup_tree.sh — cree une arborescence piegeuse pour tester ft_ls
#
#  Genere /tmp/ft_ls_test avec : fichiers caches, sous-dossiers (pour -R),
#  liens symboliques (dont un casse), droits varies, sticky/setuid bits,
#  noms a tri delicat (majuscules/minuscules), fichier ancien (date > 6 mois).
#
#  Usage:
#     ./tests/setup_tree.sh
#     ./tests/compare.sh -laR /tmp/ft_ls_test
# ============================================================================

ROOT=/tmp/ft_ls_test
rm -rf "$ROOT"
mkdir -p "$ROOT"/sub1/sub2 "$ROOT"/empty_dir

# Fichiers ordinaires + tri majuscules/minuscules (LC_ALL=C => MAJ avant min)
printf 'a\n'   > "$ROOT/Bravo"
printf 'bb\n'  > "$ROOT/alpha"
printf 'ccc\n' > "$ROOT/Charlie"
printf 'd\n'   > "$ROOT/.cache_cache"      # fichier cache

# Tailles differentes pour tester l'alignement des colonnes en -l
head -c 100000 /dev/zero > "$ROOT/gros_fichier" 2>/dev/null

# Liens symboliques (-l doit afficher "lien -> cible")
ln -s alpha          "$ROOT/lien_ok"
ln -s n_existe_pas   "$ROOT/lien_casse"

# Droits varies
chmod 700 "$ROOT/Bravo"
chmod 644 "$ROOT/alpha"
chmod 755 "$ROOT/Charlie"

# Bits speciaux (sticky / setuid) -> caracteres s / t dans les droits
chmod +t  "$ROOT/empty_dir"     2>/dev/null
chmod u+s "$ROOT/gros_fichier"  2>/dev/null

# Contenu des sous-dossiers (pour -R)
printf 'x\n' > "$ROOT/sub1/file_in_sub1"
printf 'y\n' > "$ROOT/sub1/sub2/file_in_sub2"

# Fichier "ancien" -> ls doit afficher l'annee au lieu de l'heure
touch -d '2 years ago' "$ROOT/vieux_fichier" 2>/dev/null \
	|| touch -t 202001010000 "$ROOT/vieux_fichier" 2>/dev/null

echo "Arborescence de test prete : $ROOT"
echo "Teste avec :  ./tests/compare.sh -laR $ROOT"
