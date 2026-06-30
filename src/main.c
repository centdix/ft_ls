/* ************************************************************************** */
/*                                                                            */
/*   main.c — point d'entree                                                  */
/*                                                                            */
/*   Squelette volontairement minimal : il compile, c'est tout.               */
/*   A toi d'ecrire la logique (parsing des flags, lecture des dossiers,       */
/*   tri, affichage, recursion, gestion d'erreurs).                            */
/*                                                                            */
/*   Pistes (cf. GUIDE_ft_ls.md) :                                            */
/*     1. parser argv -> remplir un t_opts + collecter les chemins            */
/*     2. separer fichiers et dossiers, traiter chaque cible                  */
/*     3. pour un dossier : readdir -> lstat chaque entree -> trier -> afficher*/
/*     4. si -R : redescendre dans chaque sous-dossier                        */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ls.h"

int	main(int argc, char **argv)
{
	t_opts	opts;

	opts = (t_opts){0, 0, 0, 0, 0};
	(void)argc;
	(void)argv;
	(void)opts;

	/* TODO: ton ft_ls commence ici. */

	return (0);
}
