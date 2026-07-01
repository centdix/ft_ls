#include "ft_ls.h"

// active time field: mtime (default), atime (-u) or ctime (-c)
time_t ft_pick_time(struct stat *st, t_timek tk) {
  if (tk == TK_ATIME)
    return (ST_ATIM_S(*st));
  if (tk == TK_CTIME)
    return (ST_CTIM_S(*st));
  return (ST_MTIM_S(*st));
}

// nanoseconds of the active time field
static long pick_nsec(struct stat *st, t_timek tk) {
  if (tk == TK_ATIME)
    return (ST_ATIM_NS(*st));
  if (tk == TK_CTIME)
    return (ST_CTIM_NS(*st));
  return (ST_MTIM_NS(*st));
}

// -t/-u/-c: newest first, compared down to the nanosecond
static int cmp_time(struct stat *a, struct stat *b, t_timek tk) {
  time_t ta;
  time_t tb;

  ta = ft_pick_time(a, tk);
  tb = ft_pick_time(b, tk);
  if (ta != tb)
    return (ta > tb ? -1 : 1);
  if (pick_nsec(a, tk) != pick_nsec(b, tk))
    return (pick_nsec(a, tk) > pick_nsec(b, tk) ? -1 : 1);
  return (0);
}

// -S: largest first
static int cmp_size(struct stat *a, struct stat *b) {
  if (a->st_size != b->st_size)
    return (a->st_size > b->st_size ? -1 : 1);
  return (0);
}

// compare by active key, fall back to name on tie (-r is applied by the caller)
static int cmp_files(t_file *a, t_file *b, t_opts *opts) {
  int c;

  c = 0;
  if (opts->sort == SORT_TIME)
    c = cmp_time(&a->st, &b->st, opts->timek);
  else if (opts->sort == SORT_SIZE)
    c = cmp_size(&a->st, &b->st);
  if (c != 0)
    return (c);
  return (ft_strncmp(a->name, b->name, ft_strlen(a->name) + 1));
}

// bubble sort swapping node contents, not the nodes themselves.
// -f (SORT_NONE) keeps the readdir order (no sorting)
int ft_sort_list(t_list *lst, t_opts *opts) {
  t_list *cur;
  void *swap;
  int cmp;
  int swapped;

  if (!lst || opts->sort == SORT_NONE)
    return (0);
  swapped = 1;
  while (swapped) {
    swapped = 0;
    cur = lst;
    while (cur->next) {
      cmp = cmp_files(cur->content, cur->next->content, opts);
      if (opts->rev)
        cmp = -cmp;
      if (cmp > 0) {
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

static int cmp_paths(t_path *a, t_path *b, t_opts *opts) {
  int cmp;

  // files before directories
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

void ft_sort_paths(t_list *paths, t_opts *opts) {
  t_list *cur;
  void *swap;
  int swapped;

  if (!paths || opts->sort == SORT_NONE)
    return;
  swapped = 1;
  while (swapped) {
    swapped = 0;
    cur = paths;
    while (cur->next) {
      if (cmp_paths(cur->content, cur->next->content, opts) > 0) {
        swap = cur->content;
        cur->content = cur->next->content;
        cur->next->content = swap;
        swapped = 1;
      }
      cur = cur->next;
    }
  }
}
