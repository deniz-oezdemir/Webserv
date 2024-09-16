#include "request_parser/RequestParser.hpp"
#include "request_parser/BodyParser.hpp"
#include "request_parser/FirstLineParser.hpp"
#include "request_parser/HeaderParser.hpp"
#include "request_parser/HttpHeaders.hpp"
#include <cctype>
#include <cstddef>
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

	// Only create map once, c++98 sucks!
	if (!headerAcceptedChars.empty())
		createHeaderAcceptedChars();

	// Replace all occurrences of "\r\n" with "\n" in the HTTP request
	std::string::size_type pos = 0;
	while ((pos = str.find("\r\n", pos)) != std::string::npos)
	{
		str.replace(pos, 2, "\n");
		pos += 1;
	}

	std::istringstream requestStream(str.c_str());

	// Extract the start line
	std::string startLine;
	std::getline(requestStream, startLine, '\n');
	ParseReqFirstLine::checkStartLine(startLine, &method, &uri, &httpVersion);

	// Extract headers
	rawHeaders = HeaderParser::parseHeaders(requestStream);
	// Normalize headers
	headers = unifyHeaders_(rawHeaders);
	// TODO: check token syntax

	// Extract and check the body
	BodyParser::parseBody(requestStream, method, headers, &body);

	HttpRequest req(method, httpVersion, uri, headers, body);
	return req;
}

// used for comma-separated HTTP request header values
std::vector<std::string> RequestParser::splitHeaderValue_(
	const std::string &headerName,
	const std::string &headerValue
)
{
	char separator = ',';
	for (int i = 0; i < SEMICOLONSEPARATE_N; i++)
	{
		if (semicolonSeparated[i] == headerName)
			separator = ';';
	}
	std::vector<std::string> values;
	std::string				 value;
	std::istringstream		 stream(headerValue);
	while (std::getline(stream, value, separator))
	{
		// Trim leading and trailing whitespace
		size_t start = value.find_first_not_of(" \t");
		size_t end = value.find_last_not_of(" \t");
		if (start != std::string::npos && end != std::string::npos)
		{
			value = value.substr(start, end - start + 1);
			if (!value.empty())
			{
				values.push_back(value);
			}
		}
	}
	return values;
}

// clang-format off
std::map<std::string, std::vector<std::string> >
RequestParser::unifyHeaders_(std::multimap<std::string, std::string> multimap)
{
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on

	for (std::multimap<std::string, std::string>::iterator it
		 = multimap.begin();
		 it != multimap.end();
		 ++it)
	{
		std::vector<std::string> newVector;
		if (multimap.count(it->first) == 1) // only one ocurrance
		{
			newVector = splitHeaderValue_(it->first, it->second);
			headers[it->first] = newVector;
		}
		else if (multimap.count(it->first) > 1) // header repeated
		{
			std::pair<
				std::multimap<std::string, std::string>::iterator,
				std::multimap<std::string, std::string>::iterator>
						matches = multimap.equal_range(it->first);
			std::string appendedValues;
			for (; matches.first != matches.second; ++matches.first)
			{
				if (!appendedValues.empty())
				{
					appendedValues += ","; // Add comma between values
				}
				appendedValues += matches.first->second;
			}
			newVector = splitHeaderValue_(it->first, appendedValues);
			headers[it->first] = newVector;
		}
	}

	return headers;
}

// void RequestParser::
// 	// clang-format off
// 	checkTokenSyntax(std::map<std::string, std::vector<std::string> > headers)
// // clang-format on
// {
// 	// clang-format off
// 	for (std::map<std::string, std::vector<std::string> >::const_iterator it
// 		// clang-format on
// 		= headers.begin();
// 		it != headers.end();
// 		++it)
// 	{
// 		if (headerAcceptedChars.find(it->first) != headerAcceptedChars.end())
// 		{
// 			for (std::vector<char>::iterator vIt = it->second;
// 				 vIt != it->second.end();
// 				 ++vIt)
// 			{
// 			}
// 		}
// 	}
// }
