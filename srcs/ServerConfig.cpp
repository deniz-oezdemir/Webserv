#include "ServerConfig.hpp"
#include "ServerException.hpp"
#include "colors.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <iostream>
#include <stack>
#include <string>

// Array of valid log levels to check if the log level set in the configuration
// file is valid.
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
	: filepath(src.filepath), _file(src.filepath), _isConfigOK(src.isConfigOK())
{
	if (!this->_file.is_open())
		throw ServerException("Could not open the file [%]", errno, filepath);
}

ServerConfig &ServerConfig::operator=(ServerConfig const &src)
{
	if (this != &src)
	{
		this->filepath = src.filepath;
		this->_isConfigOK = src.isConfigOK();

		if (this->_file.is_open())
			this->_file.close();

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

bool ServerConfig::isConfigOK(void) const
{
	return this->_isConfigOK;
}

// General directives in the configuration file.
void ServerConfig::_initGeneralConfig(void)
{
	this->_generalConfig["worker_processes"] = "";
	this->_generalConfig["worker_connections"] = "";
	this->_generalConfig["error_log"] = "info";
}

// Server directives in the configuration file.
void ServerConfig::_initServersConfig(void)
{
	std::map<std::string, ConfigValue> server;
	server["server_name"] = ConfigValue();
	server["listen"] = ConfigValue();
	server["root"] = ConfigValue();
	server["index"] = ConfigValue();
	server["client_max_body_size"] = ConfigValue();
	this->_serversConfig.push_back(server);
}

// Handle the error message, if isTest or isTestPrint is true, print the error
// message without stopping the program. Otherwise, throw an exception.
void ServerConfig::_errorHandler(
	std::string const &message,
	unsigned int	   lineIndex,
	bool			   isTest,
	bool			   isTestPrint
)
{
	if (isTest || isTestPrint)
	{
		std::cerr << PURPLE "<WebServ> " << YELLOW "[EMERG] " RESET << message
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

// Check the number of arguments in the directive, if the number of arguments is
// greater than maxSize, print an error message and return false. If the last
// argument does not end with ';', print an error message and return false.
bool ServerConfig::_checkValues(
	std::vector<std::string> const &tokens,
	unsigned int					maxSize,
	unsigned int					lineIndex,
	bool							isTest,
	bool							isTestPrint
)
{
	if (tokens.size() > maxSize)
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

// Set the host and port in the listen directive. If the argument is a port
// number, set the port. If the argument is an IP address and a port number,
// split the argument and set the host and port. Otherwise, print an error
// message.
void ServerConfig::_setListenDirective(
	std::vector<std::string> const &tokens,
	unsigned int				   &lineIndex,
	bool						   &isTest,
	bool						   &isTestPrint
)
{
	std::string host("0.0.0.0");
	std::string port("80");
	try
	{
		// If the argument is a port number, set the port.
		if (ft::isStrOfDigits(tokens[1]))
		{
			// Check if the port number is valid.
			if (!ft::isUint16(tokens[1]))
				throw std::invalid_argument("Invalid port number");
			port = tokens[1];
		}
		// If the argument is an host:port, split the argument and set the host
		// and port.
		else if (tokens[1].find(':') != std::string::npos)
		{
			std::vector<std::string> tmp;
			ft::split(tmp, tokens[1], ":");
			// Check if the argument is host:port, check if the port number is
			// valid and if the IP address is valid
			if (tmp.size() == 2 && ft::isUint16(tmp[1]) &&
				ft::isValidIPv4(tmp[0]))
			{
				host = tmp[0];
				port = tmp[1];
			}
			else
				throw std::invalid_argument("Invalid IP address or port number"
				);
		}
		else
			throw std::invalid_argument(
				"Invalid format, expected host:port or port"
			);
		// Set the host and port in the listen as a map of host and port.
		std::map<std::string, std::vector<std::string> > listenMap;
		listenMap["host"] = std::vector<std::string>(1, host);
		listenMap["port"] = std::vector<std::string>(1, port);
		this->_serversConfig.back()["listen"] = ConfigValue(listenMap);
	}
	catch (std::exception &e)
	{
		this->_errorHandler(
			"Invalid value [" + tokens[1] + "] for " + tokens[0] +
				" directive: " + e.what(),
			lineIndex,
			isTest,
			isTestPrint
		);
	}
}

// Parse the location block, if the argument is a location block, parse the
// block and set the location as a map. Otherwise, print an error message.
void ServerConfig::_parseLocationBlock(
	std::vector<std::string> &tokens,
	std::string				 &line,
	unsigned int			 &lineIndex,
	std::stack<bool>		 &brackets,
	bool					 &isTest,
	bool					 &isTestPrint
)
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
		{
			brackets.pop();
			break;
		}
		if (this->_checkValues(tokens, 99, lineIndex, isTest, isTestPrint))
		{
			tokens[tokens.size() - 1].erase(
				tokens[tokens.size() - 1].size() - 1
			);
			location[tokens[0]] =
				std::vector<std::string>(tokens.begin() + 1, tokens.end());
		}
		tokens.clear();
		++lineIndex;
	}
	if (this->_serversConfig.size() > 0)
		this->_serversConfig.back()[tmp].setMap(location);
	else
		this->_errorHandler(
			"Invalid directive [" + tokens[0] + "] outside a server block",
			lineIndex,
			isTest,
			isTestPrint
		);
}

void ServerConfig::parseFile(bool isTest, bool isTestPrint)
{
	std::stack<bool>		 brackets;
	std::string				 line;
	unsigned int			 lineIndex(1);
	std::vector<std::string> tokens;
	// Reserve memory for the tokens to avoid reallocations.
	tokens.reserve(6);
	while (std::getline(this->_file, line))
	{
		ft::trim(line);
		// Skip empty lines and comments.
		if (line.empty() || line[0] == '#')
		{
			++lineIndex;
			continue;
		}

		ft::split(tokens, line);
		// Check if the directive is a closing bracket '}'. If it is, pop the
		// bracket from the stack. If the stack is empty, print an error
		// message for unmatched brackets.
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
				if (ft::find(
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
		// If the directive is http, server or events, check if the directive
		// has an opening bracket '{'. If it doesn't, print an error message.
		else if (tokens[0] == "http" || tokens[0] == "server" ||
				 tokens[0] == "events")
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
			if (tokens.size() > 1)
			{
				if (this->_checkValues(
						tokens, 99, lineIndex, isTest, isTestPrint
					))
				{
					tokens[tokens.size() - 1].erase(
						tokens[tokens.size() - 1].size() - 1
					);
					if (this->_serversConfig.size() > 0)
						this->_serversConfig.back()[tokens[0]] =
							ConfigValue(std::vector<std::string>(
								tokens.begin() + 1, tokens.end()
							));
					else
						this->_errorHandler(
							"Invalid directive [" + tokens[0] +
								"] outside a server block",
							lineIndex,
							isTest,
							isTestPrint
						);
				}
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
				if (tokens[0] == "client_max_body_size" &&
					!ft::isStrOfDigits(tokens[1]))
					this->_errorHandler(
						"Invalid value [" + tokens[1] + "] for " + tokens[0] +
							" directive, expected a port number",
						lineIndex,
						isTest,
						isTestPrint
					);
				else
				{
					if (!this->_serversConfig.empty())
					{
						// If the directive is listen, set the host and port in
						// the listen as a map of host and port.
						if (tokens[0] == "listen")
							this->_setListenDirective(
								tokens, lineIndex, isTest, isTestPrint
							);
						else
							this->_serversConfig.back()[tokens[0]] =
								ConfigValue(std::vector<std::string>(
									tokens.begin() + 1, tokens.end()
								));
					}
					else
						this->_errorHandler(
							"Invalid directive [" + tokens[0] +
								"] outside a server block",
							lineIndex,
							isTest,
							isTestPrint
						);
				}
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
					{
						if (this->_serversConfig.size() > 0)
							this->_serversConfig.back()[*it] =
								ConfigValue(std::vector<std::string>(
									tokens.end() - 1, tokens.end()
								));
						else
							this->_errorHandler(
								"Invalid directive [" + tokens[0] +
									"] outside a server block",
								lineIndex,
								isTest,
								isTestPrint
							);
					}
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
				brackets.push(true);
				if (tokens.size() == 4 &&
					(tokens[1] != "~" && tokens[1] != "=" &&
					 tokens[1] != "^~" && tokens[1] != "~*"))
					this->_errorHandler(
						"Invalid value [" + tokens[1] + "] for " + tokens[0] +
							" directive, expected '~', '=', '^~' or '~*'",
						lineIndex,
						isTest,
						isTestPrint
					);
				else
					this->_parseLocationBlock(
						tokens, line, lineIndex, brackets, isTest, isTestPrint
					);
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
	if (!brackets.empty())
		this->_errorHandler(
			"Format error: Missing '}' to pair '{'",
			lineIndex,
			isTest,
			isTestPrint
		);
	this->_checkGeneralConfig(isTest, isTestPrint);
	this->_checkServersConfig(isTest, isTestPrint);
	// Clear the file stream flags and set the file stream position to the
	// beginning of the file.
	this->_file.clear();
	this->_file.seekg(0, std::ios::beg);
}

// Check the mandatory diirectives in the general configuration. If the
// directive is missing, set the default value.
void ServerConfig::_checkGeneralConfig(bool isTest, bool isTestPrint)
{
	if (this->_generalConfig["error_log"].empty())
	{
		if (isTest || isTestPrint)
			this->_errorHandler(
				"Missing error_log directive", 0, isTest, isTestPrint
			);
		this->_generalConfig["error_log"] = "info";
	}
}

// Check the mandatory directives in the server configuration. If the directive
// is missing, set the default value.
void ServerConfig::_checkServersConfig(bool isTest, bool isTestPrint)
{
	if (this->_serversConfig.empty())
	{
		this->_errorHandler(
			"Missing [server] directive", 0, isTest, isTestPrint
		);
		return;
	}
	std::vector<std::map<std::string, ConfigValue> >::iterator it(
		this->_serversConfig.begin()
	);
	for (; it != this->_serversConfig.end(); ++it)
	{
		if (it->find("server_name")->second.getVector().empty())
		{
			if (isTest || isTestPrint)
				this->_errorHandler(
					"Missing [server_name] directive, set default value: "
					"'localhost'",
					0,
					isTest,
					isTestPrint
				);
			it->find("server_name")
				->second.setVector(std::vector<std::string>(1, "_"));
		}
		if (it->find("listen")->second.getMap().empty())
		{
			if (isTest || isTestPrint)
				this->_errorHandler(
					"Missing [listen] directive, set default value: "
					"'0.0.0.0:80'",
					0,
					isTest,
					isTestPrint
				);
			std::map<std::string, std::vector<std::string> > listenMap;
			listenMap["host"] = std::vector<std::string>(1, "0.0.0.0");
			listenMap["port"] = std::vector<std::string>(1, "80");
			it->find("listen")->second.setMap(listenMap);
		}
		if (it->find("root")->second.getVector().empty())
		{
			if (isTest || isTestPrint)
				this->_errorHandler(
					"Missing [root] directive, set default value: './www'",
					0,
					isTest,
					isTestPrint
				);
			it->find("root")->second.setVector(
				std::vector<std::string>(1, "./www")
			);
		}
		if (it->find("index")->second.getVector().empty())
		{
			if (isTest || isTestPrint)
				this->_errorHandler(
					"Missing [index] directive, set default value: "
					"'index.html'",
					0,
					isTest,
					isTestPrint
				);
			it->find("index")->second.setVector(
				std::vector<std::string>(1, "index.html")
			);
		}
		if (it->find("client_max_body_size")->second.getVector().empty())
		{
			if (isTest || isTestPrint)
				this->_errorHandler(
					"Missing [client_max_body_size] directive, set default "
					"value: "
					"'1M'",
					0,
					isTest,
					isTestPrint
				);
			it->find("client_max_body_size")
				->second.setVector(std::vector<std::string>(1, "1048576"));
		}
	}
}

void ServerConfig::printConfig(void)
{
	if (this->_file.is_open())
	{
		std::string line;

		std::cout << PURPLE "\n<WebServ> " GREEN "Printing configuration file("
				  << CYAN << this->filepath << GREEN ")..." RESET "\n\n";
		while (std::getline(this->_file, line))
			std::cout << line << '\n';
		std::cout << PURPLE "\n<WebServ> " << GREEN "\t---- End of file ----"
				  << std::endl;
		this->_file.clear();
		this->_file.seekg(0, std::ios::beg);
	}
}

// Get the value of a key in the general configuration map.
std::string ServerConfig::getGeneralConfigValue(std::string const &key) const
{
	std::string										   value;
	std::map<std::string, std::string>::const_iterator it(
		this->_generalConfig.find(key)
	);
	if (it != this->_generalConfig.end())
	{
		value = it->second;
		return value;
	}
	return value;
}

// Get all servers stored in a vector of maps.
bool ServerConfig::getAllServersConfig(
	std::vector<std::map<std::string, ConfigValue> > &serversConfig
) const
{
	if (this->_serversConfig.empty())
		return false;
	serversConfig = this->_serversConfig;
	return true;
}

// Get the value of a key in a server[serverIndex] configuration map.
bool ServerConfig::getServerConfigValue(
	unsigned int	   serverIndex,
	std::string const &key,
	ConfigValue		  &value
) const
{
	if (serverIndex >= this->_serversConfig.size())
		return false;
	std::map<std::string, ConfigValue>::const_iterator it(
		this->_serversConfig[serverIndex].find(key)
	);
	if (it != this->_serversConfig[serverIndex].end())
	{
		value = it->second;
		return true;
	}
	return false;
}
