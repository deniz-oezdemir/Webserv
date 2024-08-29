/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValue.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/29 15:56:02 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/29 16:09:18 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <vector>

class ConfigValue
{
  public:
	enum valueType
	{
		VECTOR,
		MAP
	};

	ConfigValue(void);
	ConfigValue(std::vector<std::string> const& value);
	ConfigValue(std::map<std::string, std::vector<std::string> > const& value);

	valueType						getType(void) const;
	std::vector<std::string> const& getVector(void) const;
	std::map<std::string, std::vector<std::string> > const& getMap(void) const;
	void	setVector(std::vector<std::string> const& value);
	void	setMap(std::map<std::string, std::vector<std::string> > const& value);

  private:

	valueType																					_type;
	std::vector<std::string>													_vectorValue;
	std::map<std::string, std::vector<std::string> >	_mapValue;
};
