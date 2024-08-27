/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInput.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 16:05:45 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/27 22:53:34 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerInput.hpp"

std::map<std::string, int> const ServerInput::_flagMap =
	ServerInput::_createFlagMap();

std::map<std::string, int> const ServerInput::_createFlagMap()
{
	std::map<std::string, int> m;
	m["--version"] = VERSION;
	m["-v"] = VERSION;
	m["-V"] = VERSION;
	m["--help"] = HELP;
	m["-h"] = HELP;
	m["-?"] = HELP;
	m["--test"] = TEST;
	m["-t"] = TEST;
	m["--Test"] = TEST_PRINT;
	m["-T"] = TEST_PRINT;
	return m;
}

ServerInput::ServerInput() : _flags(NONE), _filepath("./default.conf"){};

ServerInput::ServerInput(int argc, char** argv)
	: _flags(NONE), _filepath("./default.conf")
{
	if (argc > 3)
		throw std::runtime_error("Too many arguments.\nUsage: ./webserv "
								 "[OPTIONAL: -flag][OPTIONAL: config_file]");
	for (int i = 1; i < argc; ++i)
		this->_parseArg(argv[i], i, argc);
};

void ServerInput::_parseArg(std::string const& arg, int index, int argc)
{
	if (arg[0] == '-')
		this->_setFlag(arg);
	else if (index == argc - 1)
		this->_filepath = arg;
	else
		throw std::runtime_error(
			"Invalid argument-> " + arg +
			"\nUsage: ./webserv [OPTIONAL: -flag][OPTIONAL: config_file]"
		);
};

void ServerInput::_setFlag(std::string const& flag)
{
	std::map<std::string, int>::const_iterator it = this->_flagMap.find(flag);
	if (it != this->_flagMap.end())
		this->_flags |= it->second;
	else
		throw std::runtime_error(
			"Invalid flag -> " + flag +
			"\nUsage: ./webserv [OPTIONAL: -flag][OPTIONAL: config_file]"
		);
};
