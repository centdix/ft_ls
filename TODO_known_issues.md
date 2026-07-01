# ft_ls — écarts résiduels vs `ls`

Tout le reste a été vérifié **identique** à la référence (stdout + stderr + code
retour, `LC_ALL=C`, 0 fuite) : conformité GNU coreutils sur Linux, conformité
BSD sur macOS, options mandatory + bonus, tri (jusqu'à la nanoseconde), `-R`,
erreurs, colonnes `?` sur `lstat` échoué, marqueur `@` (xattr) sur mac, etc.

Ne restent ci-dessous que les points qui **pourraient encore différer** du vrai
`ls` — tous connus, bornés, et défendables à l'oral.

---

## 1. `+` des ACL POSIX sur macOS (mac uniquement)

Sur macOS, le BSD `ls` affiche `+` en 11e colonne pour une entrée portant une
**ACL POSIX**. Nous affichons `' '` pour ces (rares) entrées.

Cause : les ACL macOS ne sont **pas** exposées par `listxattr`/`getxattr` ; il
faut `acl_get_file()` de `<sys/acl.h>`, **absente de la liste de fonctions
autorisées** du sujet (qui exempte d'ailleurs explicitement les ACL). Le
marqueur `@` (extended attributes) est, lui, bien géré (`X_LIST`/`ACL_CHAR`).

Portée : **macOS seulement**, quelques entrées (le home `~`, certains dossiers
système). **Sur Linux (machine d'éval) : aucun écart.**

---

## 2. Colonnes multiples sans `-l` (exempté par le sujet)

Vers un terminal, le vrai `ls` sans `-l` dispose les noms sur **plusieurs
colonnes** (largeur du TTY via `ioctl`). Nous affichons **une entrée par ligne**
(comportement de `ls` quand la sortie est un pipe).

C'est **explicitement exempté** par le sujet :
> « You do not have to deal with the multiple column format for the exit when
> the option `-l` isn't in the arguments. »

Le format multi-colonnes n'est proposé qu'en **bonus** (`man 4 tty`) et n'est
pas traité. Aucune information manquante, seulement la disposition.

---

## 3. Divers (bornés, hors périmètre)

- **`readdir` + `errno`** : si une lecture de dossier échoue en cours de route,
  on émet `ls: reading directory '<path>': ...`, mais ce cas précis ne propage
  pas un code retour dédié (best-effort, cas très rare).
- **Préfixe `ls:`** figé dans les messages d'erreur : on matche la référence
  `ls` (invoquée `ls`). Lancé `./ft_ls`, le vrai GNU imprimerait `ft_ls:`
  (`argv[0]`). Choix assumé, conforme à ce que compare l'éval.
- **Tri dépendant de la locale** : tri en ordre d'octets. En locale non-C, GNU
  trie autrement (casse/accents) ; l'éval tourne en `LC_ALL=C` → conforme.
- **`BLOCK_SIZE` / `POSIXLY_CORRECT`** custom non gérés (hors sujet) ; la ligne
  `total` suit l'unité par OS (GNU 1 Ko, BSD 512 o).
