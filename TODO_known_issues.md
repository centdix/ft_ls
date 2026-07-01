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

**Statut : non résolu, à investiguer.**

Sur un dossier où plusieurs entrées ont **exactement le même mtime (à la
nanoseconde)**, notre ordre `-t` ne correspond pas à celui du vrai `ls`.

### Reproduction
```sh
diff <(./ft_ls -t /etc) <(LC_ALL=C ls -1t /etc)
```

### Preuve récoltée (`/etc/rc{0..4}.d`)
Les 5 dossiers ont un mtime strictement identique :
```
2026-05-11 09:55:07.093563939   rc0.d / rc1.d / rc2.d / rc3.d / rc4.d
```
Pourtant les ordres divergent :

| source            | ordre obtenu              |
|-------------------|---------------------------|
| nous (`ft_ls -t`) | rc0, rc1, rc2, rc3, rc4   | (fallback nom ascendant)
| vrai `ls -t`      | rc2, rc1, rc0, rc4, rc3   | (ni nom, ni readdir)
| `ls -f` (readdir) | rc2, rc4, rc3, rc1, rc0   |
| inodes            | 331..372 (pas monotone)   |

→ L'ordre du vrai `ls` sur égalité parfaite ne correspond **ni** au tri par
nom (asc/desc) **ni** à l'ordre readdir brut **ni** à l'inode. Comportement
GNU coreutils à creuser (mergesort stable + comparateur exact de `ls.c`,
`cmp_mtime` → fallback `cmp_name`… mais le résultat observé contredit ça).

### Ce qui marche déjà
- `-t`, `-tr`, `-r`, listing simple : **identiques** au vrai `ls` quand les
  mtimes sont distincts (ex. dossier courant, `/etc` en listing simple).
- Le souci est **isolé** aux clusters de fichiers au timestamp identique
  (artefacts d'installation de paquets).

### Piste de fix
Comprendre l'ordre de départage réel de GNU `ls` sur égalité totale, puis
reproduire. Vérifier aussi le comportement attendu en éval 42 (souvent des
dossiers de test sans timestamps identiques → peut ne jamais se voir).

---

## 2. Portabilité macOS : champ nanoseconde de `struct stat`

**Statut : OK sous Linux (cible actuelle), à adapter si test sur Mac.**

`cmp_mtime` (src/sort.c) lit les nanosecondes via `st.st_mtim.tv_nsec`
(orthographe **Linux / POSIX.1-2008**). Sur **macOS** le champ s'appelle
`st_mtimespec.tv_nsec`.

### Fix prévu (si besoin Mac)
```c
#ifdef __APPLE__
# define ST_MTIME_NSEC(st) ((st).st_mtimespec.tv_nsec)
#else
# define ST_MTIME_NSEC(st) ((st).st_mtim.tv_nsec)
#endif
```
et remplacer les accès `st.st_mtim.tv_nsec` par `ST_MTIME_NSEC(st)`.

Les machines de correction 42 sont sous Linux → non bloquant pour l'instant.

---

## 3. `lstat` non vérifié dans `ft_extract_entries` (à durcir)

**Statut : gap connu, edge case.**

`src/list.c`, `ft_extract_entries` : `lstat(file->path, &file->st);` ignore la
valeur de retour. Si `lstat` échoue (fichier disparu entre `readdir` et
`lstat`, permission sur le dossier parent perdue en cours de route…), `file->st`
reste **non initialisé** → champs `-l` aberrants, et **pas de code retour 1**.

Comportement GNU : affiche `?` dans les champs concernés (mode, liens, taille…)
et sort en **1**. À reproduire si on veut le pixel-perfect complet.

### Piste
Tester `if (lstat(...) < 0)` → marquer l'entrée (flag `staterr`), afficher `?`
dans `format.c`, et faire remonter un niveau d'erreur 1.

---

## 4. Préfixe du nom de programme figé à `ls:`

**Statut : choix assumé, conforme à l'éval.**

`ls_error` (list.c) et les messages d'option invalide (parse.c) préfixent
`ls:` en dur. GNU utilise `argv[0]` verbatim → lancé `./ft_ls`, GNU afficherait
`ft_ls:`. On matche la **référence `ls`** (invoquée `ls`), c'est ce que compare
l'éval 42. À garder tel quel sauf consigne contraire.

---

## 5. Allocations non vérifiées dans les entrées de dossier (à durcir)

**Statut : gap connu, sous pression mémoire uniquement.**

Dans `ft_extract_entries` / `ft_list_file_operands` :
- `ft_strdup(name)` et `path_join(...)` peuvent renvoyer `NULL` → `file->name`
  ou `file->path` NULL, déréférencés plus loin dans le tri/affichage.
- `ft_lstnew(file)` peut renvoyer `NULL` → le `t_file` fuit et n'est pas ajouté.

Les opérandes (parse.c, `add_operand`) et les `malloc(sizeof(t_file))` sont déjà
durcis ; ce sont les `strdup`/`lstnew` internes qu'il reste à couvrir. Rare, mais
à fermer pour un `malloc`-hardening complet.

---

## 6. Divers (non bloquant, hors périmètre mandatory)

- **`readdir` + `errno`** : NULL est traité comme fin de dossier sans distinguer
  une vraie erreur de lecture (errno). GNU distingue. Edge case.
- **Marqueur ACL `+`** : GNU ajoute `+` après les permissions pour un fichier
  avec ACL/attributs étendus. Non implémenté (bonus hors sujet). Pas de diff sur
  cette machine car `/dev`, `/etc`… n'ont pas d'ACL.
- **Tri dépendant de la locale** : on trie en ordre d'octets (`LC_ALL=C`). GNU en
  locale non-C trie autrement (casse, accents). L'éval tourne en `LC_ALL=C` →
  conforme là où ça compte.
- **Taille des blocs `total`** : on suppose `st_blocks / 2` (blocs 512 o →
  affichage en 1 Ko). Correct par défaut sous Linux ; divergerait avec
  `POSIXLY_CORRECT`/`BLOCK_SIZE` exotiques.
- **Options non implémentées** (hors mandatory `-l -R -a -r -t`) : `-1 -d -f -g
  -i -S -c -u`… À n'ajouter que si demandé.
