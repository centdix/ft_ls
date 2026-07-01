/* ft_ls.h — includes systeme, libft, types et prototypes de ft_ls. */

#ifndef FT_LS_H
# define FT_LS_H

/* --- libft (+ ft_printf integre dedans) --- */
# include "libft.h"
# include "printf.h"

/* --- E/S, parcours de dossiers --- */
# include <unistd.h>     /* write                                  */
# include <stdio.h>      /* perror                                 */
# include <stdlib.h>     /* malloc, free, exit                     */
# include <dirent.h>     /* opendir, readdir, closedir, DIR        */

/* --- metadonnees des fichiers --- */
# include <sys/stat.h>      /* stat, lstat, struct stat, S_IS*, S_I*  */
# include <sys/types.h>     /* major, minor (mac : ici ; Linux : ci-dessous) */

/* --- traductions uid/gid, dates, liens --- */
# include <pwd.h>        /* getpwuid                               */
# include <grp.h>        /* getgrgid                               */
# include <time.h>       /* time, ctime                            */
# include <errno.h>      /* errno (messages d'erreur facon ls)     */
# include <string.h>     /* strerror                               */
# include <sys/xattr.h>  /* getxattr/lgetxattr (bonus: marqueur ACL) */

/* --- Portabilite Linux / macOS ------------------------------------------
   Les champs "nanoseconde" de struct stat, l'API xattr et l'unite des blocs
   de la ligne "total" different entre glibc/Linux et BSD/macOS. On isole ces
   differences ici pour que le reste du code reste identique sur les 2 OS. */
# ifdef __APPLE__
#  define ST_ATIM_S(st)  ((st).st_atimespec.tv_sec)
#  define ST_ATIM_NS(st) ((st).st_atimespec.tv_nsec)
#  define ST_MTIM_S(st)  ((st).st_mtimespec.tv_sec)
#  define ST_MTIM_NS(st) ((st).st_mtimespec.tv_nsec)
#  define ST_CTIM_S(st)  ((st).st_ctimespec.tv_sec)
#  define ST_CTIM_NS(st) ((st).st_ctimespec.tv_nsec)
/* BSD ls : "total" en blocs de 512 o (pas de division). */
#  define TOTAL_BLOCKS(b) (b)
/* BSD ls : 2 espaces entre les colonnes owner/group/size du -l. */
#  define COL_GAP "  "
/* BSD ls -R : pas d'en-tete "chemin:" pour un dossier-operande unique. */
#  define REC_FORCES_HEADER 0
# else
#  include <sys/sysmacros.h> /* major, minor sous glibc                 */
#  define ST_ATIM_S(st)  ((st).st_atim.tv_sec)
#  define ST_ATIM_NS(st) ((st).st_atim.tv_nsec)
#  define ST_MTIM_S(st)  ((st).st_mtim.tv_sec)
#  define ST_MTIM_NS(st) ((st).st_mtim.tv_nsec)
#  define ST_CTIM_S(st)  ((st).st_ctim.tv_sec)
#  define ST_CTIM_NS(st) ((st).st_ctim.tv_nsec)
/* GNU ls : "total" en blocs de 1 Ko (st_blocks est en unites de 512 o). */
#  define TOTAL_BLOCKS(b) ((b) / 2)
/* GNU ls : 1 espace entre les colonnes owner/group/size du -l. */
#  define COL_GAP " "
/* GNU ls -R : en-tete "chemin:" meme pour un dossier-operande unique. */
#  define REC_FORCES_HEADER 1
# endif

/* Existence d'un attribut etendu SANS suivre les symlinks (>=0 s'il existe).
   lgetxattr(4 args) sous Linux ; getxattr(6 args, XATTR_NOFOLLOW) sous mac. */
# ifdef __APPLE__
#  define X_HAS(path, name) (getxattr((path), (name), NULL, 0, 0, XATTR_NOFOLLOW))
# else
#  define X_HAS(path, name) (lgetxattr((path), (name), NULL, 0))
# endif

/* Cle de tri active (le sens -r est gere a part, via opts->rev).
   SORT_NONE = -f (ordre du readdir, pas de tri). */
typedef enum e_sort_by
{
	SORT_NAME,
	SORT_TIME,
	SORT_SIZE,
	SORT_NONE,
}	t_sort_by;

/* Champ temporel utilise par -l et le tri -t : mtime (defaut), atime (-u)
   ou ctime (-c). */
typedef enum e_timek
{
	TK_MTIME,
	TK_ATIME,
	TK_CTIME,
}	t_timek;

/* PATH_FILE/PATH_DIR (pas FILE/DIR, deja pris par <stdio.h> et <dirent.h>).
   Ordre PATH_FILE < PATH_DIR -> les fichiers passent avant les dossiers. */
typedef enum e_path_type
{
	PATH_FILE,
	PATH_DIR,
}	t_path_type;

/* --- structs --- */
typedef struct s_path
{
	char			*path;	/* chemin tel que donne en argument (pointe argv) */
	t_path_type		type;	/* fichier ou dossier (determine via stat)        */
	int				staterr;/* 0 si stat a reussi, sinon errno (chemin invalide) */
	struct stat		st;		/* metadonnees (pour le tri -t des operandes)     */
}	t_path;

typedef struct s_file
{
	char		*name;
	char		*path;
	char		acl;	/* bonus : '+' (ACL), '.' (contexte), ' ' (aucun) */
	struct stat	st;
}	t_file;

/* Options de comportement (uniquement les flags de la ligne de commande). */
typedef struct s_opts
{
	int			list;		/* -l : format long                        */
	int			rec;		/* -R : recursif                           */
	int			all;		/* -a : fichiers caches (. et .. inclus)   */
	int			almost;		/* -A : caches sauf . et ..                */
	int			rev;		/* -r : ordre inverse                      */
	int			dironly;	/* -d : les dossiers eux-memes, pas leur contenu */
	int			one;		/* -1 : une entree par ligne (deja le cas) */
	int			inode;		/* -i : numero d'inode                     */
	int			slash;		/* -p : '/' apres les dossiers             */
	int			noowner;	/* -g : -l sans la colonne proprietaire    */
	int			nogroup;	/* -o : -l sans la colonne groupe          */
	t_sort_by	sort;		/* cle de tri (-t, -S, -f)                 */
	t_timek		timek;		/* champ temporel (-u, -c)                 */
}	t_opts;

/* Contexte complet : les flags + la liste des operandes (cibles) a lister. */
typedef struct s_ls
{
	t_opts	opts;
	t_list	*operands;	/* liste chainee de (t_path *) */
}	t_ls;

/* Largeurs de colonnes du format -l (calculees sur l'ensemble des entrees
   pour aligner les colonnes). Pour les peripheriques, la colonne "size"
   accueille "major, minor" -> on suit aussi major/minor separement. */
typedef struct s_widths
{
	int	links;
	int	owner;
	int	group;
	int	size;
	int	major;
	int	minor;
	int	inode;	/* bonus -i : largeur de la colonne inode            */
	int	aclcol;	/* bonus : 1 si une entree du bloc a un ACL/contexte */
}	t_widths;

/* --- parse.c : lecture de la ligne de commande --- */
t_ls	ft_parse_args(int argc, char **argv);
void	ft_free_path(void *content);

/* --- sort.c : tri des entrees et des operandes (cle + sens via opts) --- */
int		ft_sort_list(t_list *lst, t_opts *opts);
void	ft_sort_paths(t_list *paths, t_opts *opts);
time_t	ft_pick_time(struct stat *st, t_timek tk);

/* --- format.c : rendu du format long (-l) --- */
void	ft_print_long_list(t_list *entries, int show_total, t_opts *opts);
void	ft_calc_widths(t_list *entries, t_widths *w);
void	ft_print_long_line_pub(t_file *f, t_widths *w, t_opts *opts);
void	ft_print_short_line(t_file *f, t_widths *w, t_opts *opts);

/* --- list.c : listing d'un dossier et erreurs --- */
t_list	*ft_extract_entries(DIR *dir, char *path, t_opts *opts);
int		ft_print_list(t_list *lst, t_opts *opts);
void	ft_free_file(void *content);
int		ft_print_access_errors(t_list *paths);
int		ft_list_one_dir(char *path, t_opts *opts, int header, int *printed,
			int is_arg);
void	ft_list_file_operands(t_ls *ls, int *printed);

#endif
