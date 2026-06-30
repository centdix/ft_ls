/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa_u.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 18:24:04 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/06 18:27:50 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int		find_len(unsigned int n)
{
	int						len;
	unsigned long int		nb;

	len = 1;
	nb = n;
	while (nb >= 10)
	{
		nb = nb / 10;
		len++;
	}
	return (len);
}

char			*ft_itoa_u(unsigned int n)
{
	int					len;
	int					stop;
	unsigned long int	nb;
	char				*result;

	len = find_len(n);
	stop = 0;
	nb = n;
	if (!(result = malloc(sizeof(char) * (len + 1))))
		return (0);
	result[len] = '\0';
	while (len > stop)
	{
		result[len - 1] = nb % 10 + '0';
		nb = nb / 10;
		len--;
	}
	return (result);
}
