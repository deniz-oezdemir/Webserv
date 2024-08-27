/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInput.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 14:35:22 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/27 22:47:31 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>

class ServerInput
{
  public:
	ServerInput();
	ServerInput(int argc, char** argv);
	ServerInput(ServerInput const& src);
	~ServerInput();

	ServerInput& operator=(ServerInput const& src);


	typedef enum e_serverFlags
	{
		NONE = 0x00,
		VERSION = 0x01,
		HELP = 0x02,
		TEST = 0x04,
		TEST_PRINT = 0x08,
	} t_serverFlags;

  private:
	int			 																_flags;
	static std::map<std::string, int> const 	_flagMap;
	std::string															_filepath;
	std::vector<std::string>								_configLines;

	void	_parseArg(std::string const &arg, int index, int argc);
	void	_setFlag(std::string const &flag);
	static std::map<std::string, int> const _createFlagMap(void);
};
