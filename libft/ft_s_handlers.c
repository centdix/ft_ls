/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_s_handlers.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:34:17 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/13 09:54:03 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static int	ft_lenfinder(int len, t_param param)
{
	int		result;
	int		length;

	length = (param.prec < len && param.flag_dot) ? param.prec : len;
	result = length;
	if (param.width > length)
		result += (param.width - length);
	return (result);
}

static void	ft_s_width(char *str, t_param param)
{
	int len;
	int i;

	len = ft_strlen(str);
	i = 0;
	if (param.flag_zero && !param.flag_minus)
	{
		while (i++ < (param.width - len))
			ft_putchar_fd('0', 1);
		ft_putstr_fd(str, 1);
	}
	else if (param.flag_minus)
	{
		ft_putstr_fd(str, 1);
		while (i++ < (param.width - len))
			ft_putchar_fd(' ', 1);
	}
	else
	{
		while (i++ < (param.width - len))
			ft_putchar_fd(' ', 1);
		ft_putstr_fd(str, 1);
	}
}

static void	ft_s_prec(char *str, t_param param)
{
	int i;
	int len;

	i = 0;
	len = param.width - param.prec;
	if (param.flag_zero && !param.flag_minus)
	{
		while (len-- > 0)
			ft_putchar_fd('0', 1);
	}
	else if (param.width > 0 && !param.flag_minus)
	{
		while (len-- > 0)
			ft_putchar_fd(' ', 1);
	}
	while (i < param.prec)
		ft_putchar_fd(str[i++], 1);
	if (param.flag_minus)
	{
		len = param.width - param.prec;
		while (len-- > 0)
			ft_putchar_fd(' ', 1);
	}
}

void		ft_s_handler(char *str, t_param param, int *ret)
{
	int		len;
	int		total_len;
	char	null[7];

	ft_strlcpy(null, "(null)", 7);
	str = str ? str : null;
	len = ft_strlen(str);
	total_len = ft_lenfinder(len, param);
	*ret += total_len;
	if (param.width > len && (param.prec >= len || !param.flag_dot))
		ft_s_width(str, param);
	else if (param.flag_dot && param.prec < (int)ft_strlen(str))
		ft_s_prec(str, param);
	else
		ft_putstr_fd(str, 1);
}
