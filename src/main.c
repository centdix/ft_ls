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
	int		lvl;

	ls = ft_parse_args(argc, argv);
	/* ls emet les erreurs d'acces dans l'ordre d'argv -> avant tout tri.
	   Une erreur sur un argument est "grave" pour ls -> code retour 2. */
	err = 0;
	if (ft_print_access_errors(ls.operands))
		err = 2;
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
		   signales -> ici on ne developpe que les dossiers valides. On garde
		   le niveau d'erreur le plus grave (is_arg=1 -> echec operande = 2). */
		if (operand->staterr == 0 && operand->type == PATH_DIR)
		{
			lvl = ft_list_one_dir(operand->path, &ls.opts, show_header,
					&printed, 1);
			if (lvl > err)
				err = lvl;
		}
		node = node->next;
	}
	ft_lstclear(&ls.operands, ft_free_path);
	/* code retour facon ls : 2 (erreur argument), 1 (erreur sous-dossier), 0. */
	return (err);
}
