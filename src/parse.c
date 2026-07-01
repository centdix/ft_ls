/* parse.c — lecture de la ligne de commande : options (-lRart) et operandes
   (les chemins), construits en liste de t_path. */

#include "ft_ls.h"

/* Erreur d'option facon GNU ls sur stderr, puis sortie en 2 (aucun listing). */
static void	option_error_exit(void)
{
	ft_putstr_fd("Try 'ls --help' for more information.\n", 2);
	exit(2);
}

static void	invalid_short_option(char c)
{
	ft_putstr_fd("ls: invalid option -- '", 2);
	ft_putchar_fd(c, 2);
	ft_putstr_fd("'\n", 2);
	option_error_exit();
}

static void	unrecognized_long_option(char *arg)
{
	ft_putstr_fd("ls: unrecognized option '", 2);
	ft_putstr_fd(arg, 2);
	ft_putstr_fd("'\n", 2);
	option_error_exit();
}

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
		else
			invalid_short_option(arg[i]);
		i++;
	}
}

/* Cree un t_path pour un operande (sans stat : les options peuvent suivre
   l'operande dans argv, ex "ls /bin -l" -> on stat apres le parse complet).
   op->path pointe dans argv, donc rien a liberer. */
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
	return (op);
}

/* Renseigne st/type/staterr d'un operande facon ls. En format long (-l), un
   symlink-argument N'EST PAS suivi (on affiche le lien) sauf slash final ;
   sinon on suit le lien et, s'il est casse (stat KO), on retombe sur lstat
   pour l'afficher quand meme (code retour 0, comme ls). */
static void	stat_operand(t_path *op, int longfmt)
{
	size_t	len;

	len = ft_strlen(op->path);
	if (longfmt && len > 0 && op->path[len - 1] != '/')
	{
		if (lstat(op->path, &op->st) != 0)
			op->staterr = errno;
		else if (S_ISDIR(op->st.st_mode))
			op->type = PATH_DIR;
		return ;
	}
	if (stat(op->path, &op->st) == 0)
	{
		if (S_ISDIR(op->st.st_mode))
			op->type = PATH_DIR;
	}
	else if (lstat(op->path, &op->st) != 0)
		op->staterr = errno;
}

/* Passe de stat sur tous les operandes, une fois toutes les options connues. */
static void	stat_operands(t_list *ops, int longfmt)
{
	t_path	*op;

	while (ops)
	{
		op = ops->content;
		if (op)
			stat_operand(op, longfmt);
		ops = ops->next;
	}
}

/* Ajoute un operande a la liste, en gerant proprement l'echec d'allocation
   (jamais de noeud a contenu NULL -> pas de deref plus tard). */
static void	add_operand(t_list **operands, char *str)
{
	t_path	*op;
	t_list	*node;

	op = new_path(str);
	node = ft_lstnew(op);
	if (op && node)
		ft_lstadd_back(operands, node);
	else
	{
		free(op);
		free(node);
	}
}

t_ls	ft_parse_args(int argc, char **argv)
{
	t_ls	ls;
	int		i;
	int		endopt;

	ft_bzero(&ls.opts, sizeof(ls.opts));
	ls.operands = NULL;
	endopt = 0;
	i = 1;
	while (i < argc)
	{
		/* "--" seul termine les options ; "--xxx" = option longue inconnue ;
		   "-xxx" = options courtes ; "-" seul reste un operande (fichier). */
		if (!endopt && argv[i][0] == '-' && argv[i][1] == '-' && !argv[i][2])
			endopt = 1;
		else if (!endopt && argv[i][0] == '-' && argv[i][1] == '-')
			unrecognized_long_option(argv[i]);
		else if (!endopt && argv[i][0] == '-' && argv[i][1])
			parse_flags(argv[i], &ls.opts);
		else
			add_operand(&ls.operands, argv[i]);
		i++;
	}
	/* aucun operande fourni -> ls liste le repertoire courant. */
	if (!ls.operands)
		add_operand(&ls.operands, ".");
	/* stat differe : toutes les options sont maintenant connues (-l influe
	   sur le (non-)deref des symlinks passes en argument). */
	stat_operands(ls.operands, ls.opts.list);
	return (ls);
}

/* Libere le t_path mais pas op->path (qui pointe dans argv). */
void	ft_free_path(void *content)
{
	free(content);
}
