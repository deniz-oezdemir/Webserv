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
		// std::cout << "Replaced! str now is: " << str << std::endl;
	}

	std::istringstream requestStream(str.c_str());

	// Parse the first line
	std::string firstLine;
	std::getline(requestStream, firstLine, '\n');
	if (!firstLine.empty() && firstLine[firstLine.size() - 1] == '\r')
	{
		firstLine.erase(firstLine.size() - 1); // Remove the trailing '\r'
	}

	// for now I'm extracting the firs line elements assuming correct space
	// separation
	std::istringstream firstLineStream(firstLine.c_str());
	if (!(firstLineStream >> method >> target >> httpVersion))
	{
		// Extraction operation failed, handle the error
		throw std::runtime_error("failed to parse first line of request"
		);
	}

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
