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

t_ls	ft_parse_args(int argc, char **argv)
{
	t_ls	ls;
	int		i;

	ft_bzero(&ls.opts, sizeof(ls.opts));
	ls.operands = NULL;
	i = 1;
	while (i < argc)
	{
		/* "-xxx" = options ; "-" seul reste un operande (nom de fichier). */
		if (argv[i][0] == '-' && argv[i][1])
			parse_flags(argv[i], &ls.opts);
		else
			ft_lstadd_back(&ls.operands, ft_lstnew(new_path(argv[i])));
		i++;
	}
	/* aucun operande fourni -> ls liste le repertoire courant. */
	if (!ls.operands)
		ft_lstadd_back(&ls.operands, ft_lstnew(new_path(".")));
	return (ls);
}

/* Libere le t_path mais pas op->path (qui pointe dans argv). */
void	ft_free_path(void *content)
{
	free(content);
}
