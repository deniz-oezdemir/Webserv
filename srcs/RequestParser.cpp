#include "../include/RequestParser.hpp"

HttpRequest RequestParser::parseRequest(std::string str)
{
	std::string						   method;
	std::string						   httpVersion;
	std::string						   target;
	std::map<std::string, std::string> headers;
	std::vector<char>				   body;

	// Replace all occurrences of "\r\n" with "\n" in the HTTP request
	std::string::size_type pos = 0;
	while ((pos = str.find("\r\n", pos)) != std::string::npos)
	{
		str.replace(pos, 2, "\n");
		pos += 1;
	}

	std::istringstream requestStream(str.c_str());

	// Parse the start line
	std::string startLine;
	std::getline(requestStream, startLine, '\n');
	checkStartLine(startLine, &method, &target, &httpVersion);

	// Parse the headers
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

			headers[headerName] = headerValue;
		}
	}

	// Parse the body
	std::string bodyLine;
	while (std::getline(requestStream, bodyLine))
	{
		if (!bodyLine.empty() && bodyLine[bodyLine.size() - 1] == '\r')
		{
			bodyLine.erase(bodyLine.size() - 1); // Remove the trailing '\r'
		}
		body.insert(body.end(), bodyLine.begin(), bodyLine.end());
	}

	HttpRequest req(method, httpVersion, target, headers, body);

	return req;
}

void RequestParser::checkStartLine(
	std::string &startLine,
	std::string *method,
	std::string *target,
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
	if (!(startLineStream >> *method >> *target >> *httpVersion))
	{
		Logger::log(Logger::INFO)
			<< "Request start line is malformed: " << startLine << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
	checkMethod(*method);
	checkTarget(*target);
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
		throw HttpException(HTTP_501_CODE, HTTP_501_REASON);
	}
}

void RequestParser::checkTarget(std::string &target)
{
	if (target == "*")
		return;

	if (target[0] != '/')
	{
		Logger::log(Logger::INFO)
			<< "Target is not \'*\' and does not start with /: " << target
			<< std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}

	for (std::size_t i = 0; i < target.size(); ++i)
	{
		char c = target[i];
		if (!std::isalnum(c) &&
			std::string("-._~:/?#[]@!$&'()*+,;=%").find(c) == std::string::npos)
		{
			Logger::log(Logger::INFO)
				<< "Target contains incorrect characters: " << target
				<< std::endl;
			throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
		}

		if (c == '%' &&
			(i + 2 >= target.size() || !std::isxdigit(target[i + 1]) ||
			 !std::isxdigit(target[i + 2])))
		{
			Logger::log(Logger::INFO)
				<< "Target contains non-hex characters after \'%\': " << target
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

// TODO: check that only list containing headers are allowed to appear more than
// once
void RequestParser::checkHeaders(
	const std::map<std::string, std::string> &headers
)
{
	bool hasHost = false;

	for (std::map<std::string, std::string>::const_iterator it =
			 headers.begin();
		 it != headers.end();
		 ++it)
	{
		// Only the Host header is allowed empty values
		if (it->first.empty() || (it->first != "Host" && it->second.empty()))
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
	}

	if (!hasHost)
	{
		Logger::log(Logger::INFO) << "Absent Host header." << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
}
