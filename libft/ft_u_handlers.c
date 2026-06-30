/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_u_handlers.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 17:36:15 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/10 18:48:07 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static int	ft_lenfinder(int len, t_param param)
{
	int		result;

	result = len;
	if (param.prec > len && param.width < len)
		result += (param.prec - len);
	else if (param.width >= len && param.prec > len && param.prec < param.width)
		result += (param.width - len);
	else if (param.width >= len && param.prec > len
	&& param.prec >= param.width)
		result += (param.prec - len);
	else if (param.width > len && param.prec <= len)
		result += (param.width - len);
	return (result);
}

static void	ft_u_width(unsigned int nb, t_param param, int len)
{
	int i;
	int to_print;

	i = 0;
	to_print = (nb == 0 && param.prec == 0 && param.flag_dot) ? 0 : 1;
	if (!to_print)
		len--;
	if (!param.flag_minus && (!param.flag_zero || param.flag_dot))
		while (i++ < (param.width - len))
			ft_putchar_fd(' ', 1);
	if (param.flag_zero && !param.flag_minus)
		while (i++ < (param.width - len))
			ft_putchar_fd('0', 1);
	if (to_print)
		ft_putnbr_u_fd(nb, 1);
	if (param.flag_minus)
		while (len++ < param.width)
			ft_putchar_fd(' ', 1);
}

static void	ft_u_prec(unsigned int nb, t_param param, int len)
{
	int i;

	if (param.width > len && !param.flag_minus)
	{
		i = param.prec;
		while (i++ < param.width)
			ft_putchar_fd(' ', 1);
	}
	i = 0;
	while (i++ < (param.prec - len))
		ft_putchar_fd('0', 1);
	ft_putnbr_u_fd(nb, 1);
	if (param.width > len && param.flag_minus)
	{
		i = param.prec;
		while (i++ < param.width)
			ft_putchar_fd(' ', 1);
	}
}

void		ft_u_handler(int nb, t_param param, int *ret)
{
	int				len;
	int				total_len;
	char			*tmp;

	tmp = ft_itoa_u(nb);
	len = ft_strlen(tmp);
	total_len = ft_lenfinder(len, param);
	*ret += total_len;
	free(tmp);
	if (param.prec <= len && param.width > (int)len)
		ft_u_width(nb, param, len);
	else if (param.prec > (int)len)
		ft_u_prec(nb, param, len);
	else if (nb == 0 && param.flag_dot && !param.prec)
		return ;
	else
		ft_putnbr_u_fd(nb, 1);
}
