# ft_ls — known issues / à investiguer plus tard

Notes de bugs connus et points en suspens, à reprendre pendant la passe
"pixel-perfect" finale. Aucun n'est bloquant pour l'avancement actuel.

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
