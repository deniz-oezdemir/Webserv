/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInput.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 16:05:45 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/28 13:38:39 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerInput.hpp"
#include "colors.hpp"
#include <sstream>

std::map<std::string, int> const ServerInput::_flagMap =
	ServerInput::_createFlagMap();

std::map<std::string, int> const ServerInput::_createFlagMap()
{
	std::map<std::string, int> m;
	m["--version"] = V_LITE;
	m["-v"] = V_LITE;
	m["--Version"] = V_FULL;
	m["-V"] = V_FULL;
	m["--help"] = HELP;
	m["-h"] = HELP;
	m["-H"] = HELP;
	m["-?"] = HELP;
	m["--test"] = TEST;
	m["-t"] = TEST;
	m["--Test"] = TEST_PRINT;
	m["-T"] = TEST_PRINT;
	return m;
}

ServerInput::ServerInput() : _flags(this->NONE), _filepath("./default.conf"){};

ServerInput::ServerInput(int argc, char** argv)
	: _flags(this->NONE), _filepath("./default.conf")
{
	for (int i = 1; i < argc; ++i)
		this->_parseArg(argv[i], i, argc);
};

ServerInput::ServerInput(ServerInput const& src)
{
	*this = src;
};

ServerInput& ServerInput::operator=(ServerInput const& src)
{
	if (this != &src)
	{
		this->_flags = src._flags;
		this->_filepath = src._filepath;
	}
	return *this;
};

ServerInput::~ServerInput(){};

void ServerInput::_parseArg(std::string const& arg, int index, int argc)
{
	if (arg[0] == '-')
		this->_setFlag(arg);
	else if (index == argc - 1)
		this->_filepath = arg;
	else
		throw std::runtime_error(
			"Invalid argument=> " + arg +
			"\nUsage:\t./webserv [OPTIONAL: -flag][OPTIONAL: config_file]"
		);
};

void ServerInput::_setFlag(std::string const& flag)
{
	std::map<std::string, int>::const_iterator it = this->_flagMap.find(flag);
	if (it != this->_flagMap.end())
		this->_flags |= it->second;
	else
		throw std::runtime_error(
			"Invalid flag=> " + flag +
			"\nUsage:\t./webserv [OPTIONAL: -flag][OPTIONAL: config_file]"
		);
};

bool ServerInput::hasThisFlag(t_serverFlags flag) const
{
	return (this->_flags & flag);
};

std::string ServerInput::getHelpMessage(void) const
{
	return YELLOW BOLD "Usage:\n" CYAN "\t./webserv " WHITE BOLD
					   "[OPTIONAL: -flag][OPTIONAL: config_file]\n" YELLOW BOLD
					   "Flags:\n" CYAN BOLD "\t-h, -H, -?" RESET WHITE
					   "\t\tDisplay this help message\n" CYAN BOLD
					   "\t-v, --version" RESET WHITE
					   "\t\tDisplay version information\n" CYAN BOLD
					   "\t-V, --Version" RESET WHITE
					   "\t\tDisplay full version information\n" CYAN BOLD
					   "\t-t, --test" RESET WHITE
					   "\t\tCheck the configuration file\n" CYAN BOLD
					   "\t-T, --Test" RESET WHITE
					   "\t\tCheck the configutation file and print it\n" RESET;
}
#include <iostream>

std::string ServerInput::getVersionMessage(void) const
{
	std::stringstream ss;

	ss << YELLOW BOLD "WebServ " << RESET ULINE CYAN "v0.0.1" << "\n" RESET;
	if (this->hasThisFlag(ServerInput::V_LITE))
		return ss.str();

#ifdef __clang__
	ss << WHITE "Compiled with" << YELLOW " Clang " << WHITE "version " CYAN BOLD
	   << __clang_major__ << "." << __clang_minor__ << "."
	   << __clang_patchlevel__ << RESET;
#elif defined(__GNUC__)
	ss << WHITE "Compiled with << YELLOW " GCC " WHITE << " version
		  " CYAN ULINE << __GNUC__ << "." << __GNUC_MINOR__
	   << "." << __GNUC_PATCHLEVEL__ << RESET;
#elif defined(_MSC_VER)
	ss << WHITE "Compiled with " << YELLOW "MSVC " << WHITE "version " CYAN BOLD
	   << _MSC_VER << RESET;
#else
	ss << RED "Unknown compiler" << RESET;
#endif

	ss << WHITE "\nConfiguration file path: " YELLOW << this->_filepath << '\n';
	ss << WHITE "Created by " << CYAN BOLD "[johnavar] " << "[jmigoya] "
	   << "[denizozd] " << RESET;

	return ss.str();
}
