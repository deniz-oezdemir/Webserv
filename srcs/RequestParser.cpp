#include "../include/RequestParser.hpp"

HttpRequest RequestParser::parseRequest(std::string str)
{
	std::string						   method;
	std::string						   httpVersion;
	std::string						   target;
	std::map<std::string, std::string> headers;
	std::vector<char>				   body;

	std::istringstream requestStream(str.c_str());

	// Parse the first line
	std::string firstLine;
	std::getline(requestStream, firstLine);
	if (!firstLine.empty() && firstLine[firstLine.size() - 1] == '\r')
	{
		firstLine.erase(firstLine.size() - 1); // Remove the trailing '\r'
	}

	std::istringstream firstLineStream(firstLine.c_str());
	firstLineStream >> method >> target >> httpVersion;

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
