/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 19:01:26 by fgoulama          #+#    #+#             */
/*   Updated: 2019/10/17 17:45:52 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_atoi(const char *str)
{
	int nb;
	int signe;

	nb = 0;
	signe = 1;
	while (*str == '\t' || *str == '\n' || *str == '\v' || *str == '\f'
	|| *str == '\r' || *str == ' ')
		str++;
	if (*str == '-' || *str == '+')
	{
		if (*str == '-')
			signe = -1;
		str++;
	}
	while (ft_isdigit(*str) && *str)
	{
		nb = nb * 10 + (*str - '0');
		str++;
	}
	return (nb * signe);
}
