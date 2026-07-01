# ft_ls — known issues / à investiguer plus tard

Notes de bugs connus et points en suspens, à reprendre pendant la passe
"pixel-perfect" finale. Aucun n'est bloquant pour l'avancement actuel.

---

## 0. État vérifié — conformité GNU coreutils 9.7

**Statut : 44/44 identiques à GNU** (stdout + stderr + code retour).

Le `/bin/ls` de cette machine est **uutils (Rust)**, pas GNU → les comparaisons
via `ls` nu sont trompeuses. Le **vrai GNU** est installé à **`/usr/bin/gnuls`**
(paquet `gnu-coreutils`). Pour comparer en neutralisant le préfixe du nom de
programme (GNU imprime `argv[0]` verbatim dans ses erreurs) :

```sh
# invoque GNU avec argv[0]="ls" -> messages prefixes "ls:" comme les notres
gnu() { bash -c 'exec -a ls /usr/bin/gnuls --color=never "$@"' ls "$@"; }
diff <(./ft_ls "$@") <(LC_ALL=C gnu "$@")
```

### Code retour (modèle GNU reproduit — corrigé cette passe)
- erreur sur un **argument** (inexistant, dossier non ouvrable) → **2**
- erreur sur un **sous-dossier** rencontrée en récursion `-R` → **1**
- symlink cassé en `-l`, dossier vide, etc. → **0**
- Le max des niveaux remonte. `ft_list_one_dir` prend `is_arg` pour distinguer
  opérande (échec = 2) et sous-dossier (échec = 1). Avant, les erreurs de
  récursion étaient affichées mais **jamais propagées** au code retour.

---

## 1. Tri `-t` : départage des fichiers ayant un mtime IDENTIQUE

**Statut : RÉSOLU — aucun bug vs GNU. Faux positif (comparaison uutils).**

L'ancienne divergence venait d'une comparaison contre le `ls` **uutils/Rust**
(`/bin/ls` de cette machine), **pas** contre GNU. Réenquête faite contre le
**vrai GNU** (`/usr/bin/gnuls`, invoqué `ls`, `LC_ALL=C`).

### Preuve (`/etc/rc?.d`, tous au mtime `2026-05-11 09:55:07.093563939`)
| source              | ordre sur égalité parfaite      |
|---------------------|----------------------------------|
| **GNU `ls -t`**     | rc0, rc1, rc2, rc3, rc4… (**nom asc**) |
| **nous `ft_ls -t`** | rc0, rc1, rc2, rc3, rc4… (**nom asc**) → identique |
| Rust/uutils `ls -t` | rc5, rc3, rc1, rc6, rc4, rc2, rc0 (readdir/inode) |

→ GNU départage bien par **nom ascendant** sur mtime identique, exactement
comme notre `cmp_files` (fallback `ft_strncmp` sur le nom). Vérifié identique
à GNU sur `-t -rt -tr -lt -ltr /etc` et `-t /usr/lib` (dossiers riches en
clusters de timestamps identiques). Le fallback name est **stable** et correct.

Rien à corriger : notre cible d'évaluation est GNU coreutils.

---

## 2. Portabilité macOS

**Statut : VALIDÉ SUR MAC (compilé + diff BSD `/bin/ls`, 2026-07-01).**

Toutes les divergences glibc/Linux ↔ BSD/macOS sont isolées derrière des
macros `#ifdef __APPLE__` dans `includes/ft_ls.h` :

- **Champs temporels** : `ST_ATIM_S/NS`, `ST_MTIM_S/NS`, `ST_CTIM_S/NS`
  → `st_atim.*` (Linux) vs `st_atimespec.*` (mac). Utilisés dans `sort.c`
  (`ft_pick_time`, `pick_nsec`) et `format.c`.
- **`major`/`minor`** : `<sys/sysmacros.h>` inclus **seulement** hors mac
  (sur mac ils viennent de `<sys/types.h>`).
- **xattr / marqueur du 11e champ** : macros `X_HAS` (un xattr précis existe :
  `lgetxattr` Linux / `getxattr`+`XATTR_NOFOLLOW` mac), `X_LIST` (liste des
  xattr : `llistxattr` / `listxattr`), et `ACL_CHAR(f)` → `@` si xattr présent
  sous mac (BSD), `+`/`.` ACL/contexte POSIX sous Linux (GNU). Colonne toujours
  réservée sur mac via `ACL_COL_ALWAYS`. Le `+` des **ACL** BSD reste hors
  périmètre (voir §0 « Marqueur du 11e champ »).
- **Ligne `total`** : macro `TOTAL_BLOCKS` → `st_blocks / 2` (GNU, blocs 1 Ko)
  vs `st_blocks` (BSD/mac, blocs 512 o).
- **Espacement des colonnes `-l`** : macro `COL_GAP` → 2 espaces owner→group /
  group→size (BSD) vs 1 espace (GNU). Corrige un écart de padding constaté sur
  Mac (`print_owner_group`, `format.c`).
- **En-tête `-R` d'un dossier-opérande unique** : macro `REC_FORCES_HEADER` →
  0 (BSD n'imprime pas de header `chemin:` pour un seul dossier) vs 1 (GNU le
  fait dès `-R`). Utilisée dans `main.c` (`show_header`).
- **Gestion d'erreur (code retour + messages)** : macros `RC_ERR` (BSD 1 /
  GNU 2), `ERR_REASON`/`ERR_Q1`/`ERR_Q2` (format `ls: <chemin>: <err>` en BSD
  vs `ls: <motif> '<chemin>': <err>` en GNU), et `OPT_SQ1/2`, `OPT_LQ1/2`,
  `OPT_HELP` (message d'option invalide + 2ᵉ ligne `usage:` BSD / `Try 'ls
  --help'` GNU). Dans `list.c` (`ls_error`), `parse.c` (erreurs d'option),
  `main.c`. ⚠️ La chaîne `usage:` de `OPT_HELP` (BSD) dépend de la version de
  macOS — à revérifier si la machine d'éval diffère.

### Validation Mac 2026-07-01 (compilé, comparé à `/bin/ls` BSD, `LC_ALL=C`)
- `make` clean (`-Wall -Wextra -Werror`, 0 warning) : `st_*timespec`,
  `getxattr` 6 args + `XATTR_NOFOLLOW`, `major`/`minor` OK.
- `ls`, `-a`, `-r`, `-t`, `-1`, `-rt`, `-R`, `-aR` : **identiques** au BSD ls.
- `-l`, `-la`, `-lR` : **strictement identiques** au BSD ls, **marqueur `@`
  inclus** (`COL_GAP` + colonne ACL toujours réservée + `@` via `listxattr`).
  Vérifié sur `/bin`, `/usr/bin` (980 entrées), `/usr/lib`, `/etc`,
  `/System/Library/CoreServices`, fichiers uniques, et `-lR` récursif.
- **Erreurs** (`nexiste_pas`, `-Z`, `--badopt`, permission refusée) : message
  **et** code retour **identiques** au BSD ls (fix gestion d'erreur OS-conditionnelle).

### Marqueur du 11e champ en `-l` (`@` / `+`)
- **`@` (extended attributes)** : **FAIT** sur mac via `listxattr` (macro
  `X_LIST` + `ACL_CHAR`). La colonne est toujours réservée (`ACL_COL_ALWAYS`),
  exactement comme le BSD ls. Rend `-l` pixel-perfect sur les fichiers à xattr.
- **`+` (ACL POSIX)** : **hors périmètre irréductible**. Sur macOS les ACL ne
  sont PAS exposées par `listxattr`/`getxattr` ; il faut `acl_get_file()` de
  `<sys/acl.h>`, **absente de la liste de fonctions autorisées** du sujet (qui
  exempte d'ailleurs explicitement les ACL). Conséquence : les rares entrées à
  ACL (le home `~`, certains dossiers système) affichent `+` chez BSD et `' '`
  chez nous. Seul écart résiduel, assumé et défendable à l'oral.

---

## 3. `lstat` non vérifié dans `ft_extract_entries`

**Statut : SÉCURISÉ (plus d'UB). Affichage `?` exact + rc 1 : POSTPONÉ.**

`make_entry` (`src/list.c`) teste désormais `lstat` : en cas d'échec, `st` est
**mis à zéro** (`ft_bzero`) → plus aucune lecture de champ non initialisé
(vérifié valgrind : 0 erreur sur un dossier `r--` où tous les `lstat` échouent).

Ce qui reste (volontairement postponé, edge case quasi introuvable en éval) :
GNU, lui, affiche des `?` dans chaque colonne `-l` (en tirant le type de
`dirent->d_type`), imprime `ls: cannot access '<path>': ...` par entrée, et
sort en **1**. Nous affichons à la place des champs à zéro
(`---------- 0 root root 0 Jan  1 1970 nom`), sans message ni rc 1.

### Piste (si pixel-perfect voulu un jour)
Flag `staterr` sur `t_file` → dans `format.c`, remplacer chaque colonne par
`?` (type via `d_type`), largeur `?` = 1 ; émettre l'erreur par entrée ;
propager un niveau d'erreur 1.

---

## 4. Préfixe du nom de programme figé à `ls:`

**Statut : choix assumé, conforme à l'éval.**

`ls_error` (list.c) et les messages d'option invalide (parse.c) préfixent
`ls:` en dur. GNU utilise `argv[0]` verbatim → lancé `./ft_ls`, GNU afficherait
`ft_ls:`. On matche la **référence `ls`** (invoquée `ls`), c'est ce que compare
l'éval 42. À garder tel quel sauf consigne contraire.

---

## 5. Allocations non vérifiées dans les entrées de dossier

**Statut : FAIT.**

- `make_entry` (contenu de dossier) et `make_operand` (opérandes) vérifient
  `malloc` + `ft_strdup`/`path_join` : si l'une échoue, le `t_file` partiel est
  libéré (`ft_free_file`, tolérant aux `NULL`) et renvoie `NULL`.
- Boucles appelantes : `ft_lstnew` vérifié. En contenu de dossier, un échec
  abandonne proprement le listing (`ft_lstclear` + `return NULL`) ; en bloc
  d'opérandes, l'entrée est simplement sautée. Aucun deref de `NULL`, aucune
  fuite du `t_file` non inséré.

---

## 6. Divers

- **`readdir` + `errno`** : **FAIT**. `errno` est remis à 0 avant chaque
  lecture ; si `readdir` renvoie NULL avec `errno != 0` (vraie erreur de
  lecture, pas fin de dossier), on émet `ls: reading directory '<path>': ...`.
  (rc dédié non propagé — best-effort, cas très rare.)
- **Marqueur ACL `+`** : **FAIT** (voir commit ACL). `+` (ACL POSIX) / `.`
  (contexte sécurité), colonne réservée par bloc.
- **Options `-1 -A -d -f -g -o -i -S -c -u -p`** : **FAITES** (voir commit
  options). Toutes vérifiées identiques à GNU.
- **Tri dépendant de la locale** : on trie en ordre d'octets (`LC_ALL=C`). GNU en
  locale non-C trie autrement (casse, accents). L'éval tourne en `LC_ALL=C` →
  conforme là où ça compte.
- **`total` / `BLOCK_SIZE` exotiques** : `TOTAL_BLOCKS` gère Linux (1 Ko) et
  mac (512 o) ; on ne gère pas `POSIXLY_CORRECT`/`BLOCK_SIZE` custom (hors sujet).
