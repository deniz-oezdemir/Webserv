/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInput.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 14:35:22 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/28 19:22:13 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>

class ServerInput
{
  public:

	typedef enum e_serverFlags
	{
		NONE = 0x00,
		V_LITE = 0x01,
		V_FULL = 0x02,
		HELP = 0x04,
		TEST = 0x08,
		TEST_PRINT = 0x10,
	} t_serverFlags;

	ServerInput();
	ServerInput(int argc, char** argv);
	ServerInput(ServerInput const& src);
	~ServerInput();

	ServerInput&	operator=(ServerInput const& src);

	bool					hasThisFlag(t_serverFlags flag) const;
	std::string		getHelpMessage(void) const;
	std::string		getVersionMessage(void) const;
	std::string		getFilePath(void) const;

  private:
	int			 																_flags;
	static std::map<std::string, int> const	_flagMap;
	std::string															_filepath;

	void					_parseArg(std::string const &arg, int index, int argc);
	void					_setFlag(std::string const &flag);
	static std::map<std::string, int> const _createFlagMap(void);
};
