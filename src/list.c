#include "ft_ls.h"

static char *path_join(char *dir, char *name) {
  char *tmp;
  char *res;

  tmp = ft_strjoin(dir, "/");
  if (!tmp)
    return (NULL);
  res = ft_strjoin(tmp, name);
  free(tmp);
  return (res);
}

static void ls_error(char *path, char *reason, int err) {
  ft_putstr_fd("ls: ", 2);
  ERR_REASON(reason);
  ft_putstr_fd(ERR_Q1, 2);
  ft_putstr_fd(path, 2);
  ft_putstr_fd(ERR_Q2, 2);
  ft_putstr_fd(strerror(err), 2);
  ft_putstr_fd("\n", 2);
}

// -a: everything; -A: all but "." and ".."; default: hide dotfiles
static int show_entry(char *name, t_opts *opts) {
  if (name[0] != '.')
    return (1);
  if (opts->all)
    return (1);
  if (opts->almost && ft_strncmp(name, ".", 2) != 0 &&
      ft_strncmp(name, "..", 3) != 0)
    return (1);
  return (0);
}

// t_file for a dir entry; NULL on alloc failure. on lstat failure (vanished
// file / lost perms) zero st and record errno in staterr, so -l/-i can render
// GNU-style '?' columns instead of reading uninitialized fields
static t_file *make_entry(char *path, char *name, unsigned char dtype) {
  t_file *file;

  file = malloc(sizeof(t_file));
  if (!file)
    return (NULL);
  file->name = ft_strdup(name);
  file->path = path_join(path, name);
  file->acl = ' ';
  file->staterr = 0;
  file->dtype = dtype;
  if (!file->name || !file->path) {
    ft_free_file(file);
    return (NULL);
  }
  // lstat, not stat: describe the link itself, not its target
  if (lstat(file->path, &file->st) != 0) {
    file->staterr = errno;
    ft_bzero(&file->st, sizeof(file->st));
  }
  return (file);
}

t_list *ft_extract_entries(DIR *dir, char *path, t_opts *opts) {
  t_list *entries;
  struct dirent *entry;
  t_file *file;
  t_list *node;

  entries = NULL;
  errno = 0;
  while ((entry = readdir(dir)) != NULL) {
    if (show_entry(entry->d_name, opts)) {
      file = make_entry(path, entry->d_name, entry->d_type);
      node = NULL;
      if (file)
        node = ft_lstnew(file);
      if (!node) {
        if (file)
          ft_free_file(file);
        ft_lstclear(&entries, ft_free_file);
        return (NULL);
      }
      ft_lstadd_back(&entries, node);
    }
    errno = 0;
  }
  if (errno != 0)
    ls_error(path, "reading directory", errno);
  return (entries);
}

void ft_free_file(void *content) {
  t_file *file;

  file = content;
  free(file->name);
  free(file->path);
  free(file);
}

static t_file *make_operand(t_path *op) {
  t_file *f;

  f = malloc(sizeof(t_file));
  if (!f)
    return (NULL);
  f->name = ft_strdup(op->path);
  f->path = ft_strdup(op->path);
  f->acl = ' ';
  f->staterr = 0;
  f->dtype = 0;
  f->st = op->st;
  if (!f->name || !f->path) {
    ft_free_file(f);
    return (NULL);
  }
  return (f);
}

// convert valid operands (files and directories) to t_file
static t_list *collect_operand_files(t_list *operands) {
  t_list *all;
  t_file *f;
  t_list *node;

  all = NULL;
  while (operands) {
    if (((t_path *)operands->content)->staterr == 0) {
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

static int printable(t_file *f, t_opts *opts) {
  return (opts->dironly || !S_ISDIR(f->st.st_mode));
}

// there is at least one entry to display in the operands block
static int has_printable(t_list *all, t_opts *opts) {
  while (all) {
    if (printable(all->content, opts))
      return (1);
    all = all->next;
  }
  return (0);
}

// print the operands block, widths already calculated on the entire set
static void print_operand_files(t_list *all, t_opts *opts, t_widths *w) {
  t_file *f;

  while (all) {
    f = all->content;
    if (printable(f, opts)) {
      if (opts->list)
        ft_print_long_line_pub(f, w, opts);
      else
        ft_print_short_line(f, w, opts);
    }
    all = all->next;
  }
}

void ft_list_file_operands(t_ls *ls, int *printed) {
  t_list *all;
  t_widths w;

  all = collect_operand_files(ls->operands);
  if (!has_printable(all, &ls->opts)) {
    ft_lstclear(&all, ft_free_file);
    return;
  }
  ft_calc_widths(all, &w);
  print_operand_files(all, &ls->opts, &w);
  *printed = 1;
  ft_lstclear(&all, ft_free_file);
}

int ft_print_access_errors(t_list *paths) {
  t_path *op;
  int err;

  err = 0;
  while (paths) {
    op = paths->content;
    if (op->staterr != 0) {
      ls_error(op->path, "cannot access", op->staterr);
      err = 1;
    }
    paths = paths->next;
  }
  return (err);
}

// GNU emits "ls: cannot access '<path>': <err>" per unstattable entry and
// exits 1 -- but only when it actually needs the metadata (-l, -i, or a
// time/size sort). A plain name-sorted short listing never stats -> no error.
// errors are printed in readdir order (this list is still unsorted here)
static int report_stat_errors(t_list *entries, t_opts *opts) {
  t_file *f;
  int err;

  if (!(opts->list || opts->inode || opts->sort == SORT_TIME ||
        opts->sort == SORT_SIZE))
    return (0);
  err = 0;
  while (entries) {
    f = entries->content;
    if (f->staterr) {
      ls_error(f->path, "cannot access", f->staterr);
      err = 1;
    }
    entries = entries->next;
  }
  return (err);
}

// list a directory: separator + optional header + sorted content, then recurse
// if -R.
int ft_list_one_dir(char *path, t_opts *opts, int header, int *printed,
                    int is_arg) {
  DIR *dir;
  t_list *entries;
  t_list *cur;
  t_file *file;
  int err;

  dir = opendir(path);
  if (!dir) {
    ls_error(path, "cannot open directory", errno);
    return (is_arg ? RC_ERR : 1);
  }
  // blank separator line if a block was already printed before this one
  if (*printed)
    ft_printf("\n");
  if (header)
    ft_printf("%s:\n", path);
  entries = ft_extract_entries(dir, path, opts);
  err = report_stat_errors(entries, opts);
  ft_sort_list(entries, opts);
  // 1 = print the "total" line (this is a directory's content)
  if (opts->list)
    ft_print_long_list(entries, 1, opts);
  else
    ft_print_list(entries, opts);
  *printed = 1;
  cur = entries;
  while (opts->rec && cur) {
    file = cur->content;
    // recurse if -R
    if (S_ISDIR(file->st.st_mode) && ft_strncmp(file->name, ".", 2) != 0 &&
        ft_strncmp(file->name, "..", 3) != 0 &&
        ft_list_one_dir(file->path, opts, 1, printed, 0) > err)
      err = 1;
    cur = cur->next;
  }
  ft_lstclear(&entries, ft_free_file);
  closedir(dir);
  return (err);
}
