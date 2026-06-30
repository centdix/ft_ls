/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strncmp.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 16:47:38 by fgoulama          #+#    #+#             */
/*   Updated: 2019/10/17 14:51:26 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int		ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t max_size;
	size_t i;

	if (!s1 && !s2)
		return (0);
	if (ft_strlen(s1) <= ft_strlen(s2))
		max_size = ft_strlen(s2);
	else
		max_size = ft_strlen(s1);
	i = 0;
	while (i <= max_size && i < n)
	{
		if (s1[i] != s2[i])
			return ((unsigned char)s1[i] - (unsigned char)s2[i]);
		i++;
	}
	return (0);
}
