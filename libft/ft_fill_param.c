/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_fill_param.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/09 15:53:55 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/10 19:25:14 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static void	ft_flags_finder(t_param *param, const char *str)
{
	while (*str && !ft_isconv(*str))
	{
		if (*str == '-')
		{
			param->flag_minus = 1;
			param->width = ft_myatoi(str + 1);
		}
		if (*str == '0' && !ft_isdigit(*(str - 1))
			&& !ft_isflag(*(str - 1)))
		{
			param->flag_zero = 1;
			param->width = ft_myatoi(str + 1);
		}
		if (*str == '*' && *(str - 1) == '.')
			param->flag_star_pr = 1;
		if (*str == '*' && *(str - 1) != '.')
			param->flag_star_wd = 1;
		if (*str == '.')
		{
			param->flag_dot = 1;
			param->prec = ft_myatoi(str + 1);
		}
		str++;
	}
}

static char	ft_conv_finder(const char *str)
{
	char	conv;

	conv = '\0';
	if (*(str + 1))
		str++;
	while (*str && !ft_isconv(*str))
		str++;
	if (ft_isconv(*str))
		conv = *str;
	return (conv);
}

void		ft_fill_param(t_param *param, const char *str)
{
	param->conv = ft_conv_finder(str);
	param->width = ft_myatoi(str + 1);
	ft_flags_finder(param, str + 1);
	if (param->prec < 0)
		param->prec = 0;
	if (param->width < 0)
		param->width *= -1;
}
