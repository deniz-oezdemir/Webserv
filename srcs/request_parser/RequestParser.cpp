#include "request_parser/RequestParser.hpp"
#include "Logger.hpp"
#include "request_parser/BodyParser.hpp"
#include "request_parser/FirstLineParser.hpp"
#include "request_parser/HeaderParser.hpp"
#include "request_parser/HttpHeaders.hpp"
#include "request_parser/TokenValidator.hpp"
#include <cctype>
#include <cstdlib>
#include <map>
#include <sstream>
#include <vector>

HttpRequest RequestParser::parseRequest(std::string str)
{
	std::string								method;
	std::string								httpVersion;
	std::string								uri;
	std::multimap<std::string, std::string> rawHeaders;
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	std::vector<char> body;

	// Initialize the map with accepted headers
	// Only create map once, c++98 sucks!
	if (!headerAcceptedChars.empty())
		createHeaderAcceptedChars();

	std::istringstream requestStream(str);

	// Print request_string to debug
	Logger::log(Logger::DEBUG) << "Request string: " << str << std::endl;

	// Extract and parse the start line
	std::string firstLine;
	std::getline(requestStream, firstLine, '\n');
	ParseReqFirstLine::checkStartLine(firstLine, &method, &uri, &httpVersion);

	// Extract, parse and normalize headers
	HeaderParser::parseHeaders(requestStream, &headers);

	// Check token syntax
	TokenValidator::validateTokens(headers);

	// Extract and check the body
	BodyParser::parseBody(requestStream, method, headers, &body);

	HttpRequest req(method, httpVersion, uri, headers, body);
	return req;
}
