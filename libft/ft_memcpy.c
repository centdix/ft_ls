/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcpy.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 12:53:50 by fgoulama          #+#    #+#             */
/*   Updated: 2019/10/15 18:03:05 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	size_t	i;
	char	*str1;
	char	*str2;

	if (dst == NULL && src == NULL)
		return (NULL);
	i = 0;
	str1 = (char *)src;
	str2 = (char *)dst;
	while (i < n)
	{
		str2[i] = str1[i];
		i++;
	}
	return (dst);
}
