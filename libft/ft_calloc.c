/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   calloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 19:24:10 by fgoulama          #+#    #+#             */
/*   Updated: 2019/10/10 18:46:38 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t count, size_t size)
{
	void	*ptr;
	size_t	i;

	ptr = malloc(count * size);
	if (ptr == 0)
		return (ptr);
	i = 0;
	while (i < (count * size))
	{
		((char *)ptr)[i] = 0;
		i++;
	}
	return (ptr);
}
