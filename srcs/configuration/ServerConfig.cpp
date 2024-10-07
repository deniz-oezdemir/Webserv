#include "ServerConfig.hpp"
#include "ConfigParser.hpp"
#include "ServerException.hpp"
#include "colors.hpp"
#include "macros.hpp"
#include "utils.hpp"

#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <stack>
#include <string>
#include <sys/stat.h>
#include <vector>

// Array of valid log levels to check if the log level set in the configuration
// file is valid.
std::vector<std::string> const ServerConfig::validLogLevels
	= ft::initLogLevels();

ServerConfig::ServerConfig(std::string const &filepath)
	: filepath_(filepath), file_(filepath.c_str()), isConfigOK_(true)
{
	if (!this->file_.is_open())
		throw ServerException("Could not open the file [%]", errno, filepath);
	this->initGeneralConfig_();
}

ServerConfig::ServerConfig(ServerConfig const &src)
	: filepath_(src.filepath_), file_(src.filepath_.c_str()),
	  isConfigOK_(src.isConfigOK())
{
	if (!this->file_.is_open())
		throw ServerException("Could not open the file [%]", errno, filepath_);
}

ServerConfig &ServerConfig::operator=(ServerConfig const &src)
{
	if (this != &src)
	{
		this->filepath_ = src.filepath_;
		this->isConfigOK_ = src.isConfigOK();

		if (this->file_.is_open())
			this->file_.close();

		this->file_.clear();
		this->file_.open(this->filepath_.c_str());

		if (!this->file_.is_open())
			throw ServerException(
				"Could not open the file [%]", errno, filepath_
			);
	}
	return *this;
}

ServerConfig::~ServerConfig()
{
	if (this->file_.is_open())
		this->file_.close();
}

std::ifstream &ServerConfig::getFile(void)
{
	return this->file_;
}

bool ServerConfig::isConfigOK(void) const
{
	return this->isConfigOK_;
}

// General directives in the configuration file.
void ServerConfig::initGeneralConfig_(void)
{
	this->generalConfig_["worker_processes"] = "";
	this->generalConfig_["worker_connections"] = "";
	this->generalConfig_["error_log"] = "info";
}

// Server directives in the configuration file.
void ServerConfig::initServersConfig_(void)
{
	std::map<std::string, ConfigValue> server;
	server["server_name"] = ConfigValue();
	// clang-format off
	std::map<std::string, std::vector<std::string> > listenMap; // clang-format on
	listenMap["host"] = std::vector<std::string>();
	listenMap["port"] = std::vector<std::string>();
	server["listen"] = ConfigValue(listenMap);
	server["root"] = ConfigValue();
	server["index"] = ConfigValue();
	server["client_max_body_size"] = ConfigValue();
	this->serversConfig_.push_back(server);
}

// clang-format off
void ServerConfig::initLocationConfig_(
	std::map<std::string, std::vector<std::string> > &location
) // clang-format on
{
	location["root"] = std::vector<std::string>();
	location["index"] = std::vector<std::string>();
	location["client_max_body_size"] = std::vector<std::string>();
	location["limit_except"] = std::vector<std::string>();
	location["autoindex"] = std::vector<std::string>();
	location["return"] = std::vector<std::string>();
	location["upload_store"] = std::vector<std::string>();
	location["cgi"] = std::vector<std::string>();
}

bool ServerConfig::checkDirective_(
	std::vector<std::string> const &tokens,
	unsigned int const			   &lineIndex,
	bool const					   &isTest,
	bool const					   &isTestPrint
)
{
	if (tokens[0] == "limit_except")
		return ConfigParser::checkLimitExcept(
			tokens,
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);

	else if (tokens[0] == "autoindex")
		return ConfigParser::checkAutoIndex(
			tokens,
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
	else if (tokens[0] == "return")
		return ConfigParser::checkReturn(
			tokens,
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
	else if (tokens[0] == "cgi")
		return ConfigParser::checkCgi(
			tokens,
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
	else if (tokens[0] == "upload_store")
		return ConfigParser::checkUploadStore(
			tokens,
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
	else if (tokens[0] == "client_max_body_size")
		return ConfigParser::checkClientMaxBodySize(
			tokens,
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
	else if (tokens[0] == "root")
		return ConfigParser::checkRoot(
			tokens,
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
	return false;
}

// Set the host and port in the listen directive. If the argument is a port
// number, set the port. If the argument is an IP address and a port number,
// split the argument and set the host and port. Otherwise, print an error
// message.
void ServerConfig::setListenDirective_(
	std::vector<std::string> const &tokens,
	unsigned int				   &lineIndex,
	bool						   &isTest,
	bool						   &isTestPrint
)
{
	std::string host(DEFAULT_IP);
	std::string port(DEFAULT_PORT_STR);
	try
	{
		// If the argument is a port number, set the port.
		if (ft::isStrOfDigits(tokens[1]))
		{
			// Check if the port number is valid.
			if (!ft::isUShort(tokens[1]))
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
			if (tmp.size() == 2 && ft::isUShort(tmp[1])
				&& ft::isValidIPv4(tmp[0]))
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
		if (this->checkServerListenUnique_(
				host, port, lineIndex, isTest, isTestPrint
			))
		{
			// Set the host and port in the listen as a map with the keys host
			// and port.
			this->serversConfig_.back()["listen"].pushBackMapValue(
				"host", host
			);
			this->serversConfig_.back()["listen"].pushBackMapValue(
				"port", port
			);
		}
		else
		{
			ConfigParser::errorHandler(
				"Duplicate listen directive [" + host + ":" + port
					+ "] is a server block",
				lineIndex,
				isTest,
				isTestPrint,
				this->filepath_,
				this->isConfigOK_
			);
		}
	}
	catch (std::exception &e)
	{
		ConfigParser::errorHandler(
			"Invalid value [" + tokens[1] + "] for " + tokens[0]
				+ " directive: " + e.what(),
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
	}
}

// Parse the location block, if the argument is a location block, parse the
// block and set the location as a map. Otherwise, print an error message.
void ServerConfig::parseLocationBlock_(
	std::vector<std::string> &tokens,
	std::string				 &line,
	unsigned int			 &lineIndex,
	std::stack<bool>		 &brackets,
	bool					 &isTest,
	bool					 &isTestPrint
)
{
	std::string uri(tokens[1]);
	// clang-format off
	std::map<std::string, std::vector<std::string> > location;
	// clang-format on
	this->initLocationConfig_(location);
	tokens.clear();
	++lineIndex;
	while (std::getline(this->file_, line))
	{
		ft::trim(line);
		if (line.empty() || line[0] == '#')
		{
			++lineIndex;
			continue;
		}
		ft::split(tokens, line);
		if (tokens[0] == "}")
		{
			brackets.pop();
			break;
		}
		if (ConfigParser::checkValues(
				tokens,
				99,
				lineIndex,
				isTest,
				isTestPrint,
				this->filepath_,
				this->isConfigOK_
			))
		{
			if (location.find(tokens[0]) != location.end())
			{
				tokens[tokens.size() - 1].erase(
					tokens[tokens.size() - 1].size() - 1
				);
				if (this->checkDirective_(
						tokens, lineIndex, isTest, isTestPrint
					))
				{
					location[tokens[0]] = std::vector<std::string>(
						tokens.begin() + 1, tokens.end()
					);
				}
			}
			else
			{
				ConfigParser::errorHandler(
					"Invalid directive [" + tokens[0] + "] in location block",
					lineIndex,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
			}
		}
		else
		{
			ConfigParser::errorHandler(
				"Invalid directive [" + tokens[0]
					+ "] in location block, the line should end with ';'",
				lineIndex,
				isTest,
				isTestPrint,
				this->filepath_,
				this->isConfigOK_
			);
		}
		tokens.clear();
		++lineIndex;
	}
	if (this->serversConfig_.size() > 0)
	{
		if (location["limit_except"].empty())
		{
			std::vector<std::string> methods(4);
			methods[0] = "GET";
			methods[1] = "POST";
			methods[2] = "DELETE";
			methods[3] = "PUT";
			location["limit_except"] = methods;
		}
		if (location["client_max_body_size"].empty())
			location["client_max_body_size"]
				= serversConfig_.back()["client_max_body_size"].getVector();
		if (location["autoindex"].empty())
			location["autoindex"] = std::vector<std::string>(1, "off");
		this->serversConfig_.back()[uri].setMap(location);
	}
	else
		ConfigParser::errorHandler(
			"Invalid directive [" + tokens[0] + "] outside a server block",
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
}

void ServerConfig::handleClosingBracket(
	std::stack<bool> &brackets,
	unsigned int	  lineIndex,
	bool			  isTest,
	bool			  isTestPrint
)
{
	if (!brackets.empty())
		brackets.pop();
	else
		ConfigParser::errorHandler(
			"Missing '{' to pair '}'",
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
}

bool ServerConfig::isValidLogLevel(const std::string &logLevel)
{
	return std::find(
			   this->validLogLevels.begin(),
			   this->validLogLevels.end(),
			   logLevel
		   )
		   != this->validLogLevels.end();
}

void ServerConfig::handleGeneralDirective(
	std::vector<std::string> &tokens,
	unsigned int			  lineIndex,
	bool					  isTest,
	bool					  isTestPrint
)
{
	if (ConfigParser::checkValues(
			tokens,
			2,
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		))
	{
		tokens[1].erase(tokens[1].size() - 1);
		if (ft::isStrOfDigits(tokens[1]) || tokens[1] == "auto"
			|| (tokens[0] == "error_log" && isValidLogLevel(tokens[1])))
			this->generalConfig_[tokens[0]] = tokens[1];
		else
			ConfigParser::errorHandler(
				"Invalid value [" + tokens[1] + "] for " + tokens[0]
					+ " directive",
				lineIndex,
				isTest,
				isTestPrint,
				this->filepath_,
				this->isConfigOK_
			);
	}
}

bool ServerConfig::isGeneralDirective(const std::string &directive)
{
	return directive == "worker_processes" || directive == "worker_connections"
		   || directive == "error_log";
}

void ServerConfig::parseFile(bool isTest, bool isTestPrint)
{
	std::stack<bool>		 brackets;
	std::string				 line;
	unsigned int			 lineIndex(1);
	std::vector<std::string> tokens;
	tokens.reserve(6);

	while (std::getline(this->file_, line))
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
			handleClosingBracket(brackets, lineIndex, isTest, isTestPrint);
		}
		else if (tokens[0] == "worker_processes"
				 || tokens[0] == "worker_connections")
		{
			if (ConfigParser::checkValues(
					tokens,
					2,
					lineIndex,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				))
			{
				tokens[1].erase(tokens[1].size() - 1);
				if (ft::isStrOfDigits(tokens[1]) || tokens[1] == "auto")
					this->generalConfig_[tokens[0]] = tokens[1];
				else
					ConfigParser::errorHandler(
						"Invalid value [" + tokens[1] + "] for " + tokens[0]
							+ " directive",
						lineIndex,
						isTest,
						isTestPrint,
						this->filepath_,
						this->isConfigOK_
					);
			}
		}
		else if (tokens[0] == "error_log")
		{
			if (ConfigParser::checkValues(
					tokens,
					2,
					lineIndex,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				))
			{
				tokens[1].erase(tokens[1].size() - 1);
				if (ft::find(
						this->validLogLevels.begin(),
						this->validLogLevels.end(),
						tokens[1]
					)
					!= this->validLogLevels.end())
					this->generalConfig_[tokens[0]] = tokens[1];
				else
					ConfigParser::errorHandler(
						"Invalid value [" + tokens[1] + "] for " + tokens[0]
							+ " directive",
						lineIndex,
						isTest,
						isTestPrint,
						this->filepath_,
						this->isConfigOK_
					);
			}
		}
		// If the directive is http, server or events, check if the directive
		// has an opening bracket '{'. If it doesn't, print an error message.
		else if (tokens[0] == "http" || tokens[0] == "server"
				 || tokens[0] == "events")
		{
			if (tokens.size() == 2 && tokens[1] == "{")
				brackets.push(true);
			else
				ConfigParser::errorHandler(
					"Missing '{' in [" + tokens[0] + "] directive",
					lineIndex,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
			if (tokens[0] == "server")
				this->initServersConfig_();
		}
		else if (tokens[0] == "server_name" || tokens[0] == "index")
		{
			if (tokens.size() > 1)
			{
				if (ConfigParser::checkValues(
						tokens,
						99,
						lineIndex,
						isTest,
						isTestPrint,
						this->filepath_,
						this->isConfigOK_
					))
				{
					tokens[tokens.size() - 1].erase(
						tokens[tokens.size() - 1].size() - 1
					);
					if (this->serversConfig_.size() > 0)
						this->serversConfig_.back()[tokens[0]]
							= ConfigValue(std::vector<std::string>(
								tokens.begin() + 1, tokens.end()
							));
					else
						ConfigParser::errorHandler(
							"Invalid directive [" + tokens[0]
								+ "] outside a server block",
							lineIndex,
							isTest,
							isTestPrint,
							this->filepath_,
							this->isConfigOK_
						);
				}
			}
			else
				ConfigParser::errorHandler(
					"Invalid number of arguments in [" + tokens[0]
						+ "] directive, expected at least 1",
					lineIndex,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
		}
		else if (tokens[0] == "listen" || tokens[0] == "root"
				 || tokens[0] == "client_max_body_size")
		{
			if (ConfigParser::checkValues(
					tokens,
					2,
					lineIndex,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				))
			{
				tokens[1].erase(tokens[1].size() - 1);
				if (!this->serversConfig_.empty())
				{
					if ((tokens[0] == "client_max_body_size"
						 || tokens[0] == "root")
						&& this->checkDirective_(
							tokens, lineIndex, isTest, isTestPrint
						))
					{
						this->serversConfig_.back()[tokens[0]]
							= ConfigValue(std::vector<std::string>(
								tokens.begin() + 1, tokens.end()
							));
					}
					else if (tokens[0] == "listen")
						this->setListenDirective_(
							tokens, lineIndex, isTest, isTestPrint
						);
				}
				else
					ConfigParser::errorHandler(
						"Invalid directive [" + tokens[0]
							+ "] outside a server block",
						lineIndex,
						isTest,
						isTestPrint,
						this->filepath_,
						this->isConfigOK_
					);
			}
		}
		else if (tokens[0] == "error_page")
		{
			if (tokens.size() > 2
				&& ConfigParser::checkValues(
					tokens,
					99,
					lineIndex,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				))
			{
				tokens[tokens.size() - 1].erase(
					tokens[tokens.size() - 1].size() - 1
				);
				std::vector<std::string>::iterator it(tokens.begin() + 1);
				for (; it < tokens.end() - 1; ++it)
				{
					if (!ft::isStrOfDigits(*it))
						ConfigParser::errorHandler(
							"Invalid value [" + *it + "] for " + tokens[0]
								+ " directive, expected a status code",
							lineIndex,
							isTest,
							isTestPrint,
							this->filepath_,
							this->isConfigOK_
						);
					else
					{
						if (this->serversConfig_.size() > 0)
							this->serversConfig_.back()[*it]
								= ConfigValue(std::vector<std::string>(
									tokens.end() - 1, tokens.end()
								));
						else
							ConfigParser::errorHandler(
								"Invalid directive [" + tokens[0]
									+ "] outside a server block",
								lineIndex,
								isTest,
								isTestPrint,
								this->filepath_,
								this->isConfigOK_
							);
					}
				}
			}
			else
				ConfigParser::errorHandler(
					"Invalid number of arguments in [" + tokens[0]
						+ "] directive, expected at leat 2",
					lineIndex,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
		}
		else if (tokens[0] == "location")
		{
			if (tokens.size() == 3 && tokens[tokens.size() - 1] == "{")
			{
				brackets.push(true);
				this->parseLocationBlock_(
					tokens, line, lineIndex, brackets, isTest, isTestPrint
				);
			}
			else
				ConfigParser::errorHandler(
					"Invalid number of arguments in [" + tokens[0]
						+ "] directive, expected one and open bracket '{' "
						  "after them",
					lineIndex,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
		}
		else
			ConfigParser::errorHandler(
				"Invalid directive [" + tokens[0] + "]",
				lineIndex,
				isTest,
				isTestPrint,
				this->filepath_,
				this->isConfigOK_
			);
		tokens.clear();
		++lineIndex;
	}
	if (!brackets.empty())
		ConfigParser::errorHandler(
			"Format error: Missing '}' to pair '{'",
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
	this->checkGeneralConfig_(isTest, isTestPrint);
	this->checkServersConfig_(isTest, isTestPrint);
	// Clear the file stream flags and set the file stream position to the
	// beginning of the file.
	this->file_.clear();
	this->file_.seekg(0, std::ios::beg);
}

// Check the mandatory diirectives in the general configuration. If the
// directive is missing, set the default value.
void ServerConfig::checkGeneralConfig_(bool isTest, bool isTestPrint)
{
	if (this->generalConfig_["error_log"].empty())
	{
		if (isTest || isTestPrint)
			ConfigParser::errorHandler(
				"Missing error_log directive",
				0,
				isTest,
				isTestPrint,
				this->filepath_,
				this->isConfigOK_
			);
		this->generalConfig_["error_log"] = "info";
	}
}

// Check the mandatory directives in the server configuration. If the directive
// is missing, set the default value.
void ServerConfig::checkServersConfig_(bool isTest, bool isTestPrint)
{
	if (this->serversConfig_.empty())
	{
		ConfigParser::errorHandler(
			"Missing [server] directive",
			0,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
		return;
	}
	// clang-format off
	std::vector<std::map<std::string, ConfigValue> >::iterator it(
		this->serversConfig_.begin()
	);
	// clang-format on
	for (; it != this->serversConfig_.end(); ++it)
	{
		if (it->find("server_name")->second.getVector().empty())
		{
			if (isTest || isTestPrint)
				ConfigParser::errorHandler(
					"Missing [server_name] directive, set default value: "
					"'localhost'",
					0,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
			it->find("server_name")
				->second.setVector(std::vector<std::string>(1, DEFAULT_HOST));
		}
		// NOTE: Uncomment the code below if the server_name should be unique.
		//
		// else
		// {
		// 	std::vector<std::string>::const_iterator it2(
		// 		it->find("server_name")->second.getVector().begin()
		// 	);
		// 	for (; it2 != it->find("server_name")->second.getVector().end();
		// 		 ++it2)
		// 	{
		// 		if (!this->checkServerNameUnique_(*it2))
		// 		{
		// 			this->ConfigParser::errorHandler(
		// 				"Duplicate server name [" + *it2 + "]",
		// 				0,
		// 				isTest,
		// 				isTestPrint
		// 			);
		// 		}
		// 	}
		// }

		if (it->find("listen")->second.getMap().empty()
			&& it->find("listen")->second.getMapValue("ports").empty()
			&& it->find("listen")->second.getMapValue("host").empty())
		{
			if (isTest || isTestPrint)
				ConfigParser::errorHandler(
					"Missing [listen] directive, set default value: "
					"'0.0.0.0:8080'",
					0,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
			// clang-format off
			std::map<std::string, std::vector<std::string> > listenMap;
			// clang-format on
			listenMap["host"] = std::vector<std::string>(1, DEFAULT_IP);
			listenMap["port"] = std::vector<std::string>(1, DEFAULT_PORT_STR);
			it->find("listen")->second.setMap(listenMap);
		}
		else
		{
			if (!this->checkListenUnique_(it))
			{
				ConfigParser::errorHandler(
					"Duplicate listen directive on server block["
						+ ft::toString(it - this->serversConfig_.begin()) + "]",
					0,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
			}
		}
		if (it->find("root")->second.getVector().empty())
		{
			if (isTest || isTestPrint)
				ConfigParser::errorHandler(
					"Missing [root] directive, set default value: './www'",
					0,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
			it->find("root")->second.setVector(
				std::vector<std::string>(1, "./www")
			);
		}
		if (it->find("index")->second.getVector().empty())
		{
			if (isTest || isTestPrint)
				ConfigParser::errorHandler(
					"Missing [index] directive, set default value: "
					"'index.html'",
					0,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
			it->find("index")->second.setVector(
				std::vector<std::string>(1, "index.html")
			);
		}
		if (it->find("client_max_body_size")->second.getVector().empty())
		{
			if (isTest || isTestPrint)
				ConfigParser::errorHandler(
					"Missing [client_max_body_size] directive, set default "
					"value: "
					"'1M'",
					0,
					isTest,
					isTestPrint,
					this->filepath_,
					this->isConfigOK_
				);
			it->find("client_max_body_size")
				->second.setVector(std::vector<std::string>(1, "1048576"));
		}
	}
}

void ServerConfig::printConfig(void)
{
	if (this->file_.is_open())
	{
		std::string line;

		std::cout << PURPLE "\n<WebServ> " GREEN "Printing configuration file("
				  << CYAN << this->filepath_ << GREEN ")..." RESET "\n\n";
		while (std::getline(this->file_, line))
			std::cout << line << '\n';
		std::cout << PURPLE "\n<WebServ> " << GREEN "\t---- End of file ----"
				  << std::endl;
		this->file_.clear();
		this->file_.seekg(0, std::ios::beg);
	}
}

// Get the value of a key in the general configuration map.
std::string ServerConfig::getGeneralConfigValue(std::string const &key) const
{
	std::string										   value;
	std::map<std::string, std::string>::const_iterator it(
		this->generalConfig_.find(key)
	);
	if (it != this->generalConfig_.end())
	{
		value = it->second;
		return value;
	}
	return value;
}

// Get all servers stored in a vector of maps.
// clang-format off
bool ServerConfig::getAllServersConfig(
	std::vector<std::map<std::string, ConfigValue> > &serversConfig
) const
// clang-format on
{
	if (this->serversConfig_.empty())
		return false;
	serversConfig = this->serversConfig_;
	return true;
}

// clang-format off
std::vector<std::map<std::string, ConfigValue> > const &
// clang-format on
ServerConfig::getAllServersConfig(void) const
{
	return this->serversConfig_;
}

// Get the value of a key in a server[serverIndex] configuration map.
bool ServerConfig::getServerConfigValue(
	unsigned int	   serverIndex,
	std::string const &key,
	ConfigValue		  &value
) const
{
	if (serverIndex >= this->serversConfig_.size())
		return false;
	std::map<std::string, ConfigValue>::const_iterator it(
		this->serversConfig_[serverIndex].find(key)
	);
	if (it != this->serversConfig_[serverIndex].end())
	{
		value = it->second;
		return true;
	}
	return false;
}

bool ServerConfig::checkServerNameUnique_(std::string const &serverName)
{
	unsigned int count(0);
	// clang-format off
	std::vector<std::map<std::string, ConfigValue> >::const_iterator it(
		this->serversConfig_.begin()
	);
	// clang-format on
	for (; it != this->serversConfig_.end(); ++it)
	{
		std::vector<std::string>::const_iterator it2(
			it->find("server_name")->second.getVector().begin()
		);
		for (; it2 != it->find("server_name")->second.getVector().end(); ++it2)
		{
			if (*it2 == serverName)
				++count;
		}
	}
	if (count != 1)
		return false;
	return true;
}

// clang-format off
bool ServerConfig::checkListenUnique_(
	std::vector<std::map<std::string, ConfigValue> >::iterator &itServer
)
{
	std::vector<std::map<std::string, ConfigValue> >::const_iterator itServers
		= serversConfig_.begin(); // clang-format on
	for (; itServers != serversConfig_.end(); ++itServers)
	{
		if (itServers == itServer)
			continue;
		std::vector<std::string>::const_iterator itPorts(
			itServers->find("listen")->second.getMapValue("port").begin()
		);
		for (; itPorts
			   != itServers->find("listen")->second.getMapValue("port").end();
			 ++itPorts)
		{
			std::vector<std::string>::const_iterator itPorts2(
				itServer->find("listen")->second.getMapValue("port").begin()
			);
			for (;
				 itPorts2
				 != itServer->find("listen")->second.getMapValue("port").end();
				 ++itPorts2)
			{
				if (*itPorts == *itPorts2)
				{
					return false;
				}
			}
		}
	}
	return true;
}

bool ServerConfig::checkServerListenUnique_(
	const std::string &host,
	const std::string &port,
	unsigned int	  &lineIndex,
	bool			  &isTest,
	bool			  &isTestPrint
)
{
	const std::map<std::string, ConfigValue> &lastServerConfig
		= this->serversConfig_.back();
	if (lastServerConfig.find("listen") == lastServerConfig.end())
	{
		return true;
	}

	const std::vector<std::string> &hosts
		= lastServerConfig.at("listen").getMapValue("host");
	const std::vector<std::string> &ports
		= lastServerConfig.at("listen").getMapValue("port");

	if (hosts.size() != ports.size())
	{
		ConfigParser::errorHandler(
			"Invalid number of arguments in [listen] directive",
			lineIndex,
			isTest,
			isTestPrint,
			this->filepath_,
			this->isConfigOK_
		);
		return false;
	}

	for (size_t i = 0; i < hosts.size(); ++i)
	{
		if (hosts[i] == host && ports[i] == port)
		{
			return false;
		}
	}

	return true;
}

void ServerConfig::setRootToAllServers(std::string const &root)
{
	// clang-format off
	std::vector<std::map<std::string, ConfigValue> >::iterator it(
		this->serversConfig_.begin()
	);
	// clang-format on
	for (; it != this->serversConfig_.end(); ++it)
		it->find("root")->second.setVector(std::vector<std::string>(1, root));
}
