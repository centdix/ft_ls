/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printf.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/16 16:47:14 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/10 18:23:18 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PRINTF_H
# define PRINTF_H

# include "libft.h"
# include <stdarg.h>

typedef struct	s_param
{
	char	conv;
	int		flag_dot;
	int		flag_minus;
	int		flag_star_wd;
	int		flag_star_pr;
	int		flag_zero;
	int		width;
	int		prec;
}				t_param;

int				ft_printf(const char *c, ...);
void			ft_s_handler(char *str, t_param param, int *ret);
void			ft_int_handler(int nb, t_param param, int *ret);
void			ft_char_handler(int c, t_param param, int *ret);
void			ft_x_handler(unsigned int nb, t_param param, int *ret);
void			ft_p_handler(void *ptr, t_param param, int *ret);
void			ft_u_handler(int nb, t_param param, int *ret);
void			ft_fill_param(t_param *param, const char *str);
int				ft_isflag(char format);
int				ft_isconv(char format);
int				ft_myatoi(const char *str);
int				ft_hexa_converter(unsigned long int nb, char *base, int len);
void			init_param(t_param *param);

#endif
