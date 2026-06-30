/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_id_handlers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/09 17:11:33 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/10 19:51:04 by fgoulama         ###   ########.fr       */
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

static void	ft_int_width(int nb, t_param param, int len)
{
	int i;
	int to_print;

	i = 0;
	to_print = (nb == 0 && param.prec == 0 && param.flag_dot) ? 0 : 1;
	if (!to_print)
		len--;
	len = (nb < 0 && param.flag_dot) ? len + 1 : len;
	if (!param.flag_minus && (!param.flag_zero || param.flag_dot))
		while (i++ < (param.width - len))
			ft_putchar_fd(' ', 1);
	if (nb < 0)
	{
		ft_putchar_fd('-', 1);
		nb = -nb;
	}
	if (param.flag_zero && !param.flag_minus)
		while (i++ < (param.width - len))
			ft_putchar_fd('0', 1);
	if (to_print)
		ft_putnbr_fd(nb, 1);
	if (param.flag_minus)
		while (len++ < param.width)
			ft_putchar_fd(' ', 1);
}

static void	ft_int_prec(int nb, t_param param, int len)
{
	int i;

	if (param.width > len && !param.flag_minus)
	{
		i = nb < 0 ? param.prec + 1 : param.prec;
		while (i++ < param.width)
			ft_putchar_fd(' ', 1);
	}
	if (nb < 0)
		ft_putchar_fd('-', 1);
	i = 0;
	while (i++ < (param.prec - len))
		ft_putchar_fd('0', 1);
	nb < 0 ? ft_putnbr_fd(-nb, 1) : ft_putnbr_fd(nb, 1);
	if (param.width > len && param.flag_minus)
	{
		i = nb < 0 ? param.prec + 1 : param.prec;
		while (i++ < param.width)
			ft_putchar_fd(' ', 1);
	}
}

void		ft_int_handler(int nb, t_param param, int *ret)
{
	int				len;
	int				total_len;
	char			*tmp;

	tmp = ft_itoa(nb);
	len = ft_strlen(tmp);
	total_len = ft_lenfinder(len, param);
	*ret += total_len;
	free(tmp);
	if (nb < 0 && param.prec >= len && param.width < param.prec)
		*ret += 1;
	if (nb < 0 && param.flag_dot)
		len--;
	if (param.prec <= len && param.width > (int)len)
		ft_int_width(nb, param, len);
	else if (param.prec > (int)len)
		ft_int_prec(nb, param, len);
	else if (nb == 0 && param.flag_dot && !param.prec)
		return ;
	else
		ft_putnbr_fd(nb, 1);
}
