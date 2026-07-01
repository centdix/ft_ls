#ifndef FT_LS_H
#define FT_LS_H

// --- libft (ft_printf bundled in) ---
#include "libft.h"
#include "printf.h"

// --- I/O, directory traversal ---
#include <dirent.h> /* opendir, readdir, closedir, DIR        */
#include <stdio.h>  /* perror                                 */
#include <stdlib.h> /* malloc, free, exit                     */
#include <unistd.h> /* write                                  */

// --- file metadata ---
#include <sys/stat.h>  /* stat, lstat, struct stat, S_IS*, S_I*  */
#include <sys/types.h> /* major, minor (mac: here; Linux: below) */

// --- uid/gid lookups, dates, links ---
#include <errno.h>     /* errno (ls-style error messages)        */
#include <grp.h>       /* getgrgid                               */
#include <pwd.h>       /* getpwuid                               */
#include <string.h>    /* strerror                               */
#include <sys/xattr.h> /* getxattr/lgetxattr (bonus: ACL marker) */
#include <time.h>      /* time, ctime                            */

// --- Linux / macOS portability ---
// struct stat nanosecond fields, the xattr API and the "total" block unit
// differ between glibc/Linux and BSD/macOS. isolate them here so the rest of
// the code stays identical on both
#ifdef __APPLE__
#define ST_ATIM_S(st) ((st).st_atimespec.tv_sec)
#define ST_ATIM_NS(st) ((st).st_atimespec.tv_nsec)
#define ST_MTIM_S(st) ((st).st_mtimespec.tv_sec)
#define ST_MTIM_NS(st) ((st).st_mtimespec.tv_nsec)
#define ST_CTIM_S(st) ((st).st_ctimespec.tv_sec)
#define ST_CTIM_NS(st) ((st).st_ctimespec.tv_nsec)
// BSD ls: "total" in 512-byte blocks (no division)
#define TOTAL_BLOCKS(b) (b)
// BSD ls: 2 spaces between the -l owner/group/size columns
#define COL_GAP "  "
// BSD ls: the xattr-marker column ('@') is ALWAYS reserved in -l (a space when
// the file carries no xattr)
#define ACL_COL_ALWAYS 1
// BSD ls -R: no "path:" header for a single operand-dir
#define REC_FORCES_HEADER 0
// BSD ls: any error (argument, option, open) -> exit code 1
#define RC_ERR 1
// BSD ls formats errors "ls: <path>: <err>" (no reason, no quotes)
#define ERR_REASON(r) ((void)(r))
#define ERR_Q1 ""
#define ERR_Q2 ": "
// BSD ls: short option without quotes, long option in `back-quotes', and a
// "usage:" line (instead of GNU's "Try 'ls --help'")
#define OPT_SQ1 ""
#define OPT_SQ2 ""
#define OPT_LQ1 "`"
#define OPT_LQ2 "'"
#define OPT_HELP                                                               \
  "usage: ls [-@ABCFGHILOPRSTUWabcdefghiklmnopqrstuvwxy1%,]"                   \
  " [--color=when] [-D format] [file ...]\n"
#else
#include <sys/sysmacros.h> /* major, minor on glibc                   */
#define ST_ATIM_S(st) ((st).st_atim.tv_sec)
#define ST_ATIM_NS(st) ((st).st_atim.tv_nsec)
#define ST_MTIM_S(st) ((st).st_mtim.tv_sec)
#define ST_MTIM_NS(st) ((st).st_mtim.tv_nsec)
#define ST_CTIM_S(st) ((st).st_ctim.tv_sec)
#define ST_CTIM_NS(st) ((st).st_ctim.tv_nsec)
// GNU ls: "total" in 1 KB blocks (st_blocks is in 512-byte units)
#define TOTAL_BLOCKS(b) ((b) / 2)
// GNU ls: 1 space between the -l owner/group/size columns
#define COL_GAP " "
// GNU ls: the '+'/'.' column is reserved only if an entry needs it
#define ACL_COL_ALWAYS 0
// GNU ls -R: "path:" header even for a single operand-dir
#define REC_FORCES_HEADER 1
// GNU ls: argument/option error -> exit code 2 (subdir -> 1)
#define RC_ERR 2
// GNU ls formats "ls: <reason> '<path>': <err>"
#define ERR_REASON(r) ft_putstr_fd((r), 2)
#define ERR_Q1 " '"
#define ERR_Q2 "': "
// GNU ls: short/long option in 'single-quotes' + "Try 'ls --help'"
#define OPT_SQ1 "'"
#define OPT_SQ2 "'"
#define OPT_LQ1 "'"
#define OPT_LQ2 "'"
#define OPT_HELP "Try 'ls --help' for more information.\n"
#endif

// extended attributes, WITHOUT following symlinks (describe the link itself,
// like lstat).
//   - X_HAS    : does a specific attribute exist? (>=0 = yes)
//   - X_LIST   : size of the attribute-name list (>0 = at least one xattr)
//   - ACL_CHAR : the 11th char of the -l line.
//       * BSD/mac : '@' as soon as any xattr is present (listxattr). The BSD
//         ACL '+' relies on <sys/acl.h> (not in the allowed list) -> unhandled.
//       * GNU/Linux : '+' for a POSIX ACL, '.' for a security context only.
#ifdef __APPLE__
#define X_HAS(path, name) (getxattr((path), (name), NULL, 0, 0, XATTR_NOFOLLOW))
#define X_LIST(path) (listxattr((path), NULL, 0, XATTR_NOFOLLOW))
#define ACL_CHAR(f) (X_LIST((f)->path) > 0 ? '@' : ' ')
#else
#define X_HAS(path, name) (lgetxattr((path), (name), NULL, 0))
#define X_LIST(path) (llistxattr((path), NULL, 0))
#define ACL_CHAR(f)                                                            \
  ((X_HAS((f)->path, "system.posix_acl_access") >= 0 ||                        \
    X_HAS((f)->path, "system.posix_acl_default") >= 0)                         \
       ? '+'                                                                   \
   : (X_HAS((f)->path, "security.selinux") >= 0 ||                             \
      X_HAS((f)->path, "security.SMACK64") >= 0)                               \
       ? '.'                                                                   \
       : ' ')
#endif

// active sort key (-r direction is handled separately, via opts->rev).
// SORT_NONE = -f (readdir order, no sorting)
typedef enum e_sort_by {
  SORT_NAME,
  SORT_TIME,
  SORT_SIZE,
  SORT_NONE,
} t_sort_by;

// time field used by -l and the -t sort: mtime (default), atime (-u)
// or ctime (-c)
typedef enum e_timek {
  TK_MTIME,
  TK_ATIME,
  TK_CTIME,
} t_timek;

// PATH_FILE/PATH_DIR (not FILE/DIR, already taken by <stdio.h> and <dirent.h>).
// order PATH_FILE < PATH_DIR -> files come before directories
typedef enum e_path_type {
  PATH_FILE,
  PATH_DIR,
} t_path_type;

// --- structs ---
typedef struct s_path {
  char *path;       /* path as given in argv (points into argv)       */
  t_path_type type; /* file or directory (determined via stat)        */
  int staterr;      /* 0 if stat succeeded, else errno (invalid path) */
  struct stat st;   /* metadata (for the -t sort of operands)         */
} t_path;

typedef struct s_file {
  char *name;
  char *path;
  char acl;             /* bonus: '+' (ACL), '.' (context), ' ' (none)    */
  int staterr;          /* 0 if lstat succeeded, else errno (unstattable) */
  unsigned char dtype;  /* readdir d_type (for the type char when staterr) */
  struct stat st;
} t_file;

// behaviour options (only the command-line flags)
typedef struct s_opts {
  int list;       /* -l: long format                         */
  int rec;        /* -R: recursive                           */
  int all;        /* -a: hidden files (. and .. included)    */
  int almost;     /* -A: hidden but . and ..                 */
  int rev;        /* -r: reverse order                       */
  int dironly;    /* -d: the directories themselves, not content */
  int one;        /* -1: one entry per line (already the case) */
  int inode;      /* -i: inode number                        */
  int slash;      /* -p: '/' after directories               */
  int noowner;    /* -g: -l without the owner column         */
  int nogroup;    /* -o: -l without the group column         */
  t_sort_by sort; /* sort key (-t, -S, -f)                   */
  t_timek timek;  /* time field (-u, -c)                     */
} t_opts;

// full context: the flags + the list of operands (targets) to list
typedef struct s_ls {
  t_opts opts;
  t_list *operands; /* linked list of (t_path *) */
} t_ls;

// -l column widths (computed over all entries to align the columns). for
// devices the "size" column holds "major, minor" -> we track major/minor too
typedef struct s_widths {
  int links;
  int owner;
  int group;
  int size;
  int major;
  int minor;
  int inode;  /* bonus -i: inode column width                      */
  int aclcol; /* bonus: 1 if an entry in the block has an ACL/ctx  */
} t_widths;

// --- parse.c: command-line parsing ---
t_ls ft_parse_args(int argc, char **argv);
void ft_free_path(void *content);

// --- sort.c: sort entries and operands (key + direction via opts) ---
int ft_sort_list(t_list *lst, t_opts *opts);
void ft_sort_paths(t_list *paths, t_opts *opts);
time_t ft_pick_time(struct stat *st, t_timek tk);

// --- format.c: long-format (-l) rendering ---
void ft_print_long_list(t_list *entries, int show_total, t_opts *opts);
void ft_calc_widths(t_list *entries, t_widths *w);
void ft_print_long_line_pub(t_file *f, t_widths *w, t_opts *opts);
void ft_print_short_line(t_file *f, t_widths *w, t_opts *opts);

// --- list.c: directory listing and errors ---
t_list *ft_extract_entries(DIR *dir, char *path, t_opts *opts);
int ft_print_list(t_list *lst, t_opts *opts);
void ft_free_file(void *content);
int ft_print_access_errors(t_list *paths);
int ft_list_one_dir(char *path, t_opts *opts, int header, int *printed,
                    int is_arg);
void ft_list_file_operands(t_ls *ls, int *printed);

#endif
