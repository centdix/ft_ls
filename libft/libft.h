/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/13 17:53:00 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/22 19:12:51 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIBFT_H
# define LIBFT_H

# include <unistd.h>
# include <stdlib.h>

typedef unsigned int	t_size;

typedef	struct			s_list
{
	void			*content;
	struct s_list	*next;
}						t_list;

size_t					ft_strlen(const char *s);
int						ft_atoi(const char *str);
int						ft_atoi_base(char *str, char *base);
void					ft_bzero(void *s, size_t n);
void					*ft_calloc(size_t count, size_t size);
int						ft_isalnum(int c);
int						ft_isalpha(int c);
int						ft_isascii(int c);
int						ft_isdigit(int c);
int						ft_isprint(int c);
int						ft_power(int nb, int power);
char					*ft_itoa(int n);
char					*ft_itoa_u(unsigned int n);
char					*ft_itoa_base(unsigned int nb, char *base);
void					*ft_memccpy
						(void *dst, const void *src, int c, size_t n);
void					*ft_memchr(const void *s, int c, size_t n);
int						ft_memcmp(const void *s1, const void *s2, size_t n);
void					*ft_memcpy(void *dst, const void *src, size_t n);
void					*ft_memmove(void *dst, const void *src, size_t len);
void					*ft_memset(void *b, int c, size_t len);
void					ft_putchar_fd(char c, int fd);
void					ft_putchar(char c);
void					ft_putstr(char *str);
void					ft_putendl_fd(char *s, int fd);
void					ft_putnbr_fd(int n, int fd);
void					ft_putnbr_u_fd(unsigned int n, int fd);
void					ft_putstr_fd(char *s, int fd);
char					*ft_get_eol(char *str);
char					**ft_split(char const *s, char c);
char					*ft_strchr(const char *s, int c);
char					*ft_strdup(const char *s1);
char					*ft_strjoin(char const *s1, char const *s2);
size_t					ft_strlcat(char *dst, const char *src, size_t dstsize);
size_t					ft_strlcpy
						(char *dst, const char *src, size_t dstsize);
char					*ft_strmapi
						(char const *s, char (*f)(unsigned int, char));
int						ft_strncmp(const char *s1, const char *s2, size_t n);
char					*ft_strnstr
						(const char *haystack, const char *needle, size_t len);
char					*ft_strrchr(const char *s, int c);
char					*ft_strtrim(char const *s1, char const *set);
char					*ft_substr
						(char const *s, unsigned int start, size_t len);
int						ft_tolower(int c);
int						ft_toupper(int c);
t_list					*ft_lstnew(void *content);
void					ft_lstadd_front(t_list **alst, t_list *new);
int						ft_lstsize(t_list *lst);
t_list					*ft_lstlast(t_list *lst);
void					ft_lstadd_back(t_list **alst, t_list *new);
void					ft_lstclear(t_list **lst, void (*del)(void *));
void					ft_lstdelone(t_list *lst, void (*del)(void *));
void					ft_lstiter(t_list *lst, void (*f)(void *));
t_list					*ft_lstmap(t_list *lst, void *(*f)(void *),
						void (*del)(void *));

#endif
