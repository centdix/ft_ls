#include "ft_ls.h"

// option error on stderr, then exit (no listing). the 2nd-line text and exit
// code differ GNU/BSD (see OPT_HELP / RC_ERR)
static void option_error_exit(void) {
  ft_putstr_fd(OPT_HELP, 2);
  exit(RC_ERR);
}

static void invalid_short_option(char c) {
  ft_putstr_fd("ls: invalid option -- ", 2);
  ft_putstr_fd(OPT_SQ1, 2);
  ft_putchar_fd(c, 2);
  ft_putstr_fd(OPT_SQ2, 2);
  ft_putstr_fd("\n", 2);
  option_error_exit();
}

static void unrecognized_long_option(char *arg) {
  ft_putstr_fd("ls: unrecognized option ", 2);
  ft_putstr_fd(OPT_LQ1, 2);
  ft_putstr_fd(arg, 2);
  ft_putstr_fd(OPT_LQ2, 2);
  ft_putstr_fd("\n", 2);
  option_error_exit();
}

// one flag, one effect. sort keys (-t/-S/-f) overwrite in read order, like ls.
// -g/-o imply the long format (-l). -f forces -a and disables sorting (without
// disabling -l, matching current ls)
static int set_flag(char c, t_opts *opts) {
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

static void parse_flags(char *arg, t_opts *opts) {
  int i;

  i = 1;
  while (arg[i]) {
    if (!set_flag(arg[i], opts))
      invalid_short_option(arg[i]);
    i++;
  }
}

// t_path for an operand (no stat yet: options may follow the operand in argv,
// e.g. "ls /bin -l" -> we stat after the full parse). op->path points into
// argv, so nothing to free
static t_path *new_path(char *str) {
  t_path *op;

  op = malloc(sizeof(t_path));
  if (!op)
    return (NULL);
  op->path = str;
  op->type = PATH_FILE;
  op->staterr = 0;
  ft_bzero(&op->st, sizeof(op->st));
  return (op);
}

// fill st/type/staterr ls-style. when ls doesn't expand symlink-arguments
// (-l, -d ...), don't follow (show the link) unless a trailing slash; otherwise
// follow, and if broken (stat fails) fall back to lstat to still show it
// (return code 0, like ls)
static void stat_operand(t_path *op, int nofollow) {
  size_t len;

  len = ft_strlen(op->path);
  if (nofollow && len > 0 && op->path[len - 1] != '/') {
    if (lstat(op->path, &op->st) != 0)
      op->staterr = errno;
    else if (S_ISDIR(op->st.st_mode))
      op->type = PATH_DIR;
    return;
  }
  if (stat(op->path, &op->st) == 0) {
    if (S_ISDIR(op->st.st_mode))
      op->type = PATH_DIR;
  } else if (lstat(op->path, &op->st) != 0)
    op->staterr = errno;
}

// stat every operand once all options are known
static void stat_operands(t_list *ops, int nofollow) {
  t_path *op;

  while (ops) {
    op = ops->content;
    if (op)
      stat_operand(op, nofollow);
    ops = ops->next;
  }
}

// append an operand, handling alloc failure cleanly (never a node with NULL
// content -> no deref later)
static void add_operand(t_list **operands, char *str) {
  t_path *op;
  t_list *node;

  op = new_path(str);
  node = ft_lstnew(op);
  if (op && node)
    ft_lstadd_back(operands, node);
  else {
    free(op);
    free(node);
  }
}

t_ls ft_parse_args(int argc, char **argv) {
  t_ls ls;
  int i;
  int endopt;

  ft_bzero(&ls.opts, sizeof(ls.opts));
  ls.operands = NULL;
  endopt = 0;
  i = 1;
  while (i < argc) {
    // "--" alone ends options; "--xxx" = unknown long option;
    // "-xxx" = short options; "-" alone stays an operand (file)
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
  // -u/-c without -t sort by the chosen time (atime/ctime), except with -l
  // where sorting stays by name (like ls). -S/-f keep their key
  if (ls.opts.timek != TK_MTIME && ls.opts.sort == SORT_NAME && !ls.opts.list)
    ls.opts.sort = SORT_TIME;
  // no operand given -> list the current directory
  if (!ls.operands)
    add_operand(&ls.operands, ".");
  // deferred stat: all options are known now (-l and -d affect the
  // (non-)deref of symlinks passed as arguments)
  stat_operands(ls.operands, ls.opts.list || ls.opts.dironly);
  return (ls);
}

// free the t_path but not op->path (it points into argv)
void ft_free_path(void *content) { free(content); }
