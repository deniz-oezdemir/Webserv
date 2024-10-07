#pragma once

#include <string>
#include <vector>

class ConfigParser
{
  public:
	static void errorHandler(
		std::string const  &message,
		unsigned int const &lineIndex,
		bool const		   &isTest,
		bool const		   &isTestPrint,
		std::string const  &filepath,
		bool			   &isConfigOK
	);

	static bool checkValues(
		std::vector<std::string> const &line,
		unsigned int const			   &maxSize,
		unsigned int const			   &lineIndex,
		bool const					   &isTest,
		bool const					   &isTestPrint,
		std::string const			   &filepath,
		bool						   &isConfigOK
	);

	static bool isValidErrorCode(std::string const &code);
	static bool isURI(std::string const &uri);
	static bool isURL(std::string const &url);
	static bool isExecutable(std::string const &path);
	static bool isDirectory(std::string const &path);

	static bool checkDirective(
		std::vector<std::string> const &tokens,
		unsigned int const			   &lineIndex,
		bool const					   &isTest,
		bool const					   &isTestPrint,
		std::string const			   &filepath,
		bool						   &isConfigOK
	);
	static bool checkLimitExcept(
		std::vector<std::string> const &tokens,
		unsigned int const			   &lineIndex,
		bool const					   &isTest,
		bool const					   &isTestPrint,
		std::string const			   &filepath,
		bool						   &isConfigOK
	);
	static bool checkAutoIndex(
		std::vector<std::string> const &tokens,
		unsigned int const			   &lineIndex,
		bool const					   &isTest,
		bool const					   &isTestPrint,
		std::string const			   &filepath,
		bool						   &isConfigOK
	);
	static bool checkReturn(
		std::vector<std::string> const &tokens,
		unsigned int const			   &lineIndex,
		bool const					   &isTest,
		bool const					   &isTestPrint,
		std::string const			   &filepath,
		bool						   &isConfigOK
	);
	static bool checkCgi(
		std::vector<std::string> const &tokens,
		unsigned int const			   &lineIndex,
		bool const					   &isTest,
		bool const					   &isTestPrint,
		std::string const			   &filepath,
		bool						   &isConfigOK
	);
	static bool checkUploadStore(
		std::vector<std::string> const &tokens,
		unsigned int const			   &lineIndex,
		bool const					   &isTest,
		bool const					   &isTestPrint,
		std::string const			   &filepath,
		bool						   &isConfigOK
	);
	static bool checkClientMaxBodySize(
		std::vector<std::string> const &tokens,
		unsigned int const			   &lineIndex,
		bool const					   &isTest,
		bool const					   &isTestPrint,
		std::string const			   &filepath,
		bool						   &isConfigOK
	);
	static bool checkRoot(
		std::vector<std::string> const &tokens,
		unsigned int const			   &lineIndex,
		bool const					   &isTest,
		bool const					   &isTestPrint,
		std::string const			   &filepath,
		bool						   &isConfigOK
	);

  private:
};
