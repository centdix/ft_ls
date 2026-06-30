/* ************************************************************************** */
/*                                                                            */
/*   ft_ls.h — header de base                                                 */
/*                                                                            */
/*   Ce fichier ne contient QUE :                                             */
/*     - les includes systeme dont tu auras besoin                            */
/*     - l'include de ta libft                                                */
/*     - une petite struct d'options pour demarrer ta conception              */
/*                                                                            */
/*   A toi d'ajouter ici les prototypes de TES fonctions au fur et a mesure.  */
/*                                                                            */
/* ************************************************************************** */

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
# include <sys/stat.h>   /* stat, lstat, struct stat, S_IS*, S_I*  */
# include <sys/types.h>

/* --- traductions uid/gid, dates, liens --- */
# include <pwd.h>        /* getpwuid                               */
# include <grp.h>        /* getgrgid                               */
# include <time.h>       /* time, ctime                            */
# include <errno.h>      /* errno (messages d'erreur facon ls)     */
# include <string.h>     /* strerror                               */

/*
** Struct d'options : un seul endroit pour transporter les flags actifs.
** (-l -R -a -r -t). Libre a toi de la modifier / l'etendre.
*/
typedef struct s_opts
{
	int	list;	/* -l : format long          */
	int	rec;	/* -R : recursif             */
	int	all;	/* -a : fichiers caches      */
	int	rev;	/* -r : ordre inverse        */
	int	time;	/* -t : tri par date         */
}	t_opts;

#endif
