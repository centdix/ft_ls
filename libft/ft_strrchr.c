/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 16:25:47 by fgoulama          #+#    #+#             */
/*   Updated: 2019/10/17 13:45:56 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strrchr(const char *s, int c)
{
	char	*ptr;
	int		i;

	i = 0;
	ptr = 0;
	if (!*s && c == '\0')
		ptr = (char *)s;
	else if (!*s)
		return (NULL);
	while (s[i])
	{
		if (s[i] == (char)c)
			ptr = (char *)&s[i];
		if (c == '\0' && s[i + 1] == c)
			ptr = (char *)&s[i + 1];
		i++;
	}
	return (ptr);
}
