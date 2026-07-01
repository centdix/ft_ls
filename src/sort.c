/* sort.c — tri des entrees d'un dossier (ft_sort_list) et des operandes de la
   ligne de commande (ft_sort_paths), facon ls. */

#include "ft_ls.h"

/* Champ temporel actif : mtime (defaut), atime (-u) ou ctime (-c). */
time_t	ft_pick_time(struct stat *st, t_timek tk)
{
	if (tk == TK_ATIME)
		return (ST_ATIM_S(*st));
	if (tk == TK_CTIME)
		return (ST_CTIM_S(*st));
	return (ST_MTIM_S(*st));
}

/* Nanosecondes du champ temporel actif (depart de -t a la nanoseconde). */
static long	pick_nsec(struct stat *st, t_timek tk)
{
	if (tk == TK_ATIME)
		return (ST_ATIM_NS(*st));
	if (tk == TK_CTIME)
		return (ST_CTIM_NS(*st));
	return (ST_MTIM_NS(*st));
}

/* Tri -t/-u/-c facon ls : plus recent d'abord, depart a la nanoseconde. */
static int	cmp_time(struct stat *a, struct stat *b, t_timek tk)
{
	time_t	ta;
	time_t	tb;

	ta = ft_pick_time(a, tk);
	tb = ft_pick_time(b, tk);
	if (ta != tb)
		return (ta > tb ? -1 : 1);
	if (pick_nsec(a, tk) != pick_nsec(b, tk))
		return (pick_nsec(a, tk) > pick_nsec(b, tk) ? -1 : 1);
	return (0);
}

/* Tri -S : plus gros d'abord. */
static int	cmp_size(struct stat *a, struct stat *b)
{
	if (a->st_size != b->st_size)
		return (a->st_size > b->st_size ? -1 : 1);
	return (0);
}

/* Compare deux entrees selon la cle active ; a egalite on retombe sur le nom.
   Le -r est applique par l'appelant. */
static int	cmp_files(t_file *a, t_file *b, t_opts *opts)
{
	int	c;

	c = 0;
	if (opts->sort == SORT_TIME)
		c = cmp_time(&a->st, &b->st, opts->timek);
	else if (opts->sort == SORT_SIZE)
		c = cmp_size(&a->st, &b->st);
	if (c != 0)
		return (c);
	return (ft_strncmp(a->name, b->name, ft_strlen(a->name) + 1));
}

/* Tri a bulles : on echange le contenu des noeuds, pas les noeuds eux-memes.
   -f (SORT_NONE) conserve l'ordre du readdir (aucun tri). */
int	ft_sort_list(t_list *lst, t_opts *opts)
{
	t_list	*cur;
	void	*swap;
	int		cmp;
	int		swapped;

	if (!lst || opts->sort == SORT_NONE)
		return (0);
	swapped = 1;
	while (swapped)
	{
		swapped = 0;
		cur = lst;
		while (cur->next)
		{
			cmp = cmp_files(cur->content, cur->next->content, opts);
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

/* Ordre des operandes facon ls : fichiers avant dossiers (groupage jamais
   inverse par -r), puis cle active dans chaque groupe. En -d, pas de groupage
   (les dossiers sont traites comme des entrees ordinaires). */
static int	cmp_paths(t_path *a, t_path *b, t_opts *opts)
{
	int	cmp;

	/* groupage type (fichiers < dossiers) resolu AVANT -r -> jamais inverse. */
	if (!opts->dironly && a->type != b->type)
		return ((int)a->type - (int)b->type);
	cmp = 0;
	if (opts->sort == SORT_TIME)
		cmp = cmp_time(&a->st, &b->st, opts->timek);
	else if (opts->sort == SORT_SIZE)
		cmp = cmp_size(&a->st, &b->st);
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

	if (!paths || opts->sort == SORT_NONE)
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
