/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_c_handlers.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/09 17:11:19 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/10 19:48:04 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static void	ft_char_width(int c, t_param param)
{
	int i;

	i = 0;
	if (param.flag_zero && !param.flag_minus)
	{
		while (i++ < (param.width - 1))
			ft_putchar_fd('0', 1);
		ft_putchar_fd(c, 1);
	}
	else if (param.flag_minus)
	{
		ft_putchar_fd(c, 1);
		while (i++ < (param.width - 1))
			ft_putchar_fd(' ', 1);
	}
	else
	{
		while (i++ < (param.width - 1))
			ft_putchar_fd(' ', 1);
		ft_putchar_fd(c, 1);
	}
}

void		ft_char_handler(int c, t_param param, int *ret)
{
	int		total_len;

	total_len = param.width ? param.width : 1;
	*ret += total_len;
	if (param.width > 0)
		ft_char_width(c, param);
	else
		ft_putchar_fd(c, 1);
}
