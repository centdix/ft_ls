# Guide d'apprentissage — ft_ls

Recoder `ls` en C. Ce document explique les **mécanismes Unix/C réels** dont le projet
est fait, dans l'ordre où il faut les apprendre. La référence de comportement est
**la commande `ls` elle-même** (pas le man), à comparer avec `diff`.

---

## Le sujet en bref

Recoder `ls` avec **5 options obligatoires** et leurs combinaisons :

| Option | Effet |
|--------|-------|
| `-l` | format long (droits, liens, owner, group, taille, date, nom) |
| `-R` | récursif (descend dans les sous-dossiers) |
| `-a` | affiche aussi les fichiers cachés (`.` et `..` inclus) |
| `-r` | inverse l'ordre du tri |
| `-t` | trie par date de modification (au lieu de l'ordre alphabétique) |

Contraintes :
- Exécutable nommé `ft_ls`, **Makefile** obligatoire (`all`, `clean`, `fclean`, `re`, recompilation conditionnelle).
- **Zéro crash** (segfault / bus error / double free) et **zéro fuite mémoire** (tester avec `valgrind`).
- Comportement identique au vrai `ls`. Évaluation en `LC_ALL=C` → tri ASCII pur (ça simplifie).
- Pas besoin du multi-colonnes sans `-l`. Pas d'ACL/xattr obligatoire.
- Fonctions autorisées : `write`, `opendir`, `readdir`, `closedir`, `stat`, `lstat`,
  `getpwuid`, `getgrgid`, `listxattr`, `getxattr`, `time`, `ctime`, `readlink`,
  `malloc`, `free`, `perror`, `strerror`, `exit`.
- Conseil du sujet : ajoute `ft_printf` à ta libft, la vie est bien plus simple.

---

## Leçon 1 — Un dossier est juste une liste de noms

Un dossier (directory) n'est **pas** un conteneur de fichiers. C'est un fichier spécial
qui contient une liste de paires `(nom → numéro d'inode)`. L'inode est l'identifiant du
"vrai" fichier sur le disque (métadonnées + contenu).

`readdir` rend une `struct dirent` à chaque appel, qui contient **très peu** :
- `d_name` : le nom (tout ce dont `ls` sans option a besoin)
- `d_ino` : le numéro d'inode
- `d_type` : le type (8 = fichier régulier, 4 = dossier, 10 = symlink…)

```c
#include <dirent.h>
#include <stdio.h>

int main(void)
{
    DIR *dir = opendir(".");        // ouvre le dossier courant
    struct dirent *entry;

    if (!dir)
        return (1);
    while ((entry = readdir(dir)) != NULL)  // lit UNE entrée par appel
    {
        printf("inode=%-10lu  type=%d  name=%s\n",
            (unsigned long)entry->d_ino,
            entry->d_type,
            entry->d_name);
    }
    closedir(dir);
    return (0);
}
```

> ⚠️ **Pièges** : `readdir` rend les noms **dans l'ordre du disque, pas trié**, et inclut
> **`.` et `..`** (à n'afficher qu'avec `-a`). De plus `d_type` n'est pas fiable sur tous
> les systèmes de fichiers → pour `-l`, utilise **`lstat`**.

---

## Leçon 2 — `struct stat` : tout ce que `-l` affiche vient d'ici

Les droits, la taille, la date, le propriétaire ne sont pas dans le dossier : ils sont
dans l'**inode**. On les lit avec `lstat(chemin, &buffer)`.

```c
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    struct stat st;

    if (argc < 2) return (1);
    if (lstat(argv[1], &st) == -1) { perror("lstat"); return (1); }

    printf("st_mode  = %o (octal)\n", st.st_mode);   // type + droits
    printf("st_nlink = %lu\n", (unsigned long)st.st_nlink); // nb de liens durs
    printf("st_uid   = %u\n", st.st_uid);   // id proprietaire
    printf("st_gid   = %u\n", st.st_gid);   // id groupe
    printf("st_size  = %ld octets\n", (long)st.st_size);
    printf("st_blocks= %ld\n", (long)st.st_blocks);  // blocs 512o -> sert au 'total'
    printf("st_mtime = %ld\n", (long)st.st_mtime);   // date modif (epoch)
    return (0);
}
```

Correspondance complète d'une ligne `-l` :

```
-rw-r--r--   1   root   root   3   Jun 30 13:46   /etc/hostname
└─ st_mode  nlink  uid    gid  size   st_mtime        d_name
   (100644)        ↓getpwuid ↓getgrgid  ↓à formater
```

Le mode `-l` = lire `struct stat` + **traduire chaque champ**.

---

## Leçon 3 — Décoder `st_mode` avec des masques de bits

`st_mode` est un seul entier qui encode le **type** de fichier (bits de poids fort) et les
**permissions** (9 bits + 3 bits spéciaux). On extrait avec l'opérateur `&`.
C'est la notion C la plus importante du projet.

```c
#include <sys/stat.h>
#include <stdio.h>

void print_perms(mode_t m)
{
    char str[11] = "----------";

    // 1) Type de fichier (1er caractere)
    if (S_ISDIR(m))  str[0] = 'd';   // dossier
    else if (S_ISLNK(m)) str[0] = 'l';   // lien symbolique
    else if (S_ISCHR(m)) str[0] = 'c';   // peripherique caractere
    else if (S_ISBLK(m)) str[0] = 'b';   // peripherique bloc
    else if (S_ISFIFO(m)) str[0] = 'p';  // pipe
    else if (S_ISSOCK(m)) str[0] = 's';  // socket
    // sinon '-' (fichier regulier)

    // 2) Les 9 bits de permission, testes avec un masque
    if (m & S_IRUSR) str[1] = 'r';   // owner read
    if (m & S_IWUSR) str[2] = 'w';   // owner write
    if (m & S_IXUSR) str[3] = 'x';   // owner exec
    if (m & S_IRGRP) str[4] = 'r';   // group read
    if (m & S_IWGRP) str[5] = 'w';
    if (m & S_IXGRP) str[6] = 'x';
    if (m & S_IROTH) str[7] = 'r';   // others read
    if (m & S_IWOTH) str[8] = 'w';
    if (m & S_IXOTH) str[9] = 'x';
    printf("%s\n", str);
}
```

> **Bits spéciaux à ne pas oublier** (sinon points perdus en défense) : ils **remplacent**
> le caractère `x` :
> - `S_ISUID` → position 3 : `x`→`s`, ou `-`→`S` si pas exécutable
> - `S_ISGID` → position 6 : idem
> - `S_ISVTX` (sticky) → position 9 : `x`→`t`, `-`→`T` (ex. `/tmp` = `drwxrwxrwt`)

Principe à intégrer : **un masque de bits** (`m & S_IXUSR`) teste si un bit précis est
allumé. C'est le pattern réutilisé partout.

---

## Leçon 4 — UID/GID → noms, et formatage de la date

```c
#include <sys/stat.h>
#include <pwd.h>      // getpwuid
#include <grp.h>      // getgrgid
#include <time.h>     // ctime
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    struct stat st;
    if (argc < 2 || lstat(argv[1], &st) != 0) return (1);

    struct passwd *pw = getpwuid(st.st_uid);  // uid -> struct utilisateur
    struct group  *gr = getgrgid(st.st_gid);  // gid -> struct groupe
    printf("owner = %s\n", pw ? pw->pw_name : "?");
    printf("group = %s\n", gr ? gr->gr_name : "?");

    // ctime() rend: "Tue Jun 30 13:46:25 2026\n"
    char *full = ctime(&st.st_mtime);
    char buf[13];
    strncpy(buf, full + 4, 12);   // caracteres 4..15 -> "Jun 30 13:46"
    buf[12] = '\0';
    printf("format ls = %s\n", buf);
    return (0);
}
```

> `getpwuid`/`getgrgid` peuvent renvoyer `NULL` → dans ce cas le vrai `ls` affiche le
> **numéro brut** (à gérer).

> **Subtilité de la date** : `ls` n'affiche `Jun 30 13:46` que si le fichier a **moins de
> 6 mois**. Plus vieux (ou dans le futur) → l'heure est remplacée par **l'année** :
> `Jun 30  2023`. Règle : si `mtime ∈ [maintenant - 6 mois, maintenant]` → format heure,
> sinon format année. Calcul avec `time(NULL)` (6 mois ≈ `15552000` secondes).

---

## Leçon 5 — Les liens symboliques (`readlink`)

C'est la raison d'utiliser `lstat` et **pas** `stat` :
- `stat` suit le lien → infos de la **cible**.
- `lstat` → infos du **lien lui-même** (nécessaire pour afficher `l` et la flèche).

Avec `-l`, un symlink s'affiche `lrwxr-xr-x ... mon_lien -> /la/cible`.

```c
char target[1024];
ssize_t n = readlink(path, target, sizeof(target) - 1);
if (n != -1) { target[n] = '\0'; /* readlink ne met PAS le \0 */ }
```

---

## Leçon 6 — L'alignement des colonnes en `-l` (réfléchir avant de coder)

`ls -l` aligne chaque colonne sur la **largeur de sa plus grande valeur** :

```
-rw-r--r--  1 root root     3 ...
drwxr-xr-x 24 root root 12288 ...
            ↑           ↑
         nlink sur 2   size sur 5
```

On ne connaît ces largeurs qu'après avoir `lstat` **toutes** les entrées. D'où la
conséquence architecturale majeure :

> **Deux passes obligatoires** :
> 1. stat toutes les entrées → stocke-les dans un tableau/liste de structs + calcule les
>    largeurs max (nlink, taille, owner, group…).
> 2. affiche avec le padding calculé.

→ Stocke chaque entrée dans **ta propre struct** (ex. `t_file { char *name; struct stat st;
char *owner; ... }`) plutôt que d'afficher à la volée.

Le **`total N`** en tête de `ls -l` = **somme des `st_blocks`** de toutes les entrées.
`st_blocks` est en blocs de 512 o, mais `ls` affiche en blocs de 1024 o → **divise par 2**.

---

## Leçon 7 — La récursion `-R` (à concevoir AVANT de coder)

Pense `-R` dès le début, sinon refactorisation garantie. Structure mentale :

```
afficher_dossier(chemin):
    1. lire toutes les entrées (readdir) + lstat chacune
    2. trier
    3. afficher le bloc (avec header "chemin:" si -R ou plusieurs args)
    4. SI -R : pour chaque entrée qui est un dossier (sauf . et ..) :
                 afficher_dossier(chemin + "/" + nom)   <-- appel récursif
```

> Pièges : sépare bien « afficher le contenu d'un niveau » de « descendre ». Et **construis
> le chemin complet** (`dossier/sous-dossier`) pour la récursion et les `lstat`, car
> `readdir` ne donne que le nom court.

---

## Leçon 8 — Le tri et la combinaison des options

`readdir` ne trie pas. Par défaut `ls` trie par **ordre alphabétique ASCII** (en `LC_ALL=C` :
majuscules avant minuscules, simple `strcmp`).

- `-t` : trie par `st_mtime` décroissant (récent en premier). **Égalité → départage alpha.**
- `-r` : **inverse le résultat final**, quel que soit le tri.

→ Conçois **un seul comparateur** paramétré par les flags. Un tri stable simple (bulles)
suffit pour un projet 42 (optimisation = bonus perf).

---

## Leçon 9 — Gérer les erreurs « comme `ls` »

```
$ ls /n_existe_pas /etc/hostname
ls: cannot access '/n_existe_pas': No such file or directory
/etc/hostname
code retour = 2
```

- Le message va sur **stderr** (pas stdout). Format : `ls: cannot access 'X': ` + texte de
  `strerror(errno)`. C'est `errno` (positionné par `lstat`/`opendir`) qui donne le message.
- `ls` **ne s'arrête pas** au premier mauvais argument : il l'affiche et **continue**.
- **Code de retour** : `0` = ok, `2` = erreur grave. À reproduire.

---

## Synthèse — Ce que tu dois réellement apprendre

| Notion | Où l'apprendre | Pourquoi |
|--------|----------------|----------|
| `opendir`/`readdir`/`closedir` | `man 3 readdir` | parcourir un dossier |
| `struct stat` + `lstat` | `man 2 lstat` | **toutes** les métadonnées de `-l` |
| Masques de bits (`&`, `S_IFMT`, `S_IRUSR`…) | `man 7 inode`, `man 2 stat` | décoder type + permissions |
| Bits spéciaux setuid/setgid/sticky | `man 7 inode` | le `s`/`t` dans les droits |
| `getpwuid`/`getgrgid` | `man 3 getpwuid` | uid/gid → noms |
| `time`/`ctime` + règle des 6 mois | `man 3 ctime` | formater la date |
| `readlink` | `man 2 readlink` | cible des symlinks |
| `errno`/`strerror`/`perror` | `man 3 strerror` | erreurs façon `ls` |
| Chaînes / mémoire | ta libft | construire les chemins, padding, zéro leak |

**Les 3 vrais défis intellectuels** (le reste est mécanique) :
1. **La conception en 2 passes** (stat tout → calculer largeurs → afficher) pour `-l`.
2. **La récursion `-R`** propre, avec construction correcte des chemins.
3. **Le pixel-perfect** : matcher `ls` exactement (ordre, `total`, padding, dates, sticky bit, erreurs).

**Ordre de construction conseillé :**
`readdir` brut → tri + `-a` → `lstat` et décodage de `st_mode` → `-l` complet
(owner/group/date/size) → alignement des colonnes → `-r`/`-t` → `-R` → multi-arguments & erreurs.

**Outil de validation permanent :**
```sh
diff <(./ft_ls -lR /etc) <(ls -lR /etc)
```
Tant que `diff` n'est pas vide, ce n'est pas fini.

---

## Modèle mental complet

Un dossier = liste de noms+inodes (`readdir`) → chaque inode = métadonnées (`lstat`) →
`-l` = traduire ces champs avec des masques de bits et des fonctions de conversion.
