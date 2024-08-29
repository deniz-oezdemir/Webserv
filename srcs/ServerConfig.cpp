#include "ServerConfig.hpp"
#include "ServerException.hpp"
#include "colors.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <iostream>
#include <stack>

std::array<std::string, 4> const ServerConfig::validLogLevels = {
	"debug",
	"info",
	"warn",
	"error",
};

ServerConfig::ServerConfig(std::string const &filepath)
	: filepath(filepath), _file(filepath)
{
	if (!this->_file.is_open())
		throw ServerException("Could not open the file [%]", errno, filepath);
	this->_initGeneralConfig();
}

ServerConfig::ServerConfig(ServerConfig const &src)
	: filepath(src.filepath), _file(src.filepath)
{
	if (!this->_file.is_open())
		throw ServerException("Could not open the file [%]", errno, filepath);
}

ServerConfig &ServerConfig::operator=(ServerConfig const &src)
{
	if (this != &src)
	{
		this->filepath = src.filepath;

		if (this->_file.is_open())
		{
			this->_file.close();
		}

		this->_file.clear();
		this->_file.open(this->filepath);

		if (!this->_file.is_open())
			throw ServerException(
				"Could not open the file [%]", errno, filepath
			);
	}
	return *this;
}

ServerConfig::~ServerConfig()
{
	if (this->_file.is_open())
		this->_file.close();
}

std::ifstream &ServerConfig::getFile(void)
{
	return this->_file;
}

void ServerConfig::_initGeneralConfig(void)
{
	this->_generalConfig["worker_processes"] = "";
	this->_generalConfig["worker_connections"] = "";
	this->_generalConfig["error_log"] = "info";
}

void ServerConfig::_initServersConfig(void)
{
	std::map<std::string, ConfigValue> server;
	server["server_name"] = ConfigValue();
	server["listen"] = ConfigValue();
	server["root"] = ConfigValue();
	server["index"] = ConfigValue();
	server["error_page"] = ConfigValue();
	this->_serversConfig.push_back(server);
}

void ServerConfig::parseFile(bool isTest, bool isTestPrint)
{
	(void)isTest;
	(void)isTestPrint;
	std::stack<bool>		 brackets;
	std::string				 line;
	std::vector<std::string> tmp;
	tmp.reserve(5);
	while (std::getline(this->_file, line))
	{
		ft::trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		ft::split(tmp, line);
		std::cout << tmp[0] << std::endl;
		if (tmp[0] == "worker_processes")
		{
			if (tmp.size() != 2 && (isTest || isTestPrint))
				std::cerr << PURPLE "[WebServer] " << YELLOW "(emerg)"
						  << WHITE " invalid number of arguments in "
								   "worker_processes directive" RESET
						  << std::endl;
			if (tmp[tmp.size() - 1][tmp[tmp.size() - 1].size() - 1] != ';')
				throw ServerException(
					"Invalid directive terminator [%]", 0, tmp[tmp.size() - 1]
				);
		}
		tmp.clear();
	}
}
