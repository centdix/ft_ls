/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/09 20:04:23 by fgoulama          #+#    #+#             */
/*   Updated: 2019/10/17 17:43:28 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int		find_len(int n)
{
	int			len;
	long int	nb;

	len = 1;
	nb = n;
	if (nb < 0)
	{
		len++;
		nb = nb * (-1);
	}
	while (nb >= 10)
	{
		nb = nb / 10;
		len++;
	}
	return (len);
}

char			*ft_itoa(int n)
{
	int			len;
	int			stop;
	long int	nb;
	char		*result;

	len = find_len(n);
	stop = 0;
	nb = n;
	if (!(result = malloc(sizeof(char) * (len + 1))))
		return (0);
	if (nb < 0)
	{
		nb = nb * (-1);
		*result = '-';
		stop = 1;
	}
	result[len] = '\0';
	while (len > stop)
	{
		result[len - 1] = nb % 10 + '0';
		nb = nb / 10;
		len--;
	}
	return (result);
}
