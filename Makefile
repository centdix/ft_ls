# ************************************************************************** #
#                                                                            #
#   Makefile — ft_ls                                                         #
#                                                                            #
#   Regles : all, clean, fclean, re (+ recompilation conditionnelle).        #
#   Compile d'abord la libft, puis ft_ls.                                    #
#                                                                            #
# ************************************************************************** #

NAME		= ft_ls

CC			= cc
CFLAGS		= -Wall -Wextra -Werror

LIBFT_DIR	= libft
LIBFT		= $(LIBFT_DIR)/libft.a

INC			= -Iincludes -I$(LIBFT_DIR)

SRC_DIR		= src
SRCS		= $(SRC_DIR)/main.c \
			  $(SRC_DIR)/parse.c \
			  $(SRC_DIR)/sort.c \
			  $(SRC_DIR)/list.c \
			  $(SRC_DIR)/format.c
OBJS		= $(SRCS:.c=.o)

HEADER		= includes/ft_ls.h

all: $(NAME)

# Compile la libft (bonus inclus -> les fonctions de liste sont disponibles)
$(LIBFT):
	$(MAKE) -C $(LIBFT_DIR) bonus

$(NAME): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -o $(NAME)

# Recompilation conditionnelle : un .o depend de son .c et du header
%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	$(MAKE) -C $(LIBFT_DIR) clean
	rm -f $(OBJS)

fclean: clean
	$(MAKE) -C $(LIBFT_DIR) fclean
	rm -f $(NAME)

re: fclean all

# Genere compile_commands.json pour clangd (indexation / go-to-definition).
# Fichier propre a la machine (chemins absolus) -> ignore par git.
db:
	@root=$$(pwd); \
	files=$$(ls src/*.c libft/*.c); \
	printf '[\n' > compile_commands.json; \
	first=1; \
	for f in $$files; do \
		if [ $$first -eq 0 ]; then printf ',\n' >> compile_commands.json; fi; \
		first=0; \
		printf '  {\n    "directory": "%s",\n    "command": "%s %s -c %s/%s",\n    "file": "%s/%s"\n  }' \
			"$$root" "$(CC) $(CFLAGS)" "$(INC)" "$$root" "$$f" "$$root" "$$f" >> compile_commands.json; \
	done; \
	printf '\n]\n' >> compile_commands.json; \
	echo "compile_commands.json genere ($$(echo $$files | wc -w) fichiers)"

.PHONY: all clean fclean re db
