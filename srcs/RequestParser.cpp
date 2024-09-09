#include "../include/RequestParser.hpp"
#include "../include/HttpException.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <vector>

const std::string RequestParser::repeatableHeaders[20]
	= {"Accept",
	   "Accept-Charset",
	   "Accept-Encoding",
	   "Accept-Language",
	   "Allow",
	   "Cache-Control",
	   "Connection",
	   "Content-Encoding",
	   "Content-Language",
	   "Expect",
	   "Pragma",
	   "Proxy-Authenticate",
	   "TE",
	   "Trailer",
	   "Transfer-Encoding",
	   "Upgrade",
	   "Vary",
	   "Via",
	   "Warning",
	   "WWW-Authenticate"};

HttpRequest RequestParser::parseRequest(std::string str)
{
	std::string								method;
	std::string								httpVersion;
	std::string								uri;
	std::multimap<std::string, std::string> headers;
	std::vector<char>						body;

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

	// Extract the headers
	std::string headerLine;
	while (std::getline(requestStream, headerLine) && !headerLine.empty())
	{
		if (headerLine[headerLine.size() - 1] == '\r')
		{
			headerLine.erase(headerLine.size() - 1); // Remove the trailing '\r'
		}

		std::size_t colonPos = headerLine.find(':');
		if (colonPos != std::string::npos)
		{
			std::string headerName = headerLine.substr(0, colonPos);
			std::string headerValue = headerLine.substr(colonPos + 1);

			// Trim leading spaces
			std::size_t firstNonSpacePos = headerValue.find_first_not_of(' ');
			if (firstNonSpacePos != std::string::npos)
			{
				headerValue = headerValue.substr(firstNonSpacePos);
			}

			headers.insert(
				std::pair<std::string, std::string>(headerName, headerValue)
			);
		}
	}

	// Extract the body
	std::string bodyLine;
	while (std::getline(requestStream, bodyLine))
	{
		// if (!bodyLine.empty() && bodyLine[bodyLine.size() - 1] == '\r')
		// {
		// 	bodyLine.erase(bodyLine.size() - 1); // Remove the trailing '\r'
		// }
		body.insert(body.end(), bodyLine.begin(), bodyLine.end());
		body.push_back('\n'); // Preserve the newline character
	}

	checkStartLine(startLine, &method, &uri, &httpVersion);
	checkHeaders(headers);
	checkBody(method, headers, body);

	HttpRequest req(method, httpVersion, uri, headers, body);
	return req;
}

void RequestParser::checkStartLine(
	std::string &startLine,
	std::string *method,
	std::string *uri,
	std::string *httpVersion
)
{
	if (startLine.empty())
	{
		Logger::log(Logger::INFO) << "Request start line is empy." << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
	if (startLine[startLine.size() - 1] == '\r')
	{
		startLine.erase(startLine.size() - 1); // Remove the trailing '\r'
	}

	std::istringstream startLineStream(startLine.c_str());
	if (!(startLineStream >> *method >> *uri >> *httpVersion))
	{
		Logger::log(Logger::INFO)
			<< "Request start line is malformed: " << startLine << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
	checkMethod(*method);
	checkUri(*uri);
	checkHttpVersion(*httpVersion);
}

void RequestParser::checkMethod(std::string &method)
{
	std::string array[] = HTTP_ACCEPTED_METHODS;
	size_t		arraySize = sizeof(array) / sizeof(std::string);
	if (std::find(array, array + arraySize, method) == array + arraySize)
	{
		Logger::log(Logger::INFO)
			<< "Method not found: " << method << std::endl;
		// commented out below as 501 handled with response to client by
		// ServerEngine::handleNotImplementedRequest
		// throw HttpException(HTTP_501_CODE, HTTP_501_REASON);
	}
}

void RequestParser::checkUri(std::string &uri)
{
	if (uri == "*")
		return;

	if (uri[0] != '/')
	{
		Logger::log(Logger::INFO)
			<< "Uri is not \'*\' and does not start with /: " << uri
			<< std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}

	for (std::size_t i = 0; i < uri.size(); ++i)
	{
		char c = uri[i];
		if (!std::isalnum(c)
			&& std::string("-._~:/?#[]@!$&'()*+,;=%").find(c)
				   == std::string::npos)
		{
			Logger::log(Logger::INFO)
				<< "Uri contains incorrect characters: " << uri << std::endl;
			throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
		}

		if (c == '%'
			&& (i + 2 >= uri.size() || !std::isxdigit(uri[i + 1])
				|| !std::isxdigit(uri[i + 2])))
		{
			Logger::log(Logger::INFO)
				<< "Uri contains non-hex characters after \'%\': " << uri
				<< std::endl;
			throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
		}
	}
}

void RequestParser::checkHttpVersion(std::string &httpVersion)
{
	if (httpVersion != "HTTP/1.1")
	{
		Logger::log(Logger::INFO)
			<< "HTTP version is not \'HTTP/1.1\': " << httpVersion << std::endl;
		throw HttpException(HTTP_501_CODE, HTTP_501_REASON);
	}
}

void RequestParser::checkHeaders(
	const std::multimap<std::string, std::string> &headers
)
{
	bool													hasHost = false;
	std::map<std::string, int>								headerCounts;
	std::multimap<std::string, std::string>::const_iterator it;

	for (std::multimap<std::string, std::string>::const_iterator it
		 = headers.begin();
		 it != headers.end();
		 ++it)
	{
		// Only the Host header is allowed empty values
		if (it->second.empty() && it->first != "Host")
		{
			Logger::log(Logger::INFO)
				<< "Header has emtpy value and is not Host. Header: "
				<< it->first << std::endl;
			throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
		}

		// Host header may appear only once
		if (it->first == "Host")
		{
			if (hasHost)
			{
				Logger::log(Logger::INFO)
					<< "Repeated Host header." << std::endl;
				throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
			}
			hasHost = true;
		}
		// Check if non-repeatable header appears more than once
		if (headers.count(it->first) > 0)
		{
			headerCounts[it->first]++;
			if (headerCounts[it->first] > 1)
			{
				// TODO: turn into method
				for (int i = 0; i < 20; ++i)
				{
					if (repeatableHeaders[i] == it->first)
					{
						continue;
					}
				}
				Logger::log(Logger::INFO)
					<< "Non-repeatable header appears more than once. Header: "
					<< it->first << std::endl;
				throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
			}
		}
	}

	if (!hasHost)
	{
		Logger::log(Logger::INFO) << "Absent Host header." << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
}

void RequestParser::checkBody(
	const std::string							  &method,
	const std::multimap<std::string, std::string> &headers,
	const std::vector<char>						  &body
)
{
	if ((method == "GET" || method == "DELETE") && !body.empty())
	{
		// TODO: check again this rule
		Logger::log(Logger::INFO)
			<< "Body should be empty for GET or DELETE requests." << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}

	if (headers.count("Content-Length") > 0
		&& (unsigned long
		   )std::atol(headers.find("Content-Length")->second.c_str())
			   != body.size())
	{
		Logger::log(Logger::INFO)
			<< "Content-Length does not match actual body length." << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}

	if (headers.count("Transfer-Encoding") > 0
		&& headers.find("Transfer-Encoding")->second.find("chunked")
			   != std::string::npos)
	{
		// TODO: Check that the body is in the chunked transfer coding format
	}
}
