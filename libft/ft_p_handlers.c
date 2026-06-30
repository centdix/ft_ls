/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_p_handlers.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 15:35:29 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/13 11:11:20 by fgoulama         ###   ########.fr       */
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

static void	ft_null_handler(t_param param)
{
	int i;

	if (!param.flag_minus && !param.flag_zero)
	{
		i = 0;
		while (i++ < (param.width - 2))
			ft_putchar_fd(' ', 1);
	}
	ft_putstr_fd("0x", 1);
	if (param.flag_zero && !param.flag_minus)
		while (i++ < (param.width - 2))
			ft_putchar_fd('0', 1);
	if (param.flag_minus)
	{
		i = 2;
		while (i++ < param.width)
			ft_putchar_fd(' ', 1);
	}
}

static void	ft_p_width(unsigned long int adr, t_param param,
						char *base, int len)
{
	int i;

	i = 0;
	if (!param.flag_minus && (!param.flag_zero || param.flag_dot))
	{
		i = (param.flag_dot && param.prec) ? 2 : 0;
		while (i++ < (param.width - len))
			ft_putchar_fd(' ', 1);
		ft_putstr_fd("0x", 1);
		ft_hexa_converter(adr, base, -1);
	}
	else if (param.flag_zero && !param.flag_minus)
	{
		ft_putstr_fd("0x", 1);
		while (i++ < (param.width - len))
			ft_putchar_fd('0', 1);
		ft_hexa_converter(adr, base, -1);
	}
	else if (param.flag_minus)
	{
		ft_putstr_fd("0x", 1);
		ft_hexa_converter(adr, base, -1);
		while (len++ < param.width)
			ft_putchar_fd(' ', 1);
	}
}

static void	ft_p_prec(unsigned long int adr, t_param param, char *base, int len)
{
	int i;

	if (param.width > len && !param.flag_minus)
	{
		i = param.prec + 2;
		while (i++ < param.width)
			ft_putchar_fd(' ', 1);
	}
	ft_putstr_fd("0x", 1);
	i = 0;
	while (i++ < (param.prec - len))
		ft_putchar_fd('0', 1);
	ft_hexa_converter(adr, base, -1);
	if (param.width > len && param.flag_minus)
	{
		i = param.prec + 2;
		while (i++ < param.width)
			ft_putchar_fd(' ', 1);
	}
}

void		ft_p_handler(void *ptr, t_param param, int *ret)
{
	int					len;
	int					total_len;
	char				base[17];
	unsigned long int	adr;

	adr = (unsigned long int)ptr;
	ft_strlcpy(base, "0123456789abcdef", 17);
	len = param.prec ? ft_hexa_converter(adr, base, 0)
	: ft_hexa_converter(adr, base, 0) + 2;
	total_len = ft_lenfinder(len, param);
	*ret += total_len;
	if (param.prec <= len && param.width > len)
	{
		(ptr == 0 && param.flag_dot && !param.prec) ? ft_null_handler(param)
		: ft_p_width(adr, param, base, len);
	}
	else if (param.prec > len)
		ft_p_prec(adr, param, base, len);
	else if (ptr == 0 && param.flag_dot && !param.prec)
		ft_putstr_fd("0x", 1);
	else
	{
		ft_putstr_fd("0x", 1);
		ft_hexa_converter(adr, base, -1);
	}
}
