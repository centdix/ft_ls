#!/bin/sh
# ============================================================================
#  setup_tree.sh - builds a booby-trapped tree under /tmp/ft_ls_test to
#  exercise ft_ls: hidden files, subdirectories (for -R), symlinks (one
#  broken), varied permissions, sticky/setuid bits, tricky-to-sort names
#  (upper/lower case), and an old file (mtime > 6 months).
#
#  Usage:
#     ./tests/setup_tree.sh
#     ./tests/compare.sh -laR /tmp/ft_ls_test
# ============================================================================

ROOT=/tmp/ft_ls_test
rm -rf "$ROOT"
mkdir -p "$ROOT"/sub1/sub2 "$ROOT"/empty_dir

# Regular files + upper/lower sort (LC_ALL=C => uppercase before lowercase)
printf 'a\n'   > "$ROOT/Bravo"
printf 'bb\n'  > "$ROOT/alpha"
printf 'ccc\n' > "$ROOT/Charlie"
printf 'd\n'   > "$ROOT/.cache_cache"      # hidden file

# Different sizes to test -l column alignment
head -c 100000 /dev/zero > "$ROOT/big_file" 2>/dev/null

# Symlinks (-l must print "link -> target")
ln -s alpha          "$ROOT/link_ok"
ln -s does_not_exist "$ROOT/link_broken"

# Varied permissions
chmod 700 "$ROOT/Bravo"
chmod 644 "$ROOT/alpha"
chmod 755 "$ROOT/Charlie"

# Special bits (sticky / setuid) -> s / t characters in the mode string
chmod +t  "$ROOT/empty_dir"  2>/dev/null
chmod u+s "$ROOT/big_file"   2>/dev/null

# Subdirectory contents (for -R)
printf 'x\n' > "$ROOT/sub1/file_in_sub1"
printf 'y\n' > "$ROOT/sub1/sub2/file_in_sub2"

# "Old" file -> ls must print the year instead of the time
touch -d '2 years ago' "$ROOT/old_file" 2>/dev/null \
	|| touch -t 202001010000 "$ROOT/old_file" 2>/dev/null

echo "Test tree ready: $ROOT"
echo "Try with:  ./tests/compare.sh -laR $ROOT"
