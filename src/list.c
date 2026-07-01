/* list.c — listing d'un dossier (lecture, affichage, recursion -R) et messages
   d'erreur facon ls. */

#include "ft_ls.h"

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

t_list	*ft_extract_entries(DIR *dir, char *path, int all)
{
	t_list			*entries;
	struct dirent	*entry;
	t_file			*file;

	entries = NULL;
	while ((entry = readdir(dir)) != NULL)
	{
		if (!all && entry->d_name[0] == '.')
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

/* Operandes "fichiers" (non-dossiers) : ls les affiche groupes, avant les
   dossiers, sans en-tete ni ligne "total". Alignement -l partage entre eux. */
void	ft_list_file_operands(t_ls *ls, int *printed)
{
	t_list	*files;
	t_list	*node;
	t_path	*op;
	t_file	*f;

	files = NULL;
	node = ls->operands;
	while (node)
	{
		op = node->content;
		if (op->staterr == 0 && op->type == PATH_FILE)
		{
			f = malloc(sizeof(t_file));
			f->name = ft_strdup(op->path);
			f->path = ft_strdup(op->path);
			f->st = op->st;
			ft_lstadd_back(&files, ft_lstnew(f));
		}
		node = node->next;
	}
	if (!files)
		return ;
	if (ls->opts.list)
		ft_print_long_list(files, 0);
	else
		ft_print_list(files);
	*printed = 1;
	ft_lstclear(&files, ft_free_file);
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
	entries = ft_extract_entries(dir, path, opts->all);
	ft_sort_list(entries, opts->time, opts->rev);
	if (opts->list)
		ft_print_long_list(entries, 1);
	else
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
