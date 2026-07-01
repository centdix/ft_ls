/* main.c — point d'entree : parse argv, signale les erreurs d'acces, trie les
   operandes, puis affiche les fichiers avant les dossiers (cf. list.c). */

#include "ft_ls.h"

int	main(int argc, char **argv)
{
	t_ls	ls;
	t_list	*node;
	int		printed;
	int		show_header;
	int		err;

	ls = ft_parse_args(argc, argv);
	/* ls emet les erreurs d'acces dans l'ordre d'argv -> avant tout tri. */
	err = ft_print_access_errors(ls.operands);
	ft_sort_paths(ls.operands, ls.opts.time, ls.opts.rev);
	/* printed : a-t-on deja ecrit qqch ? sert a inserer la ligne vide
	   separatrice entre deux blocs (fichiers, puis chaque dossier). */
	printed = 0;
	/* ls ne prefixe "chemin:" que s'il y a plusieurs cibles, ou en -R. */
	show_header = (ft_lstsize(ls.operands) > 1 || ls.opts.rec);
	/* ls affiche d'abord TOUS les operandes-fichiers (en bloc), puis les
	   dossiers : on traite donc les fichiers avant la boucle. */
	ft_list_file_operands(&ls, &printed);
	node = ls.operands;
	while (node)
	{
		t_path	*operand = node->content;

		/* fichiers deja traites ci-dessus ; operandes en erreur deja
		   signales -> ici on ne developpe que les dossiers valides. */
		if (operand->staterr == 0 && operand->type == PATH_DIR)
		{
			if (ft_list_one_dir(operand->path, &ls.opts, show_header, &printed))
				err = 1;
		}
		node = node->next;
	}
	ft_lstclear(&ls.operands, ft_free_path);
	/* code retour facon ls : 2 si une erreur a eu lieu, 0 sinon. */
	return (err ? 2 : 0);
}
