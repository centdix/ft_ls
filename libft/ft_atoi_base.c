/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi_base.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/22 15:25:25 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/22 17:06:03 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	check_error(char *base)
{
	int i;
	int j;

	if (ft_strlen(base) == 1)
		return (1);
	i = 0;
	while (base[i])
	{
		if (base[i] == '+' || base[i] == '-' || base[i] == ' ')
			return (1);
		i++;
	}
	i = 0;
	while (base[i])
	{
		j = 0;
		while (j < i)
		{
			if (base[i] == base[j])
				return (1);
			j++;
		}
		i++;
	}
	return (0);
}

static int	ft_isbase(char c, char *base)
{
	while (*base)
	{
		if (c == *base)
			return (1);
		base++;
	}
	return (0);
}

static int	find_index(char c, char *base)
{
	int i;

	i = 0;
	while (base[i])
	{
		if (c == base[i])
			return (i);
		i++;
	}
	return (-1);
}

int			ft_atoi_base(char *str, char *base)
{
	int		nb;
	int		signe;
	int		len;
	int		i;
	int		index;

	nb = 0;
	signe = 1;
	if (check_error(base) || base == NULL)
		return (0);
	len = ft_strlen(base);
	if (*str == '-' || *str == '+')
	{
		if (*str == '-')
			signe = -1;
		str++;
	}
	i = 0;
	while (ft_isbase(str[i], base) && str[i])
	{
		index = find_index(str[i], base);
		nb += index * ft_power(len, (ft_strlen(str) - 1) - i);
		i++;
	}
	return (nb * signe);
}
