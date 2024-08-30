#include "ServerConfig.hpp"
#include "ServerException.hpp"
#include "colors.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <iostream>
#include <stack>
#include <string>

std::array<std::string, 4> const ServerConfig::validLogLevels = {
	"debug",
	"info",
	"warn",
	"error",
};

ServerConfig::ServerConfig(std::string const &filepath)
	: filepath(filepath), _file(filepath), _isConfigOK(true)
{
	if (!this->_file.is_open())
		throw ServerException("Could not open the file [%]", errno, filepath);
	this->_initGeneralConfig();
}

ServerConfig::ServerConfig(ServerConfig const &src)
	: filepath(src.filepath), _file(src.filepath), _isConfigOK(true)
{
	if (!this->_file.is_open())
		throw ServerException("Could not open the file [%]", errno, filepath);
}

ServerConfig &ServerConfig::operator=(ServerConfig const &src)
{
	if (this != &src)
	{
		this->filepath = src.filepath;
		this->_isConfigOK = src.getIsConfigOK();

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

bool ServerConfig::getIsConfigOK(void) const
{
	return this->_isConfigOK;
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
	this->_serversConfig.push_back(server);
}

void ServerConfig::_errorHandler(
	std::string const &message,
	unsigned int	   lineIndex,
	bool			   isTest,
	bool			   isTestPrint
)
{
	if (isTest || isTestPrint)
	{
		std::cerr << PURPLE "<WebServ> " << YELLOW "[emerg] " RESET << message
				  << " in the configuration file " CYAN << this->filepath << ":"
				  << lineIndex << RESET << std::endl;
		this->_isConfigOK = false;
	}
	else
		throw ServerException(
			message + " in the configuration file " + this->filepath + ":" +
			ft::toString(lineIndex)
		);
}

bool ServerConfig::_checkValues(
	std::vector<std::string> const &tokens,
	unsigned int					maxSize,
	unsigned int					lineIndex,
	bool							isTest,
	bool							isTestPrint
)
{
	if (tokens.size() != maxSize)
	{
		this->_errorHandler(
			"Invalid number of arguments in [" + tokens[0] + "] directive",
			lineIndex,
			isTest,
			isTestPrint
		);
		return false;
	}
	if (tokens[tokens.size() - 1][tokens[tokens.size() - 1].size() - 1] != ';')
	{
		this->_errorHandler(
			"Missing ';' in [" + tokens[0] + "] directive",
			lineIndex,
			isTest,
			isTestPrint
		);
		return false;
	}
	return true;
}

void ServerConfig::parseFile(bool isTest, bool isTestPrint)
{
	std::stack<bool>		 brackets;
	std::string				 line;
	unsigned int			 lineIndex(1);
	std::vector<std::string> tokens;
	tokens.reserve(5);
	while (std::getline(this->_file, line))
	{
		ft::trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		ft::split(tokens, line);
		if (tokens[0] == "}")
		{
			if (!brackets.empty())
				brackets.pop();
			else
				this->_errorHandler(
					"Missing '{' to pair '}'", lineIndex, isTest, isTestPrint
				);
		}
		else if (tokens[0] == "worker_processes" ||
				 tokens[0] == "worker_connections")
		{
			if (this->_checkValues(tokens, 2, lineIndex, isTest, isTestPrint))
			{
				tokens[1].erase(tokens[1].size() - 1);
				if (ft::isStrOfDigits(tokens[1]) || tokens[1] == "auto")
					this->_generalConfig[tokens[0]] = tokens[1];
				else
					this->_errorHandler(
						"Invalid value [" + tokens[1] + "] for " + tokens[0] +
							" directive",
						lineIndex,
						isTest,
						isTestPrint
					);
			}
		}
		else if (tokens[0] == "error_log")
		{
			if (this->_checkValues(tokens, 2, lineIndex, isTest, isTestPrint))
			{
				tokens[1].erase(tokens[1].size() - 1);
				if (std::find(
						this->validLogLevels.begin(),
						this->validLogLevels.end(),
						tokens[1]
					) != this->validLogLevels.end())
					this->_generalConfig[tokens[0]] = tokens[1];
				else
					this->_errorHandler(
						"Invalid value [" + tokens[1] + "] for " + tokens[0] +
							" directive",
						lineIndex,
						isTest,
						isTestPrint
					);
			}
		}
		else if (tokens[0] == "events")
		{
			if (tokens.size() == 2 && tokens[1] == "{")
				brackets.push(true);
			else
				this->_errorHandler(
					"Missing '{' in [" + tokens[0] + "] directive",
					lineIndex,
					isTest,
					isTest
				);
		}
		else if (tokens[0] == "http" || tokens[0] == "server")
		{
			if (tokens.size() == 2 && tokens[1] == "{")
				brackets.push(true);
			else
				this->_errorHandler(
					"Missing '{' in [" + tokens[0] + "] directive",
					lineIndex,
					isTest,
					isTestPrint
				);
			if (tokens[0] == "server")
				this->_initServersConfig();
		}
		else if (tokens[0] == "server_name" || tokens[0] == "index")
		{
			if (tokens.size() > 1 &&
				this->_checkValues(tokens, 99, lineIndex, isTest, isTestPrint))
			{
				tokens[tokens.size() - 1].erase(
					tokens[tokens.size() - 1].size() - 1
				);
				this->_serversConfig.back()[tokens[0]] = ConfigValue(
					std::vector<std::string>(tokens.begin() + 1, tokens.end())
				);
			}
			else
				this->_errorHandler(
					"Invalid number of arguments in [" + tokens[0] +
						"] directive, expected at least 1",
					lineIndex,
					isTest,
					isTestPrint
				);
		}
		else if (tokens[0] == "listen" || tokens[0] == "root" ||
				 tokens[0] == "client_max_body_size")
		{
			if (this->_checkValues(tokens, 2, lineIndex, isTest, isTestPrint))
			{
				tokens[1].erase(tokens[1].size() - 1);
				if ((tokens[0] == "listen" ||
					 tokens[0] == "client_max_body_size") &&
					!ft::isStrOfDigits(tokens[1]))
					this->_errorHandler(
						"Invalid value [" + tokens[1] + "] for " + tokens[0] +
							" directive, expected a port number",
						lineIndex,
						isTest,
						isTestPrint
					);
				else
					this->_serversConfig.back()[tokens[0]] =
						ConfigValue(std::vector<std::string>(
							tokens.begin() + 1, tokens.end()
						));
			}
		}
		else if (tokens[0] == "error_page")
		{
			if (tokens.size() > 2 &&
				this->_checkValues(tokens, 99, lineIndex, isTest, isTestPrint))
			{
				tokens[tokens.size() - 1].erase(
					tokens[tokens.size() - 1].size() - 1
				);
				std::vector<std::string>::iterator it(tokens.begin() + 1);
				for (; it < tokens.end() - 1; ++it)
				{
					if (!ft::isStrOfDigits(*it))
						this->_errorHandler(
							"Invalid value [" + *it + "] for " + tokens[0] +
								" directive, expected a status code",
							lineIndex,
							isTest,
							isTestPrint
						);
					else
						this->_serversConfig.back()[*it] =
							ConfigValue(std::vector<std::string>(
								tokens.end() - 1, tokens.end()
							));
				}
			}
			else
				this->_errorHandler(
					"Invalid number of arguments in [" + tokens[0] +
						"] directive, expected at leat 2",
					lineIndex,
					isTest,
					isTestPrint
				);
		}
		else if (tokens[0] == "location")
		{
			if (tokens.size() > 2 && tokens.size() < 5 &&
				tokens[tokens.size() - 1] == "{")
			{
				if (tokens.size() == 4 &&
					(tokens[2] != "~" && tokens[2] != "=" &&
					 tokens[2] != "^~" && tokens[2] != "~*"))
					this->_errorHandler(
						"Invalid value [" + tokens[2] + "] for " + tokens[0] +
							" directive, expected '~' or '='",
						lineIndex,
						isTest,
						isTestPrint
					);
				else
				{
					std::string tmp(tokens[1] + " " + tokens[2]);
					std::map<std::string, std::vector<std::string> > location;
					tokens.clear();
					++lineIndex;
					while (std::getline(this->_file, line))
					{
						ft::trim(line);
						if (line.empty() || line[0] == '#')
							continue;
						ft::split(tokens, line);
						if (tokens[0] == "}")
							break;
						if (this->_checkValues(
								tokens, 99, lineIndex, isTest, isTestPrint
							))
						{
							tokens[tokens.size() - 1].erase(
								tokens[tokens.size() - 1].size() - 1
							);
							location[tokens[0]] = std::vector<std::string>(
								tokens.begin() + 1, tokens.end()
							);
						}
						tokens.clear();
						++lineIndex;
					}
					if (location.empty())
						this->_errorHandler(
							"Empty location directive",
							lineIndex,
							isTest,
							isTestPrint
						);
					else
						this->_serversConfig.back()[tmp].setMap(location);
				}
			}
			else
				this->_errorHandler(
					"Invalid number of arguments in [" + tokens[0] +
						"] directive, expected at least 1 and open bracket '{' "
						"after them",
					lineIndex,
					isTest,
					isTestPrint
				);
		}
		else
			this->_errorHandler(
				"Invalid directive [" + tokens[0] + "]",
				lineIndex,
				isTest,
				isTestPrint
			);
		tokens.clear();
		++lineIndex;
	}
	this->_file.clear();
	this->_file.seekg(0, std::ios::beg);
}

void	ServerConfig::printConfig(void)
{
	if (this->_file.is_open())
	{
		std::string line;

		std::cout << PURPLE "\n<WebServ> " GREEN "Printing configuration file("
			  << CYAN << this->filepath << GREEN ")..." RESET "\n\n";
		while (std::getline(this->_file, line))
			std::cout << line << '\n';
		std::cout << PURPLE "\n<WebServ> " << GREEN "\t---- End of file ----" << std::endl;
		this->_file.clear();
		this->_file.seekg(0, std::ios::beg);
	}
}
