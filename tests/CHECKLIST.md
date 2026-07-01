# ft_ls — checklist de tests / edge cases

Référence de comportement = le **vrai `ls`**, comparé en `LC_ALL=C` (tri ASCII).
Outils : `tests/run_all.sh` (matrice auto), `tests/compare.sh` (1 cas, diff détaillé),
`tests/leaks.sh` (valgrind), `tests/setup_tree.sh` (arbo piégeuse).

Pour chaque cas : **stdout identique**, **stderr identique**, **code retour identique**,
**0 fuite** (valgrind).

---

## 1. Cibles / arguments
- [ ] aucun argument (liste `.`)
- [ ] `.` et `..`
- [ ] un fichier simple (`ls Makefile` → affiche `Makefile`, pas une erreur)
- [ ] plusieurs fichiers
- [ ] un dossier
- [ ] plusieurs dossiers → headers `nom:` + ligne vide entre, **args triés**
- [ ] mélange fichier + dossier → **fichiers d'abord (triés), puis dossiers**
- [ ] chemin absolu (`/etc`, `/usr/lib`)
- [ ] chemin avec slash final (`src/`)
- [ ] dossier vide
- [ ] dossier sans droit de lecture (`chmod 000`) → erreur + continue
- [ ] dossier lisible mais non-traversable (`chmod 600`) → noms listés, mais
      `-l`/`-i` : colonnes `?`, `ls: cannot access` par entrée, rc 1 ; court : rc 0

## 2. Erreurs (Lesson 9)
- [ ] chemin inexistant → `ls: cannot access 'X': No such file or directory` sur **stderr**
- [ ] code retour **2** si erreur, **0** sinon
- [ ] arg inexistant **+** arg valide → affiche l'erreur ET continue le valide
- [ ] permission refusée → `Permission denied` (texte de `strerror(errno)`)
- [ ] ordre : erreurs d'accès aux args AVANT le reste, façon `ls`

## 3. Options simples
- [ ] `-a` (affiche `.` `..` et cachés)
- [ ] `-r` (ordre inversé)
- [ ] `-t` (tri par mtime, récent d'abord)
- [ ] `-l` (format long)
- [ ] `-R` (récursif)
- [ ] option inconnue (`-z`) → `ls: invalid option -- 'z'` + usage, code 2

## 4. Combinaisons d'options
- [ ] `-la` `-lr` `-lt` `-lR` `-ar` `-at` `-rt` `-tr`
- [ ] `-laR` `-lart` `-laRt`
- [ ] options collées vs séparées (`-la` == `-l -a`)
- [ ] `--` (fin des options) si géré

## 5. Format long `-l` (le gros morceau)
- [ ] type + 9 droits (`-rwxr-xr-x`)
- [ ] bits spéciaux : setuid `s`/`S`, setgid `s`/`S`, sticky `t`/`T`
- [ ] nb de liens (`st_nlink`)
- [ ] owner / group (numéro si `getpwuid`/`getgrgid` renvoie NULL)
- [ ] taille (`st_size`)
- [ ] date : format heure si < 6 mois, **année** sinon
- [ ] ligne `total N` (somme `st_blocks` / 2)
- [ ] **alignement des colonnes** (largeur max par colonne)
- [ ] symlink : `lien -> cible` (+ lien cassé)
- [ ] devices (char/block) : afficher major/minor au lieu de la taille
- [ ] `+`/`@` ACL/xattr après les droits (listxattr) — bonus
- [ ] entrée non-statable (`lstat` échoue) : `<type>????????? ? ? ? ? ? nom`
      (type via `d_type`), colonnes largeur 1, date `?` cadrée à droite sur 12

## 6. Tri (LC_ALL=C)
- [ ] alpha ASCII (MAJUSCULES avant minuscules)
- [ ] `-t` mtime décroissant, égalité → nom
- [ ] `-t` à la **nanoseconde** (cf. TODO_known_issues #1)
- [ ] `-r` inverse quel que soit le tri
- [ ] tri des **arguments** eux-mêmes (multi-cibles)

## 7. Récursion `-R`
- [ ] header `chemin:` pour chaque dossier visité
- [ ] descend dans les sous-dossiers (sauf `.` et `..`)
- [ ] construction correcte du chemin (`dir/sous/...`)
- [ ] ordre : contenu du niveau, puis sous-dossiers (triés)
- [ ] `-R` sur dossier vide / feuille
- [ ] `-laR` combiné
- [ ] pas de boucle infinie sur symlink de dossier

## 8. Mémoire / robustesse
- [ ] 0 fuite valgrind sur `-laR` (gros dossier : `/etc`, `/usr`)
- [ ] 0 crash (segfault / double free / bus error)
- [ ] `malloc` qui échoue géré proprement (pas de deref NULL)
- [ ] très grand dossier (`/usr/bin`, `/usr/lib`)

## 9. Norme 42
- [ ] norminette OK sur `src/`, `includes/`, `libft/` modifiés
- [ ] pas de déclaration+init sur la même ligne
- [ ] déclarations en haut de fonction
- [ ] ≤ 25 lignes/fonction, ≤ 5 fonctions/fichier, etc.

## 10. Makefile
- [ ] `make`, `clean`, `fclean`, `re`
- [ ] recompilation conditionnelle (pas de recompile inutile)
- [ ] pas de re-link si rien n'a changé
- [ ] `$(NAME)` = `ft_ls`, flags `-Wall -Wextra -Werror`

---

### Cas difficiles à diff automatiquement
- **Dates `-l`** : OK tant que `ft_ls` et `ls` tournent au même instant.
- **Largeurs de colonnes** : dépendent du contenu réel du dossier → diff direct suffit.
- **mtime identiques à la nanoseconde** : voir `TODO_known_issues.md` #1.
- **Permissions/owner** : lancer certains cas en tant qu'utilisateur normal (pas root,
  sinon les droits sont contournés).
