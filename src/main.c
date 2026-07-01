#include "ft_ls.h"

int main(int argc, char **argv) {
  t_ls ls;
  t_list *node;
  int printed;
  int show_header;
  int err;
  int lvl;

  ls = ft_parse_args(argc, argv);

  // print access errors first
  err = 0;
  if (ft_print_access_errors(ls.operands))
    err = RC_ERR;

  // sort paths, files first then directories
  ft_sort_paths(ls.operands, &ls.opts);

  // used to insert the empty line separator between two blocks
  printed = 0;

  // direcory header is displayed if there are multiple targets or the -R option
  // is used for a single target
  show_header =
      (ft_lstsize(ls.operands) > 1 || (ls.opts.rec && REC_FORCES_HEADER));

  // print all files first, then directories
  ft_list_file_operands(&ls, &printed);

  // directories have been printed as entries above, so we don't develop their
  // content
  node = ls.operands;
  while (node && !ls.opts.dironly) {
    t_path *operand = node->content;

    // here we only develop valid directories, everthing else has been printed
    // above
    if (operand->staterr == 0 && operand->type == PATH_DIR) {
      lvl = ft_list_one_dir(operand->path, &ls.opts, show_header, &printed, 1);
      if (lvl > err)
        err = lvl;
    }
    node = node->next;
  }
  ft_lstclear(&ls.operands, ft_free_path);

  // return code like ls: RC_ERR (argument error; GNU 2 / BSD 1), 1
  // (subdirectory error in recursion), 0 otherwise
  return (err);
}
