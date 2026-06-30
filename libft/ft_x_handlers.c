/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_x_handlers.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:49:22 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/10 18:49:04 by fgoulama         ###   ########.fr       */
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

static void	ft_x_width(unsigned int nb, t_param param, char *base, int len)
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
		ft_hexa_converter(nb, base, -1);
	if (param.flag_minus)
		while (len++ < param.width)
			ft_putchar_fd(' ', 1);
}

static void	ft_x_prec(unsigned int nb, t_param param, char *base, int len)
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
	ft_hexa_converter(nb, base, -1);
	if (param.width > len && param.flag_minus)
	{
		i = param.prec;
		while (i++ < param.width)
			ft_putchar_fd(' ', 1);
	}
}

void		ft_x_handler(unsigned int nb, t_param param, int *ret)
{
	int		len;
	int		total_len;
	char	base[17];

	param.conv == 'x' ? ft_strlcpy(base, "0123456789abcdef", 17)
	: ft_strlcpy(base, "0123456789ABCDEF", 17);
	len = ft_hexa_converter(nb, base, 0);
	total_len = ft_lenfinder(len, param);
	*ret += total_len;
	if (param.prec <= len && param.width > len)
		ft_x_width(nb, param, base, len);
	else if (param.prec > len)
		ft_x_prec(nb, param, base, len);
	else if (nb == 0 && param.flag_dot && !param.prec)
		return ;
	else
		ft_hexa_converter(nb, base, -1);
}
