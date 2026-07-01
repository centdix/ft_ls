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
# include <sys/types.h>
# include <sys/sysmacros.h> /* major, minor (fichiers de peripherique)*/

/* --- traductions uid/gid, dates, liens --- */
# include <pwd.h>        /* getpwuid                               */
# include <grp.h>        /* getgrgid                               */
# include <time.h>       /* time, ctime                            */
# include <errno.h>      /* errno (messages d'erreur facon ls)     */
# include <string.h>     /* strerror                               */

/* Cle de tri active (le sens -r est gere a part, via opts->rev). */
typedef enum e_sort_by
{
	NAME,
	TIME,
}	t_sort_by;

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
	struct stat	st;
}	t_file;

/* Options de comportement (uniquement les flags de la ligne de commande). */
typedef struct s_opts
{
	int	list;	/* -l : format long     */
	int	rec;	/* -R : recursif        */
	int	all;	/* -a : fichiers caches */
	int	rev;	/* -r : ordre inverse   */
	int	time;	/* -t : tri par date    */
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
}	t_widths;

/* --- parse.c : lecture de la ligne de commande --- */
t_ls	ft_parse_args(int argc, char **argv);
void	ft_free_path(void *content);

/* --- sort.c : tri des entrees et des operandes (by_time = -t, rev = -r) --- */
int		ft_sort_list(t_list *lst, int by_time, int rev);
void	ft_sort_paths(t_list *paths, int by_time, int rev);

/* --- format.c : rendu du format long (-l) --- */
void	ft_print_long_list(t_list *entries, int show_total);
void	ft_calc_widths(t_list *entries, t_widths *w);
void	ft_print_long_line_pub(t_file *f, t_widths *w);

/* --- list.c : listing d'un dossier et erreurs --- */
t_list	*ft_extract_entries(DIR *dir, char *path, int all);
int		ft_print_list(t_list *lst);
void	ft_free_file(void *content);
int		ft_print_access_errors(t_list *paths);
int		ft_list_one_dir(char *path, t_opts *opts, int header, int *printed,
			int is_arg);
void	ft_list_file_operands(t_ls *ls, int *printed);

#endif
