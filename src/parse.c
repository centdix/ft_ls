/* parse.c — lecture de la ligne de commande : options (-lRart) et operandes
   (les chemins), construits en liste de t_path. */

#include "ft_ls.h"

/* Erreur d'option sur stderr, puis sortie (aucun listing). Le texte de la 2e
   ligne et le code retour different GNU/BSD (cf. OPT_HELP / RC_ERR). */
static void	option_error_exit(void)
{
	ft_putstr_fd(OPT_HELP, 2);
	exit(RC_ERR);
}

static void	invalid_short_option(char c)
{
	ft_putstr_fd("ls: invalid option -- ", 2);
	ft_putstr_fd(OPT_SQ1, 2);
	ft_putchar_fd(c, 2);
	ft_putstr_fd(OPT_SQ2, 2);
	ft_putstr_fd("\n", 2);
	option_error_exit();
}

static void	unrecognized_long_option(char *arg)
{
	ft_putstr_fd("ls: unrecognized option ", 2);
	ft_putstr_fd(OPT_LQ1, 2);
	ft_putstr_fd(arg, 2);
	ft_putstr_fd(OPT_LQ2, 2);
	ft_putstr_fd("\n", 2);
	option_error_exit();
}

/* Un flag = un effet. Les cles de tri (-t/-S/-f) s'ecrasent dans l'ordre de
   lecture, comme ls. -g/-o impliquent le format long (-l). -f force -a et
   desactive le tri (sans desactiver -l, conforme au ls actuel). */
static int	set_flag(char c, t_opts *opts)
{
	if (c == 'l')
		opts->list = 1;
	else if (c == 'R')
		opts->rec = 1;
	else if (c == 'a')
		(opts->all = 1, opts->almost = 0);
	else if (c == 'A')
		(opts->almost = 1, opts->all = 0);
	else if (c == 'r')
		opts->rev = 1;
	else if (c == 't')
		opts->sort = SORT_TIME;
	else if (c == 'S')
		opts->sort = SORT_SIZE;
	else if (c == 'f')
		(opts->all = 1, opts->almost = 0, opts->sort = SORT_NONE);
	else if (c == 'd')
		opts->dironly = 1;
	else if (c == '1')
		opts->one = 1;
	else if (c == 'i')
		opts->inode = 1;
	else if (c == 'p')
		opts->slash = 1;
	else if (c == 'g')
		(opts->list = 1, opts->noowner = 1);
	else if (c == 'o')
		(opts->list = 1, opts->nogroup = 1);
	else if (c == 'u')
		opts->timek = TK_ATIME;
	else if (c == 'c')
		opts->timek = TK_CTIME;
	else
		return (0);
	return (1);
}

static void	parse_flags(char *arg, t_opts *opts)
{
	int	i;

	i = 1;
	while (arg[i])
	{
		if (!set_flag(arg[i], opts))
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

/* Renseigne st/type/staterr d'un operande facon ls. Quand ls ne developpe pas
   les symlinks-arguments (-l, -d ...), on N'EST PAS suivi (on affiche le lien)
   sauf slash final ; sinon on suit le lien et, s'il est casse (stat KO), on
   retombe sur lstat pour l'afficher quand meme (code retour 0, comme ls). */
static void	stat_operand(t_path *op, int nofollow)
{
	size_t	len;

	len = ft_strlen(op->path);
	if (nofollow && len > 0 && op->path[len - 1] != '/')
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
static void	stat_operands(t_list *ops, int nofollow)
{
	t_path	*op;

	while (ops)
	{
		op = ops->content;
		if (op)
			stat_operand(op, nofollow);
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
	/* -u/-c sans -t trient par le temps choisi (atime/ctime), sauf en -l ou
	   le tri reste par nom (facon ls). -S/-f gardent leur cle. */
	if (ls.opts.timek != TK_MTIME && ls.opts.sort == SORT_NAME && !ls.opts.list)
		ls.opts.sort = SORT_TIME;
	/* aucun operande fourni -> ls liste le repertoire courant. */
	if (!ls.operands)
		add_operand(&ls.operands, ".");
	/* stat differe : toutes les options sont maintenant connues (-l et -d
	   influent sur le (non-)deref des symlinks passes en argument). */
	stat_operands(ls.operands, ls.opts.list || ls.opts.dironly);
	return (ls);
}

/* Libere le t_path mais pas op->path (qui pointe dans argv). */
void	ft_free_path(void *content)
{
	free(content);
}
