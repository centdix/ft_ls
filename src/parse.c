/* parse.c — lecture de la ligne de commande : options (-lRart) et operandes
   (les chemins), construits en liste de t_path. */

#include "ft_ls.h"

static void	parse_flags(char *arg, t_opts *opts)
{
	int	i;

	i = 1;
	while (arg[i])
	{
		if (arg[i] == 'l')
			opts->list = 1;
		else if (arg[i] == 'R')
			opts->rec = 1;
		else if (arg[i] == 'a')
			opts->all = 1;
		else if (arg[i] == 'r')
			opts->rev = 1;
		else if (arg[i] == 't')
			opts->time = 1;
		i++;
	}
}

/* Cree un t_path pour un operande. stat() suit les symlinks (comme ls le fait
   pour ses arguments) ; op->path pointe dans argv, donc rien a liberer. */
static t_path	*new_path(char *str)
{
	t_path	*op;

	op = malloc(sizeof(t_path));
	if (!op)
		return (NULL);
	op->path = str;
	op->type = PATH_FILE;
	op->staterr = 0;
	ft_bzero(&op->st, sizeof(op->st));
	if (stat(str, &op->st) != 0)
		op->staterr = errno;
	else if (S_ISDIR(op->st.st_mode))
		op->type = PATH_DIR;
	return (op);
}

t_opts	ft_parse_opts(int argc, char **argv)
{
	t_opts	opts;
	int		i;
	int		nb_operands;

	ft_bzero(&opts, sizeof(opts));
	nb_operands = 0;
	i = 1;
	while (i < argc)
	{
		if (argv[i][0] == '-' && argv[i][1])
			parse_flags(argv[i], &opts);
		else
		{
			ft_lstadd_back(&opts.paths, ft_lstnew(new_path(argv[i])));
			nb_operands++;
		}
		i++;
	}
	if (nb_operands == 0)
	{
		ft_lstadd_back(&opts.paths, ft_lstnew(new_path(".")));
		nb_operands = 1;
	}
	opts.paths_count = nb_operands;
	return (opts);
}

/* Libere le t_path mais pas op->path (qui pointe dans argv). */
void	ft_free_path(void *content)
{
	free(content);
}
