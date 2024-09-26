#pragma once

#include <string>

class ParseReqFirstLine
{
  public:
	/*
	*/
	static void checkStartLine(
		std::string &startLine,
		std::string *method,
		std::string *uri,
		std::string *httpVersion
	);

  private:
	ParseReqFirstLine(void);
	ParseReqFirstLine(const ParseReqFirstLine &src);
	~ParseReqFirstLine(void);
	ParseReqFirstLine &operator=(const ParseReqFirstLine &rhs);

	static void checkMethod_(std::string &method);
	static void checkUri_(std::string &uri);
	static void checkHttpVersion_(std::string &httpVersion);
};
