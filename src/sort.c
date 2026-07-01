/* sort.c — tri des entrees d'un dossier (ft_sort_list) et des operandes de la
   ligne de commande (ft_sort_paths), facon ls. */

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
int	ft_sort_list(t_list *lst, int by_time, int rev)
{
	t_sort_by	by;
	t_list		*cur;
	void		*swap;
	int			cmp;
	int			swapped;

	if (!lst)
		return (0);
	by = by_time ? TIME : NAME;
	swapped = 1;
	while (swapped)
	{
		swapped = 0;
		cur = lst;
		while (cur->next)
		{
			cmp = cmp_files(cur->content, cur->next->content, by);
			if (rev)
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

/* Ordre des operandes facon ls : fichiers avant dossiers (groupage jamais
   inverse par -r), puis cle active (nom, ou date si -t) dans chaque groupe. */
static int	cmp_paths(t_path *a, t_path *b, int by_time, int rev)
{
	int	cmp;

	if (a->type != b->type)
		return ((int)a->type - (int)b->type);
	cmp = 0;
	if (by_time)
		cmp = cmp_mtime(&a->st, &b->st);
	if (cmp == 0)
		cmp = ft_strncmp(a->path, b->path, ft_strlen(a->path) + 1);
	if (rev)
		cmp = -cmp;
	return (cmp);
}

void	ft_sort_paths(t_list *paths, int by_time, int rev)
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
			if (cmp_paths(cur->content, cur->next->content, by_time, rev) > 0)
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
