/* format.c — rendu d'une entree au format long (-l) : permissions, nombre de
   liens, proprietaire, groupe, taille, date, nom (et cible si symlink), plus
   le calcul des largeurs de colonnes et la ligne "total". */

#include "ft_ls.h"

/* ls bascule "Mmm jj  AAAA" au-dela d'env. 6 mois (sinon "Mmm jj hh:mm"). */
#define SIX_MONTHS 15778476

static char	type_char(mode_t m)
{
	if (S_ISDIR(m))
		return ('d');
	if (S_ISLNK(m))
		return ('l');
	if (S_ISCHR(m))
		return ('c');
	if (S_ISBLK(m))
		return ('b');
	if (S_ISFIFO(m))
		return ('p');
	if (S_ISSOCK(m))
		return ('s');
	return ('-');
}

/* Construit "drwxr-xr-x" (10 chars + '\0'), en tenant compte de setuid /
   setgid / sticky qui transforment le bit d'execution en s/S ou t/T. */
static void	mode_to_str(mode_t m, char *s)
{
	s[0] = type_char(m);
	s[1] = (m & S_IRUSR) ? 'r' : '-';
	s[2] = (m & S_IWUSR) ? 'w' : '-';
	s[3] = (m & S_IXUSR) ? 'x' : '-';
	s[4] = (m & S_IRGRP) ? 'r' : '-';
	s[5] = (m & S_IWGRP) ? 'w' : '-';
	s[6] = (m & S_IXGRP) ? 'x' : '-';
	s[7] = (m & S_IROTH) ? 'r' : '-';
	s[8] = (m & S_IWOTH) ? 'w' : '-';
	s[9] = (m & S_IXOTH) ? 'x' : '-';
	if (m & S_ISUID)
		s[3] = (m & S_IXUSR) ? 's' : 'S';
	if (m & S_ISGID)
		s[6] = (m & S_IXGRP) ? 's' : 'S';
	if (m & S_ISVTX)
		s[9] = (m & S_IXOTH) ? 't' : 'T';
	s[10] = '\0';
}

/* Extrait le champ date de 12 caracteres de ctime() ("Www Mmm jj hh:mm:ss
   aaaa\n"). Recent -> "Mmm jj hh:mm" ; ancien/futur -> "Mmm jj  aaaa". */
static void	time_str(time_t when, char *out)
{
	char	*c;
	int		i;

	c = ctime(&when);
	i = 0;
	while (i < 6)
	{
		out[i] = c[4 + i];
		i++;
	}
	out[6] = ' ';
	if (when > time(NULL) || time(NULL) - when > SIX_MONTHS)
	{
		out[7] = ' ';
		out[8] = c[20];
		out[9] = c[21];
		out[10] = c[22];
		out[11] = c[23];
	}
	else
	{
		out[7] = c[11];
		out[8] = c[12];
		out[9] = c[13];
		out[10] = c[14];
		out[11] = c[15];
	}
	out[12] = '\0';
}

/* uid -> nom (ou uid numerique en fallback). Resultat alloue, a free. */
static char	*owner_name(uid_t uid)
{
	struct passwd	*pw;

	pw = getpwuid(uid);
	if (pw)
		return (ft_strdup(pw->pw_name));
	return (ft_itoa(uid));
}

/* gid -> nom (ou gid numerique en fallback). Resultat alloue, a free. */
static char	*group_name(gid_t gid)
{
	struct group	*gr;

	gr = getgrgid(gid);
	if (gr)
		return (ft_strdup(gr->gr_name));
	return (ft_itoa(gid));
}

/* Entier non signe -> chaine (ft_itoa est limite a int : on gere les
   grandes tailles a la main). Resultat alloue, a free. */
static char	*nbr_to_str(unsigned long n)
{
	char	buf[32];
	int		i;

	i = 31;
	buf[i] = '\0';
	if (n == 0)
		buf[--i] = '0';
	while (n > 0)
	{
		buf[--i] = '0' + (n % 10);
		n /= 10;
	}
	return (ft_strdup(&buf[i]));
}

static void	print_spaces(int n)
{
	while (n-- > 0)
		ft_printf(" ");
}

/* Colonne numerique : alignee a droite (espaces avant). */
static void	print_right(char *s, int width)
{
	print_spaces(width - (int)ft_strlen(s));
	ft_printf("%s", s);
}

/* Colonne texte (owner/group) : alignee a gauche (espaces apres). */
static void	print_left(char *s, int width)
{
	ft_printf("%s", s);
	print_spaces(width - (int)ft_strlen(s));
}

static int	is_device(mode_t m)
{
	return (S_ISCHR(m) || S_ISBLK(m));
}

/* Largeur de la valeur affichee dans la colonne "size" : taille classique, ou
   major/minor (suivis separement) pour un peripherique. */
static void	update_size_width(t_file *f, t_widths *w)
{
	char	*s;

	if (is_device(f->st.st_mode))
	{
		s = nbr_to_str(major(f->st.st_rdev));
		if ((int)ft_strlen(s) > w->major)
			w->major = ft_strlen(s);
		free(s);
		s = nbr_to_str(minor(f->st.st_rdev));
		if ((int)ft_strlen(s) > w->minor)
			w->minor = ft_strlen(s);
		free(s);
		return ;
	}
	s = nbr_to_str(f->st.st_size);
	if ((int)ft_strlen(s) > w->size)
		w->size = ft_strlen(s);
	free(s);
}

/* Met a jour les largeurs max a partir d'une entree. */
static void	update_widths(t_file *f, t_widths *w)
{
	char	*s;

	s = nbr_to_str(f->st.st_nlink);
	if ((int)ft_strlen(s) > w->links)
		w->links = ft_strlen(s);
	free(s);
	s = owner_name(f->st.st_uid);
	if ((int)ft_strlen(s) > w->owner)
		w->owner = ft_strlen(s);
	free(s);
	s = group_name(f->st.st_gid);
	if ((int)ft_strlen(s) > w->group)
		w->group = ft_strlen(s);
	free(s);
	update_size_width(f, w);
}

/* Colonne "size" : taille classique, ou "major, minor" pour un peripherique,
   le tout aligne a droite sur w->size. */
static void	print_size_field(t_file *f, t_widths *w)
{
	char	*maj;
	char	*min;
	char	*sz;

	if (is_device(f->st.st_mode))
	{
		maj = nbr_to_str(major(f->st.st_rdev));
		min = nbr_to_str(minor(f->st.st_rdev));
		print_spaces(w->size - (w->major + 2 + w->minor));
		print_right(maj, w->major);
		ft_printf(", ");
		print_right(min, w->minor);
		free(maj);
		free(min);
		return ;
	}
	sz = nbr_to_str(f->st.st_size);
	print_right(sz, w->size);
	free(sz);
}

/* Ajoute " -> cible" pour un lien symbolique (lstat a deja dit que c'en est
   un). En cas d'echec de readlink, on n'ajoute rien. */
static void	print_symlink_target(t_file *f)
{
	char	target[1024];
	ssize_t	n;

	n = readlink(f->path, target, sizeof(target) - 1);
	if (n < 0)
		return ;
	target[n] = '\0';
	ft_printf(" -> %s", target);
}

/* Imprime une ligne du format long, colonnes alignees sur w. */
static void	print_long_line(t_file *f, t_widths *w)
{
	char	perms[11];
	char	date[13];
	char	*owner;
	char	*group;
	char	*nbr;

	mode_to_str(f->st.st_mode, perms);
	time_str(f->st.st_mtim.tv_sec, date);
	owner = owner_name(f->st.st_uid);
	group = group_name(f->st.st_gid);
	ft_printf("%s ", perms);
	nbr = nbr_to_str(f->st.st_nlink);
	print_right(nbr, w->links);
	free(nbr);
	ft_printf(" ");
	print_left(owner, w->owner);
	ft_printf(" ");
	print_left(group, w->group);
	ft_printf(" ");
	print_size_field(f, w);
	ft_printf(" %s %s", date, f->name);
	if (S_ISLNK(f->st.st_mode))
		print_symlink_target(f);
	ft_printf("\n");
	free(owner);
	free(group);
}

void	ft_print_long_list(t_list *entries, int show_total)
{
	t_widths		w;
	t_list			*cur;
	unsigned long	blocks;
	char			*nbr;

	w = (t_widths){0, 0, 0, 0, 0, 0};
	blocks = 0;
	cur = entries;
	while (cur)
	{
		update_widths(cur->content, &w);
		blocks += ((t_file *)cur->content)->st.st_blocks;
		cur = cur->next;
	}
	/* La colonne "size" ne reserve que major + virgule + minor : le ", " du
	   rendu fait deborder les lignes device d'un caractere, comme GNU ls. */
	if (w.major > 0 && w.major + 1 + w.minor > w.size)
		w.size = w.major + 1 + w.minor;
	if (show_total)
	{
		nbr = nbr_to_str(blocks / 2);
		ft_printf("total %s\n", nbr);
		free(nbr);
	}
	cur = entries;
	while (cur)
	{
		print_long_line(cur->content, &w);
		cur = cur->next;
	}
}
