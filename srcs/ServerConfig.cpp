#include "ServerConfig.hpp"
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
std::array<std::string, 4> const ServerConfig::validLogLevels = {
	"debug",
	"info",
	"warn",
	"error",
};

ServerConfig::ServerConfig(std::string const &filepath)
	: filepath_(filepath), file_(filepath), isConfigOK_(true)
{
	if (!this->file_.is_open())
		throw ServerException("Could not open the file [%]", errno, filepath);
	this->initGeneralConfig_();
}

ServerConfig::ServerConfig(ServerConfig const &src)
	: filepath_(src.filepath_), file_(src.filepath_),
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
		this->file_.open(this->filepath_);

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
	std::map<std::string, std::vector<std::string> > listenMap;
	// clang-format on
	listenMap["host"] = std::vector<std::string>();
	listenMap["port"] = std::vector<std::string>();
	server["listen"] = ConfigValue(listenMap);
	server["root"] = ConfigValue();
	server["index"] = ConfigValue();
	server["client_max_body_size"] = ConfigValue();
	this->serversConfig_.push_back(server);
}

void ServerConfig::initLocationConfig_(
	// clang-format off
	std::map<std::string, std::vector<std::string> > &location
	// clang-format on
)
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

// Handle the error message, if isTest or isTestPrint is true, print the error
// message without stopping the program. Otherwise, throw an exception.
void ServerConfig::errorHandler_(
	std::string const &message,
	unsigned int	   lineIndex,
	bool			   isTest,
	bool			   isTestPrint
)
{
	if (isTest || isTestPrint)
	{
		std::cerr << PURPLE "<WebServ> " << YELLOW "[EMERG] " RESET << message
				  << " in the configuration file " CYAN << this->filepath_
				  << ":" << lineIndex << RESET << std::endl;
		this->isConfigOK_ = false;
	}
	else
		throw ServerException(
			message + " in the configuration file " + this->filepath_ + ":"
			+ ft::toString(lineIndex)
		);
}

// Check the number of arguments in the directive, if the number of arguments is
// greater than maxSize, print an error message and return false. If the last
// argument does not end with ';', print an error message and return false.
bool ServerConfig::checkValues_(
	std::vector<std::string> const &tokens,
	unsigned int					maxSize,
	unsigned int					lineIndex,
	bool							isTest,
	bool							isTestPrint
)
{
	if (tokens.size() > maxSize)
	{
		this->errorHandler_(
			"Invalid number of arguments in [" + tokens[0] + "] directive",
			lineIndex,
			isTest,
			isTestPrint
		);
		return false;
	}
	if (tokens[tokens.size() - 1][tokens[tokens.size() - 1].size() - 1] != ';')
	{
		this->errorHandler_(
			"Missing ';' in [" + tokens[0] + "] directive",
			lineIndex,
			isTest,
			isTestPrint
		);
		return false;
	}
	return true;
}

bool ServerConfig::isValidErrorCode_(std::string const &code)
{
	if (code.size() != 3)
		return false;
	if (!ft::isStrOfDigits(code))
		return false;
	unsigned short errorCode(ft::strToUShort(code));
	if (errorCode < 300 || errorCode > 599)
		return false;
	return true;
}

bool ServerConfig::isURI_(std::string const &uri)
{
	if (uri.size() < 2)
		return false;
	if (uri[0] != '/')
		return false;
	return true;
}

bool ServerConfig::isURL_(std::string const &url)
{
	if (url.size() < 8)
		return false;
	if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://")
		return false;
	if (url.find('.', url.find("://") + 3) == std::string::npos)
		return false;
	if (url.find('.', url.find("www.") + 4) == std::string::npos)
		return false;
	return true;
}

bool ServerConfig::isExecutable_(std::string const &path)
{
	struct stat fileStat;

	if (stat(path.c_str(), &fileStat) == 0)
	{
		if (fileStat.st_mode & S_IXUSR)
			return true;
	}
	return false;
}

bool ServerConfig::isDirectory_(std::string const &path)
{
	struct stat fileStat;

	if (stat(path.c_str(), &fileStat) == 0)
	{
		if (S_ISDIR(fileStat.st_mode))
			return true;
	}
	return false;
}

bool ServerConfig::checkDirective_(std::vector<std::string> const &tokens)
{
	if (tokens[0] == "limit_except")
		return this->checkLimitExcept_(tokens);
	else if (tokens[0] == "autoindex")
		return this->checkAutoIndex_(tokens);
	else if (tokens[0] == "return")
		return this->checkReturn_(tokens);
	else if (tokens[0] == "cgi")
		return this->checkCgi_(tokens);
	else if (tokens[0] == "upload_store")
		return this->checkUploadStore_(tokens);
	else if (tokens[0] == "client_max_body_size")
		return ft::isStrOfDigits(tokens[1]) && tokens.size() == 2;
	else if (tokens[0] == "root")
		return this->checkRoot_(tokens);
	return false;
}

bool ServerConfig::checkLimitExcept_(std::vector<std::string> const &tokens)
{
	std::vector<std::string>::const_iterator it(tokens.begin() + 1);
	for (; it < tokens.end(); ++it)
	{
		if (*it != "GET" && *it != "POST" && *it != "DELETE" && *it != "PUT")
			return false;
	}
	return true;
}

bool ServerConfig::checkAutoIndex_(std::vector<std::string> const &tokens)
{
	if (tokens.size() == 2 && (tokens[1] == "on" || tokens[1] == "off"))
		return true;
	return false;
}

bool ServerConfig::checkReturn_(std::vector<std::string> const &tokens)
{
	if ((tokens.size() == 2 && this->isValidErrorCode_(tokens[1]))
		|| (tokens.size() == 3 && this->isValidErrorCode_(tokens[1])
			&& (this->isURI_(tokens[2]) || this->isURL_(tokens[2]))))
		return true;
	return false;
}

bool ServerConfig::checkCgi_(std::vector<std::string> const &tokens)
{
	if ((tokens.size() == 3 && tokens[1].find('.') != std::string::npos
		 && this->isExecutable_(tokens[2]))
		|| (tokens.size() == 2 && this->isExecutable_(tokens[1])))
		return true;
	return false;
}

bool ServerConfig::checkUploadStore_(std::vector<std::string> const &tokens)
{
	if (tokens.size() == 2 && this->isDirectory_(tokens[1]))
		return true;
	return false;
}

bool ServerConfig::checkClientMaxBodySize_(
	std::vector<std::string> const &tokens
)
{
	if (tokens.size() == 2 && ft::isStrOfDigits(tokens[1]))
		return true;
	return false;
}

bool ServerConfig::checkRoot_(std::vector<std::string> const &tokens)
{
	if (tokens.size() == 2 && this->isDirectory_(tokens[1]))
		return true;
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
			this->errorHandler_(
				"Duplicate listen directive [" + host + ":" + port
					+ "] is a server block",
				lineIndex,
				isTest,
				isTestPrint
			);
		}
	}
	catch (std::exception &e)
	{
		this->errorHandler_(
			"Invalid value [" + tokens[1] + "] for " + tokens[0]
				+ " directive: " + e.what(),
			lineIndex,
			isTest,
			isTestPrint
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
		if (this->checkValues_(tokens, 99, lineIndex, isTest, isTestPrint))
		{
			if (location.find(tokens[0]) != location.end())
			{
				tokens[tokens.size() - 1].erase(
					tokens[tokens.size() - 1].size() - 1
				);
				if (this->checkDirective_(tokens))
				{
					location[tokens[0]] = std::vector<std::string>(
						tokens.begin() + 1, tokens.end()
					);
				}
				else
				{
					this->errorHandler_(
						"Invalid value [" + tokens[1] + "] for " + tokens[0]
							+ " directive",
						lineIndex,
						isTest,
						isTestPrint
					);
				}
			}
			else
			{
				this->errorHandler_(
					"Invalid directive [" + tokens[0] + "] in location block",
					lineIndex,
					isTest,
					isTestPrint
				);
			}
		}
		else
		{
			this->errorHandler_(
				"Invalid directive [" + tokens[0]
					+ "] in location block, the line should end with ';'",
				lineIndex,
				isTest,
				isTestPrint
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
		this->errorHandler_(
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
			if (!brackets.empty())
				brackets.pop();
			else
				this->errorHandler_(
					"Missing '{' to pair '}'", lineIndex, isTest, isTestPrint
				);
		}
		else if (tokens[0] == "worker_processes"
				 || tokens[0] == "worker_connections")
		{
			if (this->checkValues_(tokens, 2, lineIndex, isTest, isTestPrint))
			{
				tokens[1].erase(tokens[1].size() - 1);
				if (ft::isStrOfDigits(tokens[1]) || tokens[1] == "auto")
					this->generalConfig_[tokens[0]] = tokens[1];
				else
					this->errorHandler_(
						"Invalid value [" + tokens[1] + "] for " + tokens[0]
							+ " directive",
						lineIndex,
						isTest,
						isTestPrint
					);
			}
		}
		else if (tokens[0] == "error_log")
		{
			if (this->checkValues_(tokens, 2, lineIndex, isTest, isTestPrint))
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
					this->errorHandler_(
						"Invalid value [" + tokens[1] + "] for " + tokens[0]
							+ " directive",
						lineIndex,
						isTest,
						isTestPrint
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
				this->errorHandler_(
					"Missing '{' in [" + tokens[0] + "] directive",
					lineIndex,
					isTest,
					isTestPrint
				);
			if (tokens[0] == "server")
				this->initServersConfig_();
		}
		else if (tokens[0] == "server_name" || tokens[0] == "index")
		{
			if (tokens.size() > 1)
			{
				if (this->checkValues_(
						tokens, 99, lineIndex, isTest, isTestPrint
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
						this->errorHandler_(
							"Invalid directive [" + tokens[0]
								+ "] outside a server block",
							lineIndex,
							isTest,
							isTestPrint
						);
				}
			}
			else
				this->errorHandler_(
					"Invalid number of arguments in [" + tokens[0]
						+ "] directive, expected at least 1",
					lineIndex,
					isTest,
					isTestPrint
				);
		}
		else if (tokens[0] == "listen" || tokens[0] == "root"
				 || tokens[0] == "client_max_body_size")
		{
			if (this->checkValues_(tokens, 2, lineIndex, isTest, isTestPrint))
			{
				tokens[1].erase(tokens[1].size() - 1);
				if ((tokens[0] == "client_max_body_size" || tokens[0] == "root")
					&& !this->checkDirective_(tokens))
					this->errorHandler_(
						"Invalid value [" + tokens[1] + "] for " + tokens[0]
							+ " directive",
						lineIndex,
						isTest,
						isTestPrint
					);
				else
				{
					if (!this->serversConfig_.empty())
					{
						// If the directive is listen, set the host and port in
						// the listen as a map of host and port.
						if (tokens[0] == "listen")
							this->setListenDirective_(
								tokens, lineIndex, isTest, isTestPrint
							);
						else
							this->serversConfig_.back()[tokens[0]]
								= ConfigValue(std::vector<std::string>(
									tokens.begin() + 1, tokens.end()
								));
					}
					else
						this->errorHandler_(
							"Invalid directive [" + tokens[0]
								+ "] outside a server block",
							lineIndex,
							isTest,
							isTestPrint
						);
				}
			}
		}
		else if (tokens[0] == "error_page")
		{
			if (tokens.size() > 2
				&& this->checkValues_(
					tokens, 99, lineIndex, isTest, isTestPrint
				))
			{
				tokens[tokens.size() - 1].erase(
					tokens[tokens.size() - 1].size() - 1
				);
				std::vector<std::string>::iterator it(tokens.begin() + 1);
				for (; it < tokens.end() - 1; ++it)
				{
					if (!ft::isStrOfDigits(*it))
						this->errorHandler_(
							"Invalid value [" + *it + "] for " + tokens[0]
								+ " directive, expected a status code",
							lineIndex,
							isTest,
							isTestPrint
						);
					else
					{
						if (this->serversConfig_.size() > 0)
							this->serversConfig_.back()[*it]
								= ConfigValue(std::vector<std::string>(
									tokens.end() - 1, tokens.end()
								));
						else
							this->errorHandler_(
								"Invalid directive [" + tokens[0]
									+ "] outside a server block",
								lineIndex,
								isTest,
								isTestPrint
							);
					}
				}
			}
			else
				this->errorHandler_(
					"Invalid number of arguments in [" + tokens[0]
						+ "] directive, expected at leat 2",
					lineIndex,
					isTest,
					isTestPrint
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
				this->errorHandler_(
					"Invalid number of arguments in [" + tokens[0]
						+ "] directive, expected one and open bracket '{' "
						  "after them",
					lineIndex,
					isTest,
					isTestPrint
				);
		}
		else
			this->errorHandler_(
				"Invalid directive [" + tokens[0] + "]",
				lineIndex,
				isTest,
				isTestPrint
			);
		tokens.clear();
		++lineIndex;
	}
	if (!brackets.empty())
		this->errorHandler_(
			"Format error: Missing '}' to pair '{'",
			lineIndex,
			isTest,
			isTestPrint
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
			this->errorHandler_(
				"Missing error_log directive", 0, isTest, isTestPrint
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
		this->errorHandler_(
			"Missing [server] directive", 0, isTest, isTestPrint
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
				this->errorHandler_(
					"Missing [server_name] directive, set default value: "
					"'localhost'",
					0,
					isTest,
					isTestPrint
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
		// 			this->errorHandler_(
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
				this->errorHandler_(
					"Missing [listen] directive, set default value: "
					"'0.0.0.0:8080'",
					0,
					isTest,
					isTestPrint
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
				this->errorHandler_(
					"Duplicate listen directive on server block["
						+ ft::toString(it - this->serversConfig_.begin()) + "]",
					0,
					isTest,
					isTest
				);
			}
		}
		if (it->find("root")->second.getVector().empty())
		{
			if (isTest || isTestPrint)
				this->errorHandler_(
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
				this->errorHandler_(
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
				this->errorHandler_(
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
		this->errorHandler_(
			"Invalid number of arguments in [listen] directive",
			lineIndex,
			isTest,
			isTestPrint
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
