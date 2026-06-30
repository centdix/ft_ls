# Cheat sheet — fonctions utilisables dans `ft_ls`

Pour chaque fonction : **prototype** (avec types), **entrées**, **valeur de retour**.
Trois sources : (1) fonctions système autorisées par le sujet, (2) ta `libft`, (3) ton `ft_printf`.

---

## 0) Les types et les structs (à lire en premier)

### Les types "bizarres" sont juste des entiers déguisés

Tous ces types (`mode_t`, `off_t`…) sont des `typedef` vers des entiers. Tu peux les
manipuler comme des nombres ; il faut juste les **caster** au bon format pour `printf`.

| Type | C'est quoi, vraiment | Sert à | Cast conseillé pour l'afficher |
|------|----------------------|--------|--------------------------------|
| `size_t` | entier non signé | tailles / longueurs | `%zu` |
| `ssize_t` | entier signé | retour de `write`/`read`/`readlink` (peut être `-1`) | `%zd` |
| `mode_t` | entier non signé | `st_mode` (type + droits) | `%o` (octal) |
| `nlink_t` | entier | `st_nlink` (nb liens durs) | `(unsigned long)` puis `%lu` |
| `uid_t` / `gid_t` | entier non signé | id user / groupe | `%u` |
| `off_t` | entier signé (souvent 64 bits) | `st_size` (taille en octets) | `(long long)` puis `%lld` |
| `blkcnt_t` | entier signé | `st_blocks` (blocs de 512 o) | `(long long)` puis `%lld` |
| `time_t` | entier signé | dates (secondes depuis 1970) | `(long)` puis `%ld` |
| `ino_t` | entier non signé | `d_ino` (numéro d'inode) | `(unsigned long)` puis `%lu` |

### Les 3 structs que tu vas vraiment utiliser

```c
/* <dirent.h> — ce que readdir() te rend (1 entrée du dossier) */
struct dirent {
    ino_t          d_ino;       /* numero d'inode                       */
    unsigned char  d_type;      /* type (DT_DIR, DT_REG…) — PAS fiable  */
    char           d_name[256]; /* <-- LE NOM (tout ce qu'il te faut)   */
    /* …autres champs ignorables… */
};
```

```c
/* <sys/stat.h> — ce que lstat() remplit : TOUTES les infos de "-l" */
struct stat {
    mode_t    st_mode;    /* type de fichier + permissions (masques)   */
    nlink_t   st_nlink;   /* nombre de liens durs                      */
    uid_t     st_uid;     /* id du proprietaire  -> getpwuid()         */
    gid_t     st_gid;     /* id du groupe        -> getgrgid()         */
    off_t     st_size;    /* taille en octets                          */
    blkcnt_t  st_blocks;  /* nb de blocs de 512o -> sert au "total"    */
    time_t    st_mtime;   /* date de derniere modification             */
    /* …st_atime, st_ctime, st_ino, st_dev… */
};
```

```c
/* <pwd.h> et <grp.h> — pour traduire uid/gid en noms */
struct passwd { char *pw_name; /* … */ };   /* getpwuid(st.st_uid)->pw_name */
struct group  { char *gr_name; /* … */ };   /* getgrgid(st.st_gid)->gr_name */
```

> ⚠️ `getpwuid` / `getgrgid` peuvent rendre `NULL` → afficher alors le **numéro brut**.

### Tes structs à toi

```c
/* fournie dans includes/ft_ls.h : transporte les flags actifs */
typedef struct s_opts {
    int  list;   /* -l */   int  rec;   /* -R */   int  all;  /* -a */
    int  rev;    /* -r */   int  time;  /* -t */
} t_opts;

/* le maillon de liste de ta libft */
typedef struct s_list {
    void           *content;   /* tu y mettras un (t_file *) par ex.    */
    struct s_list  *next;
} t_list;

/* conseillé : stocke chaque entrée dans TA struct (cf. Leçon 6) */
typedef struct s_file {
    char         *name;    /* d_name copie (ft_strdup)                  */
    char         *path;    /* chemin complet (dir + "/" + name)         */
    struct stat   st;      /* rempli par lstat                          */
} t_file;
```

### Comment afficher chaque champ sans warning

```c
struct stat st;
lstat(path, &st);

ft_printf("mode  octal : %o\n",  st.st_mode);
ft_printf("nlink       : %lu\n", (unsigned long)st.st_nlink);
ft_printf("uid / gid   : %u %u\n", st.st_uid, st.st_gid);
ft_printf("size        : %lld\n", (long long)st.st_size);
ft_printf("blocks      : %lld\n", (long long)st.st_blocks);
ft_printf("mtime       : %ld\n",  (long)st.st_mtime);
```

---

## 1) Fonctions système autorisées

> Les seules fonctions externes que tu as le droit d'appeler.

### Écriture / sortie

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `write` | `ssize_t write(int fd, const void *buf, size_t count)` | `fd` (1=stdout, 2=stderr), buffer, nb d'octets | `ssize_t` : octets écrits, ou `-1` (erreur) |

### Parcours de dossiers — `<dirent.h>`

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `opendir` | `DIR *opendir(const char *name)` | chemin du dossier | `DIR *` (flux) ou `NULL` (+ `errno`) |
| `readdir` | `struct dirent *readdir(DIR *dirp)` | flux ouvert | `struct dirent *` (1 entrée) ou `NULL` (fin/erreur) |
| `closedir` | `int closedir(DIR *dirp)` | flux ouvert | `0` si ok, `-1` sinon |

→ rend un `struct dirent *` (voir section 0). Champ clé : `d_name`.

### Métadonnées des fichiers — `<sys/stat.h>`

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `stat` | `int stat(const char *path, struct stat *buf)` | chemin, pointeur de sortie | `0` si ok, `-1` (+ `errno`) — **suit** les symlinks |
| `lstat` | `int lstat(const char *path, struct stat *buf)` | chemin, pointeur de sortie | `0` si ok, `-1` (+ `errno`) — **ne suit pas** les symlinks (à utiliser) |

→ remplit un `struct stat` (voir section 0 pour tous les champs et leurs types).

### uid/gid → noms — `<pwd.h>` / `<grp.h>`

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `getpwuid` | `struct passwd *getpwuid(uid_t uid)` | id utilisateur | `struct passwd *` ou `NULL` |
| `getgrgid` | `struct group *getgrgid(gid_t gid)` | id groupe | `struct group *` ou `NULL` |

→ `pw->pw_name` / `gr->gr_name` (voir section 0). Si `NULL` : afficher le **numéro brut**.

### Dates — `<time.h>`

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `time` | `time_t time(time_t *tloc)` | `NULL` (ou pointeur de sortie) | `time_t` : secondes depuis epoch, ou `-1` |
| `ctime` | `char *ctime(const time_t *timep)` | pointeur vers un `time_t` | `char *` statique `"Tue Jun 30 13:46:25 2026\n"` |

> Format `ls` : caractères `4..15` du `ctime` → `"Jun 30 13:46"`. Règle des 6 mois pour heure vs année.

### Liens symboliques — `<unistd.h>`

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `readlink` | `ssize_t readlink(const char *path, char *buf, size_t bufsiz)` | chemin du lien, buffer, taille | `ssize_t` : octets écrits, ou `-1`. **Ne met PAS le `\0`** |

### Attributs étendus — `<sys/xattr.h>` (pour le `+`/`@` après les droits)

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `listxattr` | `ssize_t listxattr(const char *path, char *list, size_t size)` | chemin, buffer, taille | `ssize_t` : taille de la liste, ou `-1` |
| `getxattr` | `ssize_t getxattr(const char *path, const char *name, void *value, size_t size)` | chemin, nom attr, buffer, taille | `ssize_t` : taille valeur, ou `-1` |

### Mémoire — `<stdlib.h>`

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `malloc` | `void *malloc(size_t size)` | nb d'octets | `void *` ou `NULL` (à tester !) |
| `free` | `void free(void *ptr)` | pointeur alloué | `void` |
| `exit` | `void exit(int status)` | code de sortie (`0` ok, `2` erreur) | ne retourne pas |

### Erreurs — `<stdio.h>` / `<string.h>` / `<errno.h>`

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `perror` | `void perror(const char *s)` | préfixe | `void` (écrit `s: <message errno>` sur stderr) |
| `strerror` | `char *strerror(int errnum)` | `errno` | `char *` (message lisible) |

> `errno` (`int` global) est positionné par `lstat`/`opendir`/… Format `ls` : `ls: cannot access 'X': ` + `strerror(errno)` sur **stderr**.

---

## 2) Libft — `libft.h`

### Chaînes / mémoire (les plus utiles ici)

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `ft_strlen` | `size_t ft_strlen(const char *s)` | chaîne | longueur (`size_t`) |
| `ft_strdup` | `char *ft_strdup(const char *s1)` | chaîne | copie allouée (`char *`) ou `NULL` |
| `ft_strjoin` | `char *ft_strjoin(char const *s1, char const *s2)` | 2 chaînes | concat allouée (`char *`) — **utile pour `dir + "/" + nom`** |
| `ft_strcmp`* | `int ft_strncmp(const char *s1, const char *s2, size_t n)` | 2 chaînes, n | `<0`, `0`, `>0` — **comparateur de tri** |
| `ft_strchr` | `char *ft_strchr(const char *s, int c)` | chaîne, char | pointeur sur 1ère occurrence ou `NULL` |
| `ft_strrchr` | `char *ft_strrchr(const char *s, int c)` | chaîne, char | pointeur sur dernière occurrence ou `NULL` |
| `ft_strlcpy` | `size_t ft_strlcpy(char *dst, const char *src, size_t size)` | dst, src, taille | longueur de `src` (`size_t`) |
| `ft_strlcat` | `size_t ft_strlcat(char *dst, const char *src, size_t size)` | dst, src, taille | longueur tentée (`size_t`) |
| `ft_substr` | `char *ft_substr(char const *s, unsigned int start, size_t len)` | chaîne, début, longueur | sous-chaîne allouée (`char *`) ou `NULL` |
| `ft_strtrim` | `char *ft_strtrim(char const *s1, char const *set)` | chaîne, set | chaîne rognée allouée (`char *`) |
| `ft_split` | `char **ft_split(char const *s, char c)` | chaîne, séparateur | tableau `char **` terminé par `NULL` |
| `ft_strnstr` | `char *ft_strnstr(const char *h, const char *n, size_t len)` | botte de foin, aiguille, len | pointeur ou `NULL` |
| `ft_strmapi` | `char *ft_strmapi(char const *s, char (*f)(unsigned int, char))` | chaîne, fonction | nouvelle chaîne (`char *`) |

\* pas de `ft_strcmp` pur dans ta libft → utilise `ft_strncmp`.

### Mémoire brute

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `ft_calloc` | `void *ft_calloc(size_t count, size_t size)` | nb, taille | zone zéro-initialisée (`void *`) ou `NULL` |
| `ft_bzero` | `void ft_bzero(void *s, size_t n)` | zone, n | `void` |
| `ft_memset` | `void *ft_memset(void *b, int c, size_t len)` | zone, valeur, len | `void *` (b) |
| `ft_memcpy` | `void *ft_memcpy(void *dst, const void *src, size_t n)` | dst, src, n | `void *` (dst) |
| `ft_memmove` | `void *ft_memmove(void *dst, const void *src, size_t len)` | dst, src, len | `void *` (dst) |
| `ft_memchr` | `void *ft_memchr(const void *s, int c, size_t n)` | zone, valeur, n | pointeur ou `NULL` |
| `ft_memcmp` | `int ft_memcmp(const void *s1, const void *s2, size_t n)` | 2 zones, n | `<0`, `0`, `>0` |
| `ft_memccpy` | `void *ft_memccpy(void *dst, const void *src, int c, size_t n)` | dst, src, char stop, n | pointeur après `c` ou `NULL` |

### Conversions / nombres

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `ft_atoi` | `int ft_atoi(const char *str)` | chaîne | `int` |
| `ft_atoi_base` | `int ft_atoi_base(char *str, char *base)` | chaîne, base | `int` |
| `ft_itoa` | `char *ft_itoa(int n)` | entier | chaîne allouée (`char *`) ou `NULL` |
| `ft_itoa_u` | `char *ft_itoa_u(unsigned int n)` | entier non signé | chaîne allouée (`char *`) |
| `ft_itoa_base` | `char *ft_itoa_base(unsigned int nb, char *base)` | nb, base | chaîne allouée (`char *`) |
| `ft_power` | `int ft_power(int nb, int power)` | base, exposant | `int` |

### Caractères (`int` → `int` booléen)

| Fonction | Prototype | Retour |
|----------|-----------|--------|
| `ft_isalpha` | `int ft_isalpha(int c)` | `!=0` si lettre |
| `ft_isdigit` | `int ft_isdigit(int c)` | `!=0` si chiffre |
| `ft_isalnum` | `int ft_isalnum(int c)` | `!=0` si alphanum |
| `ft_isascii` | `int ft_isascii(int c)` | `!=0` si 0–127 |
| `ft_isprint` | `int ft_isprint(int c)` | `!=0` si imprimable |
| `ft_toupper` | `int ft_toupper(int c)` | char majuscule (`int`) |
| `ft_tolower` | `int ft_tolower(int c)` | char minuscule (`int`) |

### Affichage sur fd

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `ft_putchar_fd` | `void ft_putchar_fd(char c, int fd)` | char, fd | `void` |
| `ft_putstr_fd` | `void ft_putstr_fd(char *s, int fd)` | chaîne, fd | `void` |
| `ft_putendl_fd` | `void ft_putendl_fd(char *s, int fd)` | chaîne, fd | `void` (ajoute `\n`) |
| `ft_putnbr_fd` | `void ft_putnbr_fd(int n, int fd)` | entier, fd | `void` |
| `ft_putnbr_u_fd` | `void ft_putnbr_u_fd(unsigned int n, int fd)` | entier u, fd | `void` |
| `ft_putchar` / `ft_putstr` | `void ft_putchar(char c)` / `void ft_putstr(char *s)` | char / chaîne | `void` |

### Listes chaînées — `t_list { void *content; t_list *next; }`

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `ft_lstnew` | `t_list *ft_lstnew(void *content)` | contenu | nouveau maillon (`t_list *`) ou `NULL` |
| `ft_lstadd_front` | `void ft_lstadd_front(t_list **alst, t_list *new)` | tête, maillon | `void` |
| `ft_lstadd_back` | `void ft_lstadd_back(t_list **alst, t_list *new)` | tête, maillon | `void` |
| `ft_lstsize` | `int ft_lstsize(t_list *lst)` | liste | nb de maillons (`int`) |
| `ft_lstlast` | `t_list *ft_lstlast(t_list *lst)` | liste | dernier maillon (`t_list *`) |
| `ft_lstclear` | `void ft_lstclear(t_list **lst, void (*del)(void *))` | liste, libérateur | `void` |
| `ft_lstdelone` | `void ft_lstdelone(t_list *lst, void (*del)(void *))` | maillon, libérateur | `void` |
| `ft_lstiter` | `void ft_lstiter(t_list *lst, void (*f)(void *))` | liste, fonction | `void` |
| `ft_lstmap` | `t_list *ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))` | liste, transfo, libérateur | nouvelle liste (`t_list *`) ou `NULL` |

> Note : `typedef unsigned int t_size;` est défini dans `libft.h`.

---

## 3) ft_printf — `printf.h`

| Fonction | Prototype | Entrées | Retour |
|----------|-----------|---------|--------|
| `ft_printf` | `int ft_printf(const char *c, ...)` | format + args variables | nb de caractères écrits (`int`) |

Conversions gérées par les handlers internes : `%c %s %d %i %u %x %X %p %%`.
> Pour `ft_ls` tu n'appelleras en pratique que `ft_printf(...)` (le reste sont des helpers internes).

---

## Mémo express pour `-l`

```c
struct stat st;
lstat(path, &st);                       // -1 + errno si echec
// type + droits :   st.st_mode   (masques S_ISDIR / S_IRUSR ...)
// liens :           st.st_nlink
// owner :           getpwuid(st.st_uid)->pw_name   (NULL -> numero)
// group :           getgrgid(st.st_gid)->gr_name   (NULL -> numero)
// taille :          st.st_size
// date :            ctime(&st.st_mtime) + 4, 12 chars
// total :           somme(st_blocks) / 2
// symlink cible :   readlink(path, buf, size)       // pas de \0 !
```
