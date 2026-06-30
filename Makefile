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
SRCS		= $(SRC_DIR)/main.c
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

.PHONY: all clean fclean re
