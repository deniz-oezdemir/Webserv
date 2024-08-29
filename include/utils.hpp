/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/29 12:51:57 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/29 12:56:10 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <sstream>

namespace ft
{
	template <typename T>
	std::string	to_string(T const& value)
	{
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}
}
