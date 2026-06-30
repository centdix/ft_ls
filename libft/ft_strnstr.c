/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 18:08:23 by fgoulama          #+#    #+#             */
/*   Updated: 2019/10/14 17:12:03 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *haystack, const char *needle, size_t len)
{
	char *save;
	char *found;

	save = (char*)needle;
	if (!*needle)
		return ((char *)haystack);
	while (*haystack && len > 0)
	{
		if (*haystack == *needle)
		{
			found = (char *)haystack;
			while (*haystack == *needle && *needle && *haystack)
			{
				haystack++;
				needle++;
			}
			if (!*needle && len >= ft_strlen(save))
				return (found);
			haystack = found;
			needle = save;
		}
		haystack++;
		len--;
	}
	return (0);
}
