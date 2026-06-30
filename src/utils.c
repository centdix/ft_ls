/* utils.c — helpers de ft_ls : tri, parsing des options, listing d'un dossier. */

#include "ft_ls.h"

/* Tri -t facon ls : plus recent d'abord, depart a la nanoseconde. */
static int	cmp_mtime(struct stat *a, struct stat *b)
{
	if (a->st_mtim.tv_sec != b->st_mtim.tv_sec)
		return (a->st_mtim.tv_sec > b->st_mtim.tv_sec ? -1 : 1);
	if (a->st_mtim.tv_nsec != b->st_mtim.tv_nsec)
		return (a->st_mtim.tv_nsec > b->st_mtim.tv_nsec ? -1 : 1);
	return (0);
}

/* Compare deux entrees selon la cle active ; -t departage par date puis,
   a egalite exacte, retombe sur le nom. Le -r est applique par l'appelant. */
static int	cmp_files(t_file *a, t_file *b, t_sort_by by)
{
	int	time_cmp;

	if (by == TIME)
	{
		time_cmp = cmp_mtime(&a->st, &b->st);
		if (time_cmp != 0)
			return (time_cmp);
	}
	return (ft_strncmp(a->name, b->name, ft_strlen(a->name) + 1));
}

/* Tri a bulles : on echange le contenu des noeuds, pas les noeuds eux-memes. */
int	ft_sort_list(t_list *lst, t_opts *opts)
{
	t_sort_by	by;
	t_list		*cur;
	void		*swap;
	int			cmp;
	int			swapped;

	if (!lst)
		return (0);
	by = opts->time ? TIME : NAME;
	swapped = 1;
	while (swapped)
	{
		swapped = 0;
		cur = lst;
		while (cur->next)
		{
			cmp = cmp_files(cur->content, cur->next->content, by);
			if (opts->rev)
				cmp = -cmp;
			if (cmp > 0)
			{
				swap = cur->content;
				cur->content = cur->next->content;
				cur->next->content = swap;
				swapped = 1;
			}
			cur = cur->next;
		}
	}
	return (0);
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

/* Ordre des operandes facon ls : fichiers avant dossiers (groupage jamais
   inverse par -r), puis cle active (nom, ou date si -t) dans chaque groupe. */
static int	cmp_paths(t_path *a, t_path *b, t_opts *opts)
{
	int	cmp;

	if (a->type != b->type)
		return ((int)a->type - (int)b->type);
	cmp = 0;
	if (opts->time)
		cmp = cmp_mtime(&a->st, &b->st);
	if (cmp == 0)
		cmp = ft_strncmp(a->path, b->path, ft_strlen(a->path) + 1);
	if (opts->rev)
		cmp = -cmp;
	return (cmp);
}

void	ft_sort_paths(t_list *paths, t_opts *opts)
{
	t_list	*cur;
	void	*swap;
	int		swapped;

	if (!paths)
		return ;
	swapped = 1;
	while (swapped)
	{
		swapped = 0;
		cur = paths;
		while (cur->next)
		{
			if (cmp_paths(cur->content, cur->next->content, opts) > 0)
			{
				swap = cur->content;
				cur->content = cur->next->content;
				cur->next->content = swap;
				swapped = 1;
			}
			cur = cur->next;
		}
	}
}

/* Libere le t_path mais pas op->path (qui pointe dans argv). */
void	ft_free_path(void *content)
{
	free(content);
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

/* Construit "dir/name" (la libft n'a qu'un ft_strjoin a 2 arguments). */
static char	*path_join(char *dir, char *name)
{
	char	*tmp;
	char	*res;

	tmp = ft_strjoin(dir, "/");
	if (!tmp)
		return (NULL);
	res = ft_strjoin(tmp, name);
	free(tmp);
	return (res);
}

t_list	*ft_extract_entries(DIR *dir, char *path, t_opts *opts)
{
	t_list			*entries;
	struct dirent	*entry;
	t_file			*file;

	entries = NULL;
	while ((entry = readdir(dir)) != NULL)
	{
		if (!opts->all && entry->d_name[0] == '.')
			continue ;
		file = malloc(sizeof(t_file));
		if (!file)
			return (NULL);
		file->name = ft_strdup(entry->d_name);
		file->path = path_join(path, entry->d_name);
		lstat(file->path, &file->st);
		ft_lstadd_back(&entries, ft_lstnew(file));
	}
	return (entries);
}

int	ft_print_list(t_list *lst)
{
	t_file	*file;

	while (lst)
	{
		file = lst->content;
		ft_printf("%s\n", file->name);
		lst = lst->next;
	}
	return (0);
}

void	ft_free_file(void *content)
{
	t_file	*file;

	file = content;
	free(file->name);
	free(file->path);
	free(file);
}

/* Erreur facon ls sur stderr : "ls: <reason> '<path>': <strerror>". Prefixe
   "ls" (et non "ft_ls") pour matcher la sortie comparee dans tests/. */
static void	ls_error(char *path, char *reason, int err)
{
	ft_putstr_fd("ls: ", 2);
	ft_putstr_fd(reason, 2);
	ft_putstr_fd(" '", 2);
	ft_putstr_fd(path, 2);
	ft_putstr_fd("': ", 2);
	ft_putstr_fd(strerror(err), 2);
	ft_putstr_fd("\n", 2);
}

/* Passe 1 : erreurs d'acces aux operandes, en ordre argv, avant le tri. */
int	ft_print_access_errors(t_list *paths)
{
	t_path	*op;
	int		err;

	err = 0;
	while (paths)
	{
		op = paths->content;
		if (op->staterr != 0)
		{
			ls_error(op->path, "cannot access", op->staterr);
			err = 1;
		}
		paths = paths->next;
	}
	return (err);
}

/* Liste un dossier : separateur + en-tete eventuel + contenu trie, puis
   redescend dans chaque sous-dossier si -R. */
int	ft_list_one_dir(char *path, t_opts *opts, int header, int *printed)
{
	DIR		*dir;
	t_list	*entries;
	t_list	*cur;
	t_file	*file;

	dir = opendir(path);
	if (!dir)
	{
		ls_error(path, "cannot open directory", errno);
		return (1);
	}
	if (*printed)
		ft_printf("\n");
	if (header)
		ft_printf("%s:\n", path);
	entries = ft_extract_entries(dir, path, opts);
	ft_sort_list(entries, opts);
	ft_print_list(entries);
	*printed = 1;
	cur = entries;
	while (opts->rec && cur)
	{
		file = cur->content;
		if (S_ISDIR(file->st.st_mode) && ft_strncmp(file->name, ".", 2) != 0
			&& ft_strncmp(file->name, "..", 3) != 0)
			ft_list_one_dir(file->path, opts, 1, printed);
		cur = cur->next;
	}
	ft_lstclear(&entries, ft_free_file);
	closedir(dir);
	return (0);
}
