/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa_base.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/22 18:49:07 by fgoulama          #+#    #+#             */
/*   Updated: 2019/11/22 18:57:48 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_itoa_base(unsigned int nb, char *base)
{
	char	*result;
	int		size;
	int		tmp;
	int		i;
	int		len;

	size = 1;
	tmp = nb;
	len = ft_strlen(base);
	while (tmp >= len)
	{
		tmp = tmp / len;
		size++;
	}
	if (!(result = malloc(sizeof(char) * (size + 1))))
		return (NULL);
	result[size] = '\0';
	i = size - 1;
	while (i >= 0)
	{
		result[i] = base[nb % len];
		nb = nb / len;
		i--;
	}
	return (result);
}
