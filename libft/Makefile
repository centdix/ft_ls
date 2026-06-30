HEADER = libft

BONUS_SRCS = $(wildcard *.c)

SRCS = $(filter-out $(wildcard *bonus.c), $(wildcard *.c))

OBJS = ${SRCS:.c=.o}

BONUS_OBJS = ${BONUS_SRCS:.c=.o}

NAME = libft.a

CFLAGS = -Wall -Wextra -Werror

CC = gcc

all: ${NAME}

$(NAME): ${OBJS}
	ar -rcs ${NAME} ${OBJS}

bonus: ${BONUS_OBJS}
	ar -rcs ${NAME} ${BONUS_OBJS}

%.o : %.c 
	${CC} ${CFLAGS} -I ${HEADER} -c $< -o ${<:.c=.o}

clean:
	/bin/rm -f ${BONUS_OBJS}

fclean: clean
	/bin/rm -f ${NAME}

re: fclean all

.PHONY: clean all
