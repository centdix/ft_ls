/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/15 11:49:14 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/10 19:40:03 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static void		ft_dispatcher(t_param *param, va_list list, int *ret)
{
	if (param->flag_star_wd)
	{
		param->width = va_arg(list, int);
		if (param->width < 0)
		{
			param->flag_minus = 1;
			param->width *= -1;
		}
	}
	if (param->flag_star_pr)
		param->prec = va_arg(list, int);
	if (param->conv == 's')
		return (ft_s_handler(va_arg(list, char *), *param, ret));
	if (param->conv == 'i' || param->conv == 'd')
		return (ft_int_handler(va_arg(list, int), *param, ret));
	if (param->conv == 'c')
		return (ft_char_handler(va_arg(list, int), *param, ret));
	if (param->conv == 'x' || param->conv == 'X')
		return (ft_x_handler(va_arg(list, unsigned int), *param, ret));
	if (param->conv == 'p')
		return (ft_p_handler(va_arg(list, void *), *param, ret));
	if (param->conv == 'u')
		return (ft_u_handler(va_arg(list, unsigned int), *param, ret));
	if (param->conv == '%')
		return (ft_char_handler('%', *param, ret));
}

static void		format_handler(const char *format, t_param *param,
				va_list list, int *ret)
{
	while (*format)
	{
		if (*format == '%')
		{
			init_param(param);
			ft_fill_param(param, format);
			ft_dispatcher(param, list, ret);
			if (*(format + 1))
				format++;
			while (!ft_isconv(*format))
				format++;
			if (*(format + 1))
				format++;
			else
				break ;
		}
		else
		{
			ft_putchar_fd(*format, 1);
			format++;
			*ret = *ret + 1;
		}
	}
}

int				ft_printf(const char *format, ...)
{
	va_list		list;
	t_param		param;
	int			ret;

	ret = 0;
	va_start(list, format);
	format_handler(format, &param, list, &ret);
	va_end(list);
	return (ret);
}
