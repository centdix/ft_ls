# ft_ls — subtilités d'implémentation & comportement de `ls`

Document de référence pour comprendre (et défendre à l'oral) les choix de code
et les pièges du vrai `ls`. Il suit le trajet d'une exécution :
**parsing → stat des opérandes → tri → listing → formatting**, puis détaille les
points délicats transverses (`stat` vs `lstat`, portabilité GNU/BSD, gestion
d'erreurs, colonnes `?`, etc.).

Fichiers : `main.c`, `parse.c`, `sort.c`, `list.c`, `format.c`, `includes/ft_ls.h`.

---

## 0. Vue d'ensemble du pipeline (`main.c`)

L'ordre des étapes n'est pas anodin, il **reproduit celui de `ls`** :

1. `ft_parse_args` : parse les flags et construit la liste des opérandes
   (`t_path`), puis les `stat`.
2. `ft_print_access_errors` : imprime **d'abord** les erreurs d'accès
   (`ls: cannot access ...`) sur stderr, avant toute sortie normale.
3. `ft_sort_paths` : trie les opérandes (fichiers avant dossiers, puis clé de
   tri).
4. `ft_list_file_operands` : imprime **tous les fichiers-arguments d'abord**, en
   un seul bloc.
5. Boucle sur les opérandes de type dossier : `ft_list_one_dir` développe leur
   contenu (avec `-R` si demandé).
6. Code de retour agrégé (`err`).

Points clés :
- **Fichiers avant dossiers** : `ls a_dir a_file` affiche `a_file`, puis le bloc
  `a_dir:`. C'est l'ordre de `ls`, pas l'ordre d'argv.
- **`printed`** : booléen passé par pointeur qui sert à insérer la ligne vide de
  séparation **entre** deux blocs (jamais avant le premier).
- **`show_header`** : un en-tête `path:` n'apparaît que s'il y a plusieurs cibles
  OU `-R` sur une cible unique (nuance GNU/BSD, cf. §7).

---

## 1. Parsing des arguments (`parse.c`)

### 1.1 Séparation flags / opérandes

Dans `ft_parse_args`, chaque `argv[i]` est classé :

| Motif           | Interprétation                                  |
|-----------------|-------------------------------------------------|
| `--`            | fin des options ; tout ce qui suit = opérande   |
| `--xxx`         | option longue inconnue → erreur + exit          |
| `-xxx` (xxx≠∅)  | groupe d'options courtes (`parse_flags`)        |
| `-` seul        | **opérande** (nom de fichier `-`), pas une option |
| autre           | opérande                                        |

Subtilités :
- `-` **seul** reste un opérande (`argv[i][1]` est nul → on tombe dans le `else`).
  C'est le comportement de `ls -` (essaie de lister le fichier nommé `-`).
- `--` bascule le drapeau `endopt` : après lui, même `-l` devient un nom de
  fichier.
- Les options sont **groupables** : `-la` = `-l -a`. `parse_flags` boucle sur
  chaque caractère à partir de l'index 1.

### 1.2 `set_flag` : une lettre, un effet, et les implications croisées

Toutes les options passent par `set_flag`. Les cas non triviaux :

- **`-g` / `-o` impliquent `-l`** : `-g` = format long sans la colonne owner ;
  `-o` = format long sans la colonne group. Ils activent `opts->list`.
- **`-a` et `-A` s'annulent mutuellement** : le dernier vu gagne (`-a` remet
  `almost=0`, `-A` remet `all=0`), comme `ls`.
- **`-f`** force `all=1` (montre les cachés) **et** `sort=SORT_NONE` (ordre
  brut du `readdir`, pas de tri). Il ne désactive **pas** `-l` (conforme au `ls`
  actuel).
- **Clés de tri (`-t`, `-S`, `-f`)** s'écrasent dans l'ordre de lecture : la
  dernière rencontrée l'emporte, comme `ls`.
- **`-u` / `-c`** ne changent **pas** la clé de tri directement : ils changent
  le *champ temps* utilisé (`timek` : atime/ctime), qui sert à la fois pour
  l'affichage `-l` et pour le tri `-t`.

### 1.3 La subtilité `-u`/`-c` sans `-t` (post-traitement)

Après la boucle de parsing :

```170:171:src/parse.c
  if (ls.opts.timek != TK_MTIME && ls.opts.sort == SORT_NAME && !ls.opts.list)
    ls.opts.sort = SORT_TIME;
```

`ls -u` (sans `-t`, sans `-l`) **trie par atime**. Mais `ls -lu` trie **par
nom** (le `-u` ne change alors que la date affichée). C'est exactement le
comportement de `ls` : `-u`/`-c` n'impliquent le tri par temps que hors format
long.

### 1.4 Pourquoi le `stat` des opérandes est *différé*

`new_path` ne fait **aucun** `stat` à la création. On stat seulement à la fin
(`stat_operands`), une fois **toutes** les options connues :

```176:177:src/parse.c
  stat_operands(ls.operands, ls.opts.list || ls.opts.dironly);
```

Raison : les options peuvent **suivre** l'opérande dans argv
(`ls /bin -l`), et `-l`/`-d` changent la manière de stat les liens symboliques
passés en argument (déréférencer ou non — cf. §2). Il faut donc connaître les
flags avant de stat.

`op->path` pointe **dans argv** (pas de `strdup`), donc `ft_free_path` ne libère
que la structure, jamais la chaîne.

---

## 2. `stat` vs `lstat` — le cœur de la sémantique symlink

C'est LA subtilité que l'oral attend. Règle de `ls` :

- **`lstat`** décrit le **lien lui-même** (type `l`, taille = longueur de la
  cible, etc.).
- **`stat`** suit le lien et décrit sa **cible**.

Quand utilise-t-on l'un ou l'autre ?

### 2.1 Entrées *à l'intérieur* d'un dossier → toujours `lstat` (`list.c`)

```55:59:src/list.c
  // lstat, not stat: describe the link itself, not its target
  if (lstat(file->path, &file->st) != 0) {
    file->staterr = errno;
    ft_bzero(&file->st, sizeof(file->st));
  }
```

Dans un listing de dossier, un symlink s'affiche **en tant que lien** (`l` +
` -> cible`). Toujours `lstat`.

### 2.2 Opérandes (arguments en ligne de commande) → dépend des flags (`parse.c`)

`stat_operand(op, nofollow)` avec `nofollow = (-l || -d)` :

```100:116:src/parse.c
static void stat_operand(t_path *op, int nofollow) {
  size_t len;

  len = ft_strlen(op->path);
  if (nofollow && len > 0 && op->path[len - 1] != '/') {
    if (lstat(op->path, &op->st) != 0)
      op->staterr = errno;
    else if (S_ISDIR(op->st.st_mode))
      op->type = PATH_DIR;
    return;
  }
  if (stat(op->path, &op->st) == 0) {
    if (S_ISDIR(op->st.st_mode))
      op->type = PATH_DIR;
  } else if (lstat(op->path, &op->st) != 0)
    op->staterr = errno;
}
```

Trois subtilités empilées :

1. **`-l` / `-d` ne suivent PAS un symlink-argument** → `lstat`. `ls -l lien`
   affiche le lien, pas sa cible. Sans ces options, `ls` suit le lien.
2. **Slash final force le suivi** : `ls -l lien/` (avec `/`) **suit** quand même
   le lien (un `/` final signifie « le dossier pointé »). D'où le test
   `op->path[len-1] != '/'`.
3. **Fallback lien cassé** : en mode « suivre » (`stat`), si le `stat` échoue
   (lien cassé, cible absente), on retombe sur `lstat` pour **quand même
   afficher** le lien. `ls` fait pareil et **ne** retourne pas d'erreur (code 0)
   pour un symlink cassé passé en argument.

### 2.3 `PATH_DIR` seulement si c'est *vraiment* un dossier

`op->type` ne devient `PATH_DIR` que si `S_ISDIR`. Un symlink non déréférencé
(mode `-l`) reste `PATH_FILE` même s'il pointe vers un dossier → on l'affiche
comme entrée, on ne développe pas son contenu.

---

## 3. Tri (`sort.c`)

### 3.1 Tri stable par bulle, sur le *contenu* des nœuds

`ft_sort_list` / `ft_sort_paths` font un tri à bulles qui **échange le contenu**
des nœuds (`cur->content`), pas les nœuds eux-mêmes. Simple, pas de
réallocation, suffisant pour la taille d'un dossier.

`SORT_NONE` (`-f`) → **aucun tri**, on garde l'ordre brut de `readdir`.

### 3.2 Clé de tri + départage par nom

`cmp_files` :
- si `-t` : `cmp_time` (plus récent d'abord, **jusqu'à la nanoseconde**) ;
- si `-S` : `cmp_size` (plus gros d'abord) ;
- **égalité → départage par nom** (comparaison d'octets `ft_strncmp`).

Le tri par nom seul est le cas par défaut (`SORT_NAME`), qui tombe directement
sur le `ft_strncmp` final.

### 3.3 La direction `-r` est appliquée *par-dessus*

`-r` (`opts->rev`) **n'est pas** une clé : c'est une inversion appliquée après
coup au signe de la comparaison (`cmp = -cmp`). Ça marche pour toutes les clés
(nom, temps, taille) sans dupliquer la logique.

### 3.4 Comparaison temps à la nanoseconde

```28:32:src/sort.c
  if (ta != tb)
    return (ta > tb ? -1 : 1);
  if (pick_nsec(a, tk) != pick_nsec(b, tk))
    return (pick_nsec(a, tk) > pick_nsec(b, tk) ? -1 : 1);
  return (0);
```

On compare les secondes, puis **les nanosecondes** en cas d'égalité. C'est
nécessaire pour matcher `ls` sur des fichiers créés dans la même seconde. Les
champs nanoseconde diffèrent GNU/BSD → isolés derrière les macros `ST_*TIM_NS`
(cf. §6).

### 3.5 Tri des opérandes : fichiers avant dossiers

`cmp_paths` ajoute une règle en amont :

```89:91:src/sort.c
  // files before directories
  if (!opts->dironly && a->type != b->type)
    return ((int)a->type - (int)b->type);
```

`PATH_FILE (0) < PATH_DIR (1)` → les fichiers passent avant les dossiers dans la
liste d'arguments. Neutralisé par `-d` (où tout est traité comme entrée).

---

## 4. Listing d'un dossier (`list.c`)

### 4.1 Filtrage des entrées cachées (`show_entry`)

```26:35:src/list.c
static int show_entry(char *name, t_opts *opts) {
  if (name[0] != '.')
    return (1);
  if (opts->all)
    return (1);
  if (opts->almost && ft_strncmp(name, ".", 2) != 0 &&
      ft_strncmp(name, "..", 3) != 0)
    return (1);
  return (0);
}
```

- Pas de `.` initial → toujours affiché.
- `-a` → tout, y compris `.` et `..`.
- `-A` → les cachés **sauf** `.` et `..`.
- défaut → on cache les dotfiles.

### 4.2 `readdir` + `errno` : détecter une erreur de lecture

`ft_extract_entries` remet `errno = 0` avant/après chaque `readdir`. `readdir`
renvoie `NULL` à la fin **et** en cas d'erreur : on distingue les deux via
`errno` après la boucle. Si erreur → `ls: reading directory '<path>': ...`.

### 4.3 Erreurs `lstat` : *seulement si on a besoin de la méta* (`report_stat_errors`)

```212:214:src/list.c
  if (!(opts->list || opts->inode || opts->sort == SORT_TIME ||
        opts->sort == SORT_SIZE))
    return (0);
```

Subtilité GNU importante : un listing **court trié par nom** ne stat jamais les
entrées → une entrée qui a « disparu » entre `readdir` et `lstat` ne produit
**aucune** erreur. On n'émet `cannot access` que si une option a **réellement
besoin** de la métadonnée (`-l`, `-i`, tri temps/taille). Les erreurs sont
imprimées en ordre `readdir` (avant le tri).

### 4.4 Récursion `-R`

```257:265:src/list.c
  while (opts->rec && cur) {
    file = cur->content;
    if (S_ISDIR(file->st.st_mode) && ft_strncmp(file->name, ".", 2) != 0 &&
        ft_strncmp(file->name, "..", 3) != 0 &&
        ft_list_one_dir(file->path, opts, 1, printed, 0) > err)
      err = 1;
    cur = cur->next;
  }
```

- On descend dans les sous-dossiers **après** avoir imprimé le dossier courant
  (parcours en profondeur, dans l'ordre trié).
- On saute **explicitement** `.` et `..` (sinon récursion infinie).
- `is_arg = 0` pour les sous-dossiers : une erreur d'ouverture en récursion
  donne le code **1** (pas `RC_ERR`), cf. §7.

### 4.5 Codes de retour par contexte

`ft_list_one_dir` renvoie `is_arg ? RC_ERR : 1` sur échec `opendir` : un dossier
**argument** illisible → `RC_ERR` (GNU 2 / BSD 1) ; un **sous-dossier** en
récursion illisible → 1.

---

## 5. Formatting (`format.c`)

### 5.1 La chaîne de permissions (`mode_to_str`)

Construit `drwxr-xr-x` (10 chars). Le 1er char vient de `type_char` (`d`, `l`,
`c`, `b`, `p`, `s`, sinon `-`). Puis rwx pour user/group/other. Les bits
spéciaux **réécrivent le bit d'exécution** :
- setuid → `s` (ou `S` si pas d'exec) sur user ;
- setgid → `s`/`S` sur group ;
- sticky → `t`/`T` sur other.

### 5.2 La date : découpage manuel de `ctime()` (`time_str`)

`ctime` renvoie `"Www Mmm jj hh:mm:ss yyyy\n"`. On en extrait le champ de 12
chars façon `ls` :
- **récent** (< ~6 mois) → `"Mmm jj hh:mm"` ;
- **ancien ou futur** → `"Mmm jj  yyyy"`.

Le seuil `SIX_MONTHS = 15778476` secondes. Test `when > time(NULL)` = futur →
format année aussi.

### 5.3 Deux passes : calcul des largeurs puis impression

Le format long **aligne les colonnes**. Il faut donc :
1. `ft_calc_widths` : 1re passe, calcule la largeur max de chaque colonne
   (liens, owner, group, size, inode, major/minor) sur **tout** le bloc.
2. impression alignée (`print_right` pour le numérique, `print_left` pour
   owner/group).

C'est pour ça que les widths des **opérandes-fichiers** sont calculés sur
l'ensemble (fichiers + dossiers-arguments) avant impression, façon GNU.

### 5.4 Colonne « size » spéciale pour les devices

Pour un char/block device, la colonne taille contient `major, minor` au lieu de
la taille. `update_size_width` suit `major` et `minor` séparément, et
`ft_calc_widths` s'assure que `size >= major + 2 + minor` (les 2 = `", "`).

### 5.5 Symlink : `-> cible` via `readlink`

`print_symlink_target` fait un `readlink` (buffer 1024). Si échec, on n'affiche
rien (best-effort). N'est appelé que si `S_ISLNK`.

### 5.6 Colonnes `?` pour une entrée non-`lstat`-able (`print_long_unknown`)

Si `staterr` est posé (le fichier a disparu / perte de droits), on ne lit
**aucun** champ de `st` (il est mis à zéro). On imite GNU :
- 1er char depuis `d_type` du `readdir` (`dtype_char`), le reste = `?????????` ;
- `?` dans chaque colonne (liens, owner, group, size), date `?` calée à droite
  sur 12.

`update_widths_unknown` garantit qu'une telle entrée contribue au minimum `1`
de large par colonne, **sans** élargir avec un `st` zéroté (uid 0 = "root",
etc. — qu'il ne faut surtout pas laisser fausser les largeurs).

### 5.7 La ligne `total`

`sum_blocks` additionne `st_blocks` (unité 512 o). L'affichage passe par la
macro `TOTAL_BLOCKS` : **GNU divise par 2** (blocs de 1 Ko), **BSD non** (512
o). Cf. §6.

### 5.8 `nbr_to_str` maison

`ft_itoa` ne gère que `int`. Les tailles / inodes peuvent dépasser → on
convertit à la main (`unsigned long`).

---

## 6. Portabilité GNU/BSD (macros de `ft_ls.h`)

Tout ce qui diffère entre glibc/Linux et BSD/macOS est isolé derrière des
macros, pour que le reste du code soit **identique** sur les deux plateformes :

| Aspect                    | GNU / Linux                  | BSD / macOS               |
|---------------------------|------------------------------|---------------------------|
| Champs nanoseconde        | `st_atim.tv_nsec`            | `st_atimespec.tv_nsec`    |
| Ligne `total`             | blocs 1 Ko (`/2`)            | blocs 512 o (brut)        |
| Espace colonnes `-l`      | 1 espace (`COL_GAP`)         | 2 espaces                 |
| Colonne xattr `-l`        | réservée si besoin           | **toujours** réservée     |
| En-tête `-R` cible unique | oui (`REC_FORCES_HEADER`)    | non                       |
| Code retour erreur arg    | `2` (`RC_ERR`)               | `1`                       |
| Format msg erreur         | `ls: <raison> '<path>': err` | `ls: <path>: err`         |
| Msg option invalide       | quotes + `Try 'ls --help'`   | back-quotes + `usage:`    |
| Marqueur d'accès étendu   | `+` (ACL POSIX), `.` (ctx)   | `@` (dès qu'un xattr)     |

### 6.1 Le marqueur d'accès étendu (bonus, 11e colonne)

`ACL_CHAR` (sans suivre les liens, comme `lstat`) :
- **Linux** : `+` si ACL POSIX (`system.posix_acl_*`), `.` si contexte de
  sécurité seul (`security.selinux`/`SMACK64`), sinon espace.
- **macOS** : `@` dès qu'un xattr est présent (`listxattr`).

La colonne n'est imprimée que si `w->aclcol` est posé (GNU) ou toujours (BSD via
`ACL_COL_ALWAYS`).

---

## 7. Gestion d'erreurs & codes de retour

### 7.1 Format des messages (`ls_error`)

`ls: ` + (raison GNU) + quote + path + quote + `strerror(err)`. Les guillemets
et la présence de la raison changent GNU/BSD (macros `ERR_*`).

Trois raisons utilisées :
- `cannot access` : opérande non-statable (`ft_print_access_errors`) ou entrée
  disparue (`report_stat_errors`).
- `cannot open directory` : `opendir` échoue.
- `reading directory` : `readdir` échoue en cours de lecture.

### 7.2 Le préfixe `ls:` est figé (choix assumé)

On imprime `ls:` en dur (pas `argv[0]`) pour matcher exactement la référence
`ls` de l'évaluation. Lancé `./ft_ls`, le vrai GNU dirait `ft_ls:`. Cf.
`TODO_known_issues.md`.

### 7.3 Hiérarchie des codes retour

- `0` : succès.
- `1` : erreur dans un **sous-dossier** (récursion) ; erreur d'accès à un
  opérande (`ft_print_access_errors` → `RC_ERR`, qui vaut 1 en BSD, 2 en GNU).
- `RC_ERR` : erreur d'**argument/option** (GNU 2, BSD 1) ; dossier-argument
  illisible.

`main` agrège avec `err = max(err, ...)`.

---

## 8. Gestion mémoire

- `t_path->path` **pointe dans argv** → jamais libéré (`ft_free_path` ne libère
  que la struct).
- `t_file->name` et `t_file->path` sont **alloués** (`strdup`/`path_join`) →
  `ft_free_file` les libère.
- Toutes les chaînes temporaires du formatting (`owner_name`, `group_name`,
  `nbr_to_str`) sont libérées immédiatement après usage.
- Chemins d'échec d'allocation : on ne crée **jamais** un nœud de liste avec un
  `content` NULL (évite les déréférencements plus loin) ; on nettoie
  partiellement en cas d'échec (`make_entry`, `add_operand`).
- Objectif affiché : **0 fuite** (cf. `TODO_known_issues.md`).

---

## 9. Écarts résiduels connus (résumé)

Détaillés dans `TODO_known_issues.md` :
1. `+` des ACL POSIX **sur macOS** non géré (`<sys/acl.h>` hors liste autorisée) ;
   le `@` xattr l'est.
2. Pas de format **multi-colonnes** sans `-l` (une entrée/ligne) — explicitement
   exempté par le sujet.
3. `readdir`+`errno` best-effort (pas de code retour dédié) ; préfixe `ls:`
   figé ; tri en ordre d'octets (conforme en `LC_ALL=C`) ; `BLOCK_SIZE` /
   `POSIXLY_CORRECT` non gérés.
