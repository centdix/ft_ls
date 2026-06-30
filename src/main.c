/* main.c — point d'entree : parse argv, signale les erreurs d'acces, trie les
   operandes, puis affiche les fichiers avant les dossiers (cf. utils.c). */

#include "ft_ls.h"

int	main(int argc, char **argv)
{
	t_opts	opts;
	t_list	*node;
	int		printed;
	int		show_header;
	int		err;

	opts = ft_parse_opts(argc, argv);
	err = ft_print_access_errors(opts.paths);
	ft_sort_paths(opts.paths, &opts);
	printed = 0;
	show_header = (opts.paths_count > 1 || opts.rec);
	node = opts.paths;
	while (node)
	{
		t_path	*operand = node->content;

		if (operand->staterr == 0 && operand->type == PATH_FILE)
		{
			ft_printf("%s\n", operand->path);
			printed = 1;
		}
		else if (operand->staterr == 0 && operand->type == PATH_DIR)
		{
			if (ft_list_one_dir(operand->path, &opts, show_header, &printed))
				err = 1;
		}
		node = node->next;
	}
	ft_lstclear(&opts.paths, ft_free_path);
	return (err ? 2 : 0);
}
