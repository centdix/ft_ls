/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 17:39:48 by fgoulama          #+#    #+#             */
/*   Updated: 2019/10/14 16:37:43 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcat(char *dst, const char *src, size_t dstsize)
{
	size_t i;
	size_t j;
	size_t len;

	i = 0;
	while (dst[i])
		i++;
	len = i;
	j = 0;
	while (i < (dstsize - 1) && src[j] && j < dstsize)
	{
		dst[i] = src[j];
		j++;
		i++;
	}
	dst[i] = '\0';
	if (len > dstsize)
		return (dstsize + ft_strlen(src));
	else
		return (len + ft_strlen(src));
}
