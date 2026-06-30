/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/04 13:25:08 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/10 18:38:13 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

int			ft_isflag(char format)
{
	if (format != '-' && format != '0' && format != '*' && format != '.')
		return (0);
	return (1);
}

int			ft_isconv(char format)
{
	if (format != 'c' && format != 'd' && format != 'u'
	&& format != 'i' && format != 'p' && format != 'x'
	&& format != 's' && format != 'X' && format != '%')
		return (0);
	return (1);
}

int			ft_myatoi(const char *str)
{
	int nb;
	int signe;

	nb = 0;
	signe = 1;
	if (*str == '-' || *str == '+')
	{
		if (*str == '-')
			signe = -1;
		str++;
	}
	while (*str == '-')
		str++;
	while (ft_isdigit(*str) && *str)
	{
		nb = nb * 10 + (*str - '0');
		str++;
	}
	return (nb * signe);
}

int			ft_hexa_converter(unsigned long int nb, char *base, int len)
{
	char	x;

	if (nb >= 16)
	{
		if (len != -1)
			len = ft_hexa_converter(nb / 16, base, len);
		else
			ft_hexa_converter(nb / 16, base, len);
	}
	x = base[nb % 16];
	if (len == -1)
		ft_putchar_fd(x, 1);
	len++;
	return (len);
}

void		init_param(t_param *param)
{
	param->conv = '\0';
	param->flag_dot = 0;
	param->flag_minus = 0;
	param->flag_star_wd = 0;
	param->flag_star_pr = 0;
	param->flag_zero = 0;
	param->width = 0;
	param->prec = 0;
}
