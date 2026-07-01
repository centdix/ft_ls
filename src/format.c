#include "ft_ls.h"

// ls switches to "Mmm jj  YYYY" past ~6 months (else "Mmm jj hh:mm")
#define SIX_MONTHS 15778476

// type char from readdir's d_type, for an entry we couldn't lstat (GNU pulls
// the leading char from d_type and prints '?' for the rest). DT_UNKNOWN -> '?'
static char dtype_char(unsigned char dt) {
  if (dt == DT_DIR)
    return ('d');
  if (dt == DT_LNK)
    return ('l');
  if (dt == DT_CHR)
    return ('c');
  if (dt == DT_BLK)
    return ('b');
  if (dt == DT_FIFO)
    return ('p');
  if (dt == DT_SOCK)
    return ('s');
  if (dt == DT_REG)
    return ('-');
  return ('?');
}

static char type_char(mode_t m) {
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

// build "drwxr-xr-x" (10 chars + '\0'); setuid/setgid/sticky turn the exec bit
// into s/S or t/T
static void mode_to_str(mode_t m, char *s) {
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

// pull the 12-char date field out of ctime() ("Www Mmm jj hh:mm:ss yyyy\n").
// recent -> "Mmm jj hh:mm"; old/future -> "Mmm jj  yyyy"
static void time_str(time_t when, char *out) {
  char *c;
  int i;

  c = ctime(&when);
  i = 0;
  while (i < 6) {
    out[i] = c[4 + i];
    i++;
  }
  out[6] = ' ';
  if (when > time(NULL) || time(NULL) - when > SIX_MONTHS) {
    out[7] = ' ';
    out[8] = c[20];
    out[9] = c[21];
    out[10] = c[22];
    out[11] = c[23];
  } else {
    out[7] = c[11];
    out[8] = c[12];
    out[9] = c[13];
    out[10] = c[14];
    out[11] = c[15];
  }
  out[12] = '\0';
}

// uid -> name (numeric fallback); allocated, caller frees
static char *owner_name(uid_t uid) {
  struct passwd *pw;

  pw = getpwuid(uid);
  if (pw)
    return (ft_strdup(pw->pw_name));
  return (ft_itoa(uid));
}

// gid -> name (numeric fallback); allocated, caller frees
static char *group_name(gid_t gid) {
  struct group *gr;

  gr = getgrgid(gid);
  if (gr)
    return (ft_strdup(gr->gr_name));
  return (ft_itoa(gid));
}

// unsigned long -> string (ft_itoa is int-only, so we handle big sizes by
// hand); allocated, caller frees
static char *nbr_to_str(unsigned long n) {
  char buf[32];
  int i;

  i = 31;
  buf[i] = '\0';
  if (n == 0)
    buf[--i] = '0';
  while (n > 0) {
    buf[--i] = '0' + (n % 10);
    n /= 10;
  }
  return (ft_strdup(&buf[i]));
}

static void print_spaces(int n) {
  while (n-- > 0)
    ft_printf(" ");
}

// numeric column: right-aligned (leading spaces)
static void print_right(char *s, int width) {
  print_spaces(width - (int)ft_strlen(s));
  ft_printf("%s", s);
}

// text column (owner/group): left-aligned (trailing spaces)
static void print_left(char *s, int width) {
  ft_printf("%s", s);
  print_spaces(width - (int)ft_strlen(s));
}

static int is_device(mode_t m) { return (S_ISCHR(m) || S_ISBLK(m)); }

// "size" column width: normal size, or major/minor (tracked separately) for a
// device
static void update_size_width(t_file *f, t_widths *w) {
  char *s;

  if (is_device(f->st.st_mode)) {
    s = nbr_to_str(major(f->st.st_rdev));
    if ((int)ft_strlen(s) > w->major)
      w->major = ft_strlen(s);
    free(s);
    s = nbr_to_str(minor(f->st.st_rdev));
    if ((int)ft_strlen(s) > w->minor)
      w->minor = ft_strlen(s);
    free(s);
    return;
  }
  s = nbr_to_str(f->st.st_size);
  if ((int)ft_strlen(s) > w->size)
    w->size = ft_strlen(s);
  free(s);
}

// GNU ls gates the '.' context marker behind is_selinux_enabled(): if selinuxfs
// is not mounted, ls never shows '.' even when a security.selinux xattr exists
// (e.g. an NFS mount-wide label on '..'). We mirror that with a cached stat of
// the mount point (stat is authorized; access is not).
#ifndef __APPLE__
static int selinux_enabled(void) {
  static int cached = -1;
  struct stat st;

  if (cached == -1)
    cached = (stat("/sys/fs/selinux", &st) == 0);
  return (cached);
}
#endif

// bonus: extended-access indicator (11th column of -l). the logic differs
// GNU ('+'/'.' via POSIX ACL/context) vs BSD ('@' as soon as any xattr is
// present): it is isolated behind the ACL_CHAR macro (see ft_ls.h). doesn't
// follow symlinks (describes the link itself, like lstat)
static char acl_char(t_file *f) { return (ACL_CHAR(f)); }

// an unstattable entry (staterr) contributes a single '?' to every column, so
// its zeroed stat (uid 0 -> "root", etc.) must not widen anything
static void update_widths_unknown(t_widths *w) {
  if (w->inode < 1)
    w->inode = 1;
  if (w->links < 1)
    w->links = 1;
  if (w->owner < 1)
    w->owner = 1;
  if (w->group < 1)
    w->group = 1;
  if (w->size < 1)
    w->size = 1;
}

// update the max widths from an entry (and the ACL indicator)
static void update_widths(t_file *f, t_widths *w) {
  char *s;

  if (f->staterr) {
    update_widths_unknown(w);
    return;
  }
  f->acl = acl_char(f);
  if (f->acl != ' ')
    w->aclcol = 1;
  s = nbr_to_str(f->st.st_ino);
  if ((int)ft_strlen(s) > w->inode)
    w->inode = ft_strlen(s);
  free(s);
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

// "size" column: normal size, or "major, minor" for a device, right-aligned on
// w->size
static void print_size_field(t_file *f, t_widths *w) {
  char *maj;
  char *min;
  char *sz;

  if (is_device(f->st.st_mode)) {
    maj = nbr_to_str(major(f->st.st_rdev));
    min = nbr_to_str(minor(f->st.st_rdev));
    print_spaces(w->size - (w->major + 2 + w->minor));
    print_right(maj, w->major);
    ft_printf(", ");
    print_right(min, w->minor);
    free(maj);
    free(min);
    return;
  }
  sz = nbr_to_str(f->st.st_size);
  print_right(sz, w->size);
  free(sz);
}

// append " -> target" for a symlink; nothing if readlink fails
static void print_symlink_target(t_file *f) {
  char target[1024];
  ssize_t n;

  n = readlink(f->path, target, sizeof(target) - 1);
  if (n < 0)
    return;
  target[n] = '\0';
  ft_printf(" -> %s", target);
}

// -i prefix: inode number right-aligned, then a space ('?' if unstattable)
static void print_inode_prefix(t_file *f, t_widths *w, t_opts *opts) {
  char *nbr;

  if (!opts->inode)
    return;
  if (f->staterr) {
    print_right("?", w->inode);
    ft_printf(" ");
    return;
  }
  nbr = nbr_to_str(f->st.st_ino);
  print_right(nbr, w->inode);
  free(nbr);
  ft_printf(" ");
}

// entry name, with trailing '/' for a dir if -p
static void print_name(t_file *f, t_opts *opts) {
  ft_printf("%s", f->name);
  if (opts->slash && S_ISDIR(f->st.st_mode))
    ft_printf("/");
}

// owner/group columns, unless -g (no owner) / -o (no group)
static void print_owner_group(t_file *f, t_widths *w, t_opts *opts) {
  char *owner;
  char *group;

  if (!opts->noowner) {
    owner = owner_name(f->st.st_uid);
    print_left(owner, w->owner);
    ft_printf(COL_GAP);
    free(owner);
  }
  if (!opts->nogroup) {
    group = group_name(f->st.st_gid);
    print_left(group, w->group);
    ft_printf(COL_GAP);
    free(group);
  }
}

// GNU-style long line for an entry we couldn't lstat: type char (from d_type)
// then '?' in every field, date '?' right-justified in the 12-wide date column
static void print_long_unknown(t_file *f, t_widths *w, t_opts *opts) {
  print_inode_prefix(f, w, opts);
  ft_printf("%c?????????", dtype_char(f->dtype));
  if (w->aclcol)
    ft_printf(" ");
  ft_printf(" ");
  print_right("?", w->links);
  ft_printf(" ");
  if (!opts->noowner) {
    print_left("?", w->owner);
    ft_printf(COL_GAP);
  }
  if (!opts->nogroup) {
    print_left("?", w->group);
    ft_printf(COL_GAP);
  }
  print_right("?", w->size);
  ft_printf(" ");
  print_right("?", 12);
  ft_printf(" %s\n", f->name);
}

// print one long-format line, columns aligned on w
static void print_long_line(t_file *f, t_widths *w, t_opts *opts) {
  char perms[11];
  char date[13];
  char *nbr;

  if (f->staterr) {
    print_long_unknown(f, w, opts);
    return;
  }
  mode_to_str(f->st.st_mode, perms);
  time_str(ft_pick_time(&f->st, opts->timek), date);
  print_inode_prefix(f, w, opts);
  ft_printf("%s", perms);
  if (w->aclcol)
    ft_printf("%c", f->acl);
  ft_printf(" ");
  nbr = nbr_to_str(f->st.st_nlink);
  print_right(nbr, w->links);
  free(nbr);
  ft_printf(" ");
  print_owner_group(f, w, opts);
  print_size_field(f, w);
  ft_printf(" %s ", date);
  print_name(f, opts);
  if (S_ISLNK(f->st.st_mode))
    print_symlink_target(f);
  ft_printf("\n");
}

// short-format line: (inode) + name (+ '/')
void ft_print_short_line(t_file *f, t_widths *w, t_opts *opts) {
  print_inode_prefix(f, w, opts);
  print_name(f, opts);
  ft_printf("\n");
}

// calculate the widths of the -l on the entire set of entries provided
// (1st pass, before any printing). The "size" column must accommodate
// "major, minor" in integer: major + ", " (2 chars) + minor, like GNU ls.
void ft_calc_widths(t_list *entries, t_widths *w) {
  *w = (t_widths){0, 0, 0, 0, 0, 0, 0, 0};
  // BSD always reserves the xattr-marker column; GNU only if an entry needs it
  w->aclcol = ACL_COL_ALWAYS;
  while (entries) {
    update_widths(entries->content, w);
    entries = entries->next;
  }
  if (w->major > 0 && w->major + 2 + w->minor > w->size)
    w->size = w->major + 2 + w->minor;
}

// sum of blocks for the "total" line; st_blocks is in 512-byte units
static unsigned long sum_blocks(t_list *entries) {
  unsigned long blocks;

  blocks = 0;
  while (entries) {
    blocks += ((t_file *)entries->content)->st.st_blocks;
    entries = entries->next;
  }
  return (blocks);
}

// inode column width for the short format (the others aren't needed)
static int inode_width(t_list *lst) {
  int w;
  char *s;

  w = 0;
  while (lst) {
    s = nbr_to_str(((t_file *)lst->content)->st.st_ino);
    if ((int)ft_strlen(s) > w)
      w = ft_strlen(s);
    free(s);
    lst = lst->next;
  }
  return (w);
}

// short format (no -l): one entry per line, with inode/'/' per options.
// computes its own inode width (no needless ACL lookups here)
int ft_print_list(t_list *lst, t_opts *opts) {
  t_widths w;

  w = (t_widths){0, 0, 0, 0, 0, 0, 0, 0};
  if (opts->inode)
    w.inode = inode_width(lst);
  while (lst) {
    ft_print_short_line(lst->content, &w, opts);
    lst = lst->next;
  }
  return (0);
}

// print an already-prepared entry with precomputed widths (used for the
// operand-files block, whose widths also include operand-dirs, GNU-style)
void ft_print_long_line_pub(t_file *f, t_widths *w, t_opts *opts) {
  print_long_line(f, w, opts);
}

void ft_print_long_list(t_list *entries, int show_total, t_opts *opts) {
  t_widths w;
  t_list *cur;
  char *nbr;

  // entries may be NULL -> empty dir, "total 0"
  ft_calc_widths(entries, &w);
  // "total" block unit per OS (GNU: 1 KB; BSD/mac: 512 B)
  if (show_total) {
    nbr = nbr_to_str(TOTAL_BLOCKS(sum_blocks(entries)));
    ft_printf("total %s\n", nbr);
    free(nbr);
  }
  cur = entries;
  while (cur) {
    print_long_line(cur->content, &w, opts);
    cur = cur->next;
  }
}
