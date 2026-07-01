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

/* Faut-il afficher cette entree ? -a : tout ; -A : tout sauf "." et ".." ;
   defaut : rien de ce qui commence par '.'. */
static int	show_entry(char *name, t_opts *opts)
{
	if (name[0] != '.')
		return (1);
	if (opts->all)
		return (1);
	if (opts->almost && ft_strncmp(name, ".", 2) != 0
		&& ft_strncmp(name, "..", 3) != 0)
		return (1);
	return (0);
}

/* Cree un t_file pour une entree de dossier ; NULL si une allocation echoue
   (name/path/malloc). #3 : si lstat echoue (fichier disparu, droits perdus),
   on met st a zero -> pas de lecture de champs non initialises. NB : GNU
   afficherait des '?' dans les colonnes -l ; ce rendu exact est postpose. */
static t_file	*make_entry(char *path, char *name)
{
	t_file	*file;

	file = malloc(sizeof(t_file));
	if (!file)
		return (NULL);
	file->name = ft_strdup(name);
	file->path = path_join(path, name);
	file->acl = ' ';
	if (!file->name || !file->path)
	{
		ft_free_file(file);
		return (NULL);
	}
	/* lstat (pas stat) : on decrit le lien lui-meme, pas sa cible. */
	if (lstat(file->path, &file->st) != 0)
		ft_bzero(&file->st, sizeof(file->st));
	return (file);
}

t_list	*ft_extract_entries(DIR *dir, char *path, t_opts *opts)
{
	t_list			*entries;
	struct dirent	*entry;
	t_file			*file;
	t_list			*node;

	entries = NULL;
	errno = 0;
	while ((entry = readdir(dir)) != NULL)
	{
		if (show_entry(entry->d_name, opts))
		{
			file = make_entry(path, entry->d_name);
			node = NULL;
			if (file)
				node = ft_lstnew(file);
			if (!node)
			{
				if (file)
					ft_free_file(file);
				ft_lstclear(&entries, ft_free_file);
				return (NULL);
			}
			ft_lstadd_back(&entries, node);
		}
		errno = 0;
	}
	/* #6 : readdir renvoie NULL en fin de dossier ET sur erreur -> errno
	   distingue les deux (remis a 0 avant chaque lecture). */
	if (errno != 0)
		ls_error(path, "reading directory", errno);
	return (entries);
}

void	ft_free_file(void *content)
{
	t_file	*file;

	file = content;
	free(file->name);
	free(file->path);
	free(file);
}

/* t_file a partir d'un operande deja stat (durci : NULL si strdup echoue). */
static t_file	*make_operand(t_path *op)
{
	t_file	*f;

	f = malloc(sizeof(t_file));
	if (!f)
		return (NULL);
	f->name = ft_strdup(op->path);
	f->path = ft_strdup(op->path);
	f->acl = ' ';
	f->st = op->st;
	if (!f->name || !f->path)
	{
		ft_free_file(f);
		return (NULL);
	}
	return (f);
}

/* Convertit chaque operande valide (fichier ET dossier) en t_file. Les
   dossiers sont inclus car GNU calcule les largeurs de colonnes du bloc
   -l sur TOUS les operandes (ex "ls -l *"), meme s'ils ne sont pas imprimes
   ici (les dossiers sont listes ensuite, a part). */
static t_list	*collect_operand_files(t_list *operands)
{
	t_list	*all;
	t_file	*f;
	t_list	*node;

	all = NULL;
	while (operands)
	{
		if (((t_path *)operands->content)->staterr == 0)
		{
			f = make_operand(operands->content);
			node = NULL;
			if (f)
				node = ft_lstnew(f);
			if (node)
				ft_lstadd_back(&all, node);
			else if (f)
				ft_free_file(f);
		}
		operands = operands->next;
	}
	return (all);
}

/* En -d chaque operande (dossier compris) est une entree a imprimer ; sinon
   seuls les non-dossiers le sont ici (les dossiers sont listes a part). */
static int	printable(t_file *f, t_opts *opts)
{
	return (opts->dironly || !S_ISDIR(f->st.st_mode));
}

/* Y a-t-il au moins une entree a afficher dans le bloc operandes ? */
static int	has_printable(t_list *all, t_opts *opts)
{
	while (all)
	{
		if (printable(all->content, opts))
			return (1);
		all = all->next;
	}
	return (0);
}

/* Imprime le bloc des operandes, largeurs deja calculees sur l'ensemble. */
static void	print_operand_files(t_list *all, t_opts *opts, t_widths *w)
{
	t_file	*f;

	while (all)
	{
		f = all->content;
		if (printable(f, opts))
		{
			if (opts->list)
				ft_print_long_line_pub(f, w, opts);
			else
				ft_print_short_line(f, w, opts);
		}
		all = all->next;
	}
}

/* Operandes affiches en bloc (avant les dossiers a developper), sans en-tete
   ni ligne "total". Alignement calcule sur l'ensemble des operandes (dossiers
   inclus) comme GNU. En -d, les dossiers-operandes sont eux aussi imprimes. */
void	ft_list_file_operands(t_ls *ls, int *printed)
{
	t_list		*all;
	t_widths	w;

	all = collect_operand_files(ls->operands);
	if (!has_printable(all, &ls->opts))
	{
		ft_lstclear(&all, ft_free_file);
		return ;
	}
	ft_calc_widths(all, &w);
	print_operand_files(all, &ls->opts, &w);
	*printed = 1;
	ft_lstclear(&all, ft_free_file);
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
   redescend dans chaque sous-dossier si -R. Renvoie le niveau d'erreur facon
   ls (0 = ok) ; is_arg distingue un operande (echec -> 2) d'un sous-dossier
   de recursion (echec -> 1). Le max des erreurs remonte a l'appelant. */
int	ft_list_one_dir(char *path, t_opts *opts, int header, int *printed,
	int is_arg)
{
	DIR		*dir;
	t_list	*entries;
	t_list	*cur;
	t_file	*file;
	int		err;

	dir = opendir(path);
	if (!dir)
	{
		ls_error(path, "cannot open directory", errno);
		return (is_arg ? 2 : 1);
	}
	/* ligne vide separatrice si un bloc a deja ete imprime avant celui-ci. */
	if (*printed)
		ft_printf("\n");
	if (header)
		ft_printf("%s:\n", path);
	entries = ft_extract_entries(dir, path, opts);
	ft_sort_list(entries, opts);
	/* 1 = imprimer "total" : c'est le contenu d'un dossier. */
	if (opts->list)
		ft_print_long_list(entries, 1, opts);
	else
		ft_print_list(entries, opts);
	*printed = 1;
	err = 0;
	cur = entries;
	while (opts->rec && cur)
	{
		file = cur->content;
		/* on ne redescend que dans les vrais sous-dossiers ; ignorer "." et
		   ".." evite la recursion infinie (et les symlinks ne sont pas des
		   dossiers via lstat, donc pas suivis). Un echec de sous-dossier
		   remonte en niveau 1, cumule via le max. */
		if (S_ISDIR(file->st.st_mode) && ft_strncmp(file->name, ".", 2) != 0
			&& ft_strncmp(file->name, "..", 3) != 0
			&& ft_list_one_dir(file->path, opts, 1, printed, 0) > err)
			err = 1;
		cur = cur->next;
	}
	ft_lstclear(&entries, ft_free_file);
	closedir(dir);
	return (err);
}
