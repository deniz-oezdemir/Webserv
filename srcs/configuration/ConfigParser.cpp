#include "ConfigParser.hpp"
#include "ServerException.hpp"
#include "colors.hpp"
#include "utils.hpp"

#include <cerrno>
#include <iostream>
#include <sys/stat.h>

// Handle the error message, if isTest or isTestPrint is true, print the error
// message without stopping the program. Otherwise, throw an exception.
void ConfigParser::errorHandler(
	std::string const  &message,
	unsigned int const &lineIndex,
	bool const		   &isTest,
	bool const		   &isTestPrint,
	std::string const  &filepath,
	bool			   &isConfigOK
)
{
	if (isTest || isTestPrint)
	{
		std::cerr << PURPLE "<WebServ> " << YELLOW "[EMERG] " RESET << message
				  << " in the configuration file " CYAN << filepath << ":"
				  << lineIndex << RESET << std::endl;
		isConfigOK = false;
	}
	else
		throw ServerException(
			message + " in the configuration file " + filepath + ":"
			+ ft::toString(lineIndex)
		);
}

bool ConfigParser::checkDirective(
	std::vector<std::string> const &tokens,
	unsigned int const			   &lineIndex,
	bool const					   &isTest,
	bool const					   &isTestPrint,
	std::string const			   &filepath,
	bool						   &isConfigOK
)
{
	if (tokens[0] == "limit_except")
		return ConfigParser::checkLimitExcept(
			tokens, lineIndex, isTest, isTestPrint, filepath, isConfigOK
		);

	else if (tokens[0] == "autoindex")
		return ConfigParser::checkAutoIndex(
			tokens, lineIndex, isTest, isTestPrint, filepath, isConfigOK
		);
	else if (tokens[0] == "return")
		return ConfigParser::checkReturn(
			tokens, lineIndex, isTest, isTestPrint, filepath, isConfigOK
		);
	else if (tokens[0] == "cgi")
		return ConfigParser::checkCgi(
			tokens, lineIndex, isTest, isTestPrint, filepath, isConfigOK
		);
	else if (tokens[0] == "upload_store")
		return ConfigParser::checkUploadStore(
			tokens, lineIndex, isTest, isTestPrint, filepath, isConfigOK
		);
	else if (tokens[0] == "client_max_body_size")
		return ConfigParser::checkClientMaxBodySize(
			tokens, lineIndex, isTest, isTestPrint, filepath, isConfigOK
		);
	else if (tokens[0] == "root")
		return ConfigParser::checkRoot(
			tokens, lineIndex, isTest, isTestPrint, filepath, isConfigOK
		);
	return false;
}

// Check the number of arguments in the directive, if the number of arguments is
// greater than maxSize, print an error message and return false. If the last
// argument does not end with ';', print an error message and return false.
bool ConfigParser::checkValues(
	std::vector<std::string> const &tokens,
	unsigned int const			   &maxSize,
	unsigned int const			   &lineIndex,
	bool const					   &isTest,
	bool const					   &isTestPrint,
	std::string const			   &filepath,
	bool						   &isConfigOK
)
{
	if (tokens.size() > maxSize)
	{
		ConfigParser::errorHandler(
			"Invalid number of arguments in [" + tokens[0] + "] directive",
			lineIndex,
			isTest,
			isTestPrint,
			filepath,
			isConfigOK
		);
		return false;
	}
	if (tokens[tokens.size() - 1][tokens[tokens.size() - 1].size() - 1] != ';')
	{
		ConfigParser::errorHandler(
			"Missing ';' in [" + tokens[0] + "] directive",
			lineIndex,
			isTest,
			isTestPrint,
			filepath,
			isConfigOK
		);
		return false;
	}
	return true;
}

bool ConfigParser::isValidErrorCode(std::string const &code)
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

bool ConfigParser::isURI(std::string const &uri)
{
	if (uri.size() < 2)
		return false;
	if (uri[0] != '/')
		return false;
	return true;
}

bool ConfigParser::isURL(std::string const &url)
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

bool ConfigParser::isExecutable(std::string const &path)
{
	struct stat fileStat;

	if (stat(path.c_str(), &fileStat) == 0)
	{
		if (fileStat.st_mode & S_IXUSR)
			return true;
	}
	return false;
}

bool ConfigParser::isDirectory(std::string const &path)
{
	struct stat fileStat;

	if (stat(path.c_str(), &fileStat) == 0)
	{
		if (S_ISDIR(fileStat.st_mode))
			return true;
	}
	return false;
}

bool ConfigParser::checkLimitExcept(
	std::vector<std::string> const &tokens,
	unsigned int const			   &lineIndex,
	bool const					   &isTest,
	bool const					   &isTestPrint,
	std::string const			   &filepath,
	bool						   &isConfigOK
)
{
	std::vector<std::string>::const_iterator it(tokens.begin() + 1);
	for (; it < tokens.end(); ++it)
	{
		if (*it != "GET" && *it != "POST" && *it != "DELETE" && *it != "PUT")
		{
			ConfigParser::errorHandler(
				"Invalid value [" + *it + "] for limit_except directive",
				lineIndex,
				isTest,
				isTestPrint,
				filepath,
				isConfigOK
			);
			return false;
		}
	}
	return true;
}

bool ConfigParser::checkAutoIndex(
	std::vector<std::string> const &tokens,
	unsigned int const			   &lineIndex,
	bool const					   &isTest,
	bool const					   &isTestPrint,
	std::string const			   &filepath,
	bool						   &isConfigOK
)
{
	if (tokens.size() == 2 && (tokens[1] == "on" || tokens[1] == "off"))
	{
		return true;
	}
	ConfigParser::errorHandler(
		"Invalid value [" + tokens[1] + "] for autoindex directive",
		lineIndex,
		isTest,
		isTestPrint,
		filepath,
		isConfigOK
	);
	return false;
}

bool ConfigParser::checkReturn(
	std::vector<std::string> const &tokens,
	unsigned int const			   &lineIndex,
	bool const					   &isTest,
	bool const					   &isTestPrint,
	std::string const			   &filepath,
	bool						   &isConfigOK
)
{
	if (tokens.size() == 3)
	{
		if (!ConfigParser::isValidErrorCode(tokens[1]))
		{
			ConfigParser::errorHandler(
				"Invalid error code [" + tokens[1] + "] for return directive",
				lineIndex,
				isTest,
				isTestPrint,
				filepath,
				isConfigOK
			);
			return false;
		}
		if (!ConfigParser::isURI(tokens[2]))
		{
			ConfigParser::errorHandler(
				"Invalid URI [" + tokens[2] + "] for return directive",
				lineIndex,
				isTest,
				isTestPrint,
				filepath,
				isConfigOK
			);
			return false;
		}
		return true;
	}
	else if (tokens.size() == 2)
	{
		if (!ConfigParser::isURI(tokens[1]))
		{
			ConfigParser::errorHandler(
				"Invalid URI [" + tokens[1] + "] for return directive",
				lineIndex,
				isTest,
				isTestPrint,
				filepath,
				isConfigOK
			);
			return false;
		}
		return true;
	}
	else
	{
		ConfigParser::errorHandler(
			"Invalid number of arguments for return directive",
			lineIndex,
			isTest,
			isTestPrint,
			filepath,
			isConfigOK
		);
		return false;
	}
}

bool ConfigParser::checkCgi(
	std::vector<std::string> const &tokens,
	unsigned int const			   &lineIndex,
	bool const					   &isTest,
	bool const					   &isTestPrint,
	std::string const			   &filepath,
	bool						   &isConfigOK
)
{
	if (tokens.size() == 3)
	{
		if (tokens[1].find('.') == std::string::npos)
		{
			ConfigParser::errorHandler(
				"Invalid file extension [" + tokens[1] + "] for CGI directive",
				lineIndex,
				isTest,
				isTestPrint,
				filepath,
				isConfigOK
			);
			return false;
		}
		if (!ConfigParser::isExecutable(tokens[2]))
		{
			ConfigParser::errorHandler(
				"Invalid executable [" + tokens[2] + "] for CGI directive",
				lineIndex,
				isTest,
				isTestPrint,
				filepath,
				isConfigOK
			);
			return false;
		}
		return true;
	}
	else if (tokens.size() == 2)
	{
		if (!ConfigParser::isExecutable(tokens[1]))
		{
			ConfigParser::errorHandler(
				"Invalid executable [" + tokens[1] + "] for CGI directive",
				lineIndex,
				isTest,
				isTestPrint,
				filepath,
				isConfigOK
			);
			return false;
		}
		return true;
	}
	else
	{
		ConfigParser::errorHandler(
			"Invalid number of arguments for CGI directive",
			lineIndex,
			isTest,
			isTestPrint,
			filepath,
			isConfigOK
		);
		return false;
	}
}

bool ConfigParser::checkUploadStore(
	std::vector<std::string> const &tokens,
	unsigned int const			   &lineIndex,
	bool const					   &isTest,
	bool const					   &isTestPrint,
	std::string const			   &filepath,
	bool						   &isConfigOK
)
{
	if (tokens.size() == 2)
	{
		if (!ConfigParser::isDirectory(tokens[1]))
		{
			ConfigParser::errorHandler(
				"Invalid directory [" + tokens[1]
					+ "] for upload_store directive",
				lineIndex,
				isTest,
				isTestPrint,
				filepath,
				isConfigOK
			);
			return false;
		}
		return true;
	}
	else
	{
		ConfigParser::errorHandler(
			"Invalid number of arguments for upload_store directive",
			lineIndex,
			isTest,
			isTestPrint,
			filepath,
			isConfigOK
		);
		return false;
	}
}

bool ConfigParser::checkClientMaxBodySize(
	std::vector<std::string> const &tokens,
	unsigned int const			   &lineIndex,
	bool const					   &isTest,
	bool const					   &isTestPrint,
	std::string const			   &filepath,
	bool						   &isConfigOK
)
{
	if (tokens.size() == 2)
	{
		if (!ft::isStrOfDigits(tokens[1]))
		{
			ConfigParser::errorHandler(
				"Invalid size [" + tokens[1]
					+ "] for client_max_body_size directive",
				lineIndex,
				isTest,
				isTestPrint,
				filepath,
				isConfigOK
			);
			return false;
		}
		return true;
	}
	else
	{
		ConfigParser::errorHandler(
			"Invalid number of arguments for client_max_body_size directive",
			lineIndex,
			isTest,
			isTestPrint,
			filepath,
			isConfigOK
		);
		return false;
	}
}

bool ConfigParser::checkRoot(
	std::vector<std::string> const &tokens,
	unsigned int const			   &lineIndex,
	bool const					   &isTest,
	bool const					   &isTestPrint,
	std::string const			   &filepath,
	bool						   &isConfigOK
)
{
	if (tokens.size() == 2)
	{
		if (!ConfigParser::isDirectory(tokens[1]))
		{
			ConfigParser::errorHandler(
				"Invalid directory [" + tokens[1] + "] for root directive",
				lineIndex,
				isTest,
				isTestPrint,
				filepath,
				isConfigOK
			);
			return false;
		}
		return true;
	}
	else
	{
		ConfigParser::errorHandler(
			"Invalid number of arguments for root directive",
			lineIndex,
			isTest,
			isTestPrint,
			filepath,
			isConfigOK
		);
		return false;
	}
}
