#include "request_parser/HeaderParser.hpp"
#include "HttpException.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include "request_parser/HttpHeaders.hpp"
#include <map>
#include <string>

/**
 * @brief Parses the headers from the given request stream.
 *
 * This function reads the headers from the provided request stream, performs
 * syntax checks on individual headers, and returns a multimap of header names
 * and values.
 *
 * @param requestStream The input stream containing the HTTP request headers.
 * @return std::multimap<std::string, std::string> A multimap containing the
 * parsed headers.
 * @throws HttpException if there are syntax errors in the headers.
 */
std::multimap<std::string, std::string>
HeaderParser::parseHeaders(std::istream &requestStream)
{

	std::multimap<std::string, std::string> rawHeaders;
	std::string								headerLine;

	// Extract the headers and do syntax check on individual headers
	std::getline(requestStream, headerLine);
	while (!headerLine.empty())
	{
		if (headerLine[headerLine.size() - 1] == '\r')
		{
			headerLine.erase(headerLine.size() - 1); // Remove the trailing '\r'
		}

		checkSingleHeader(headerLine);

		std::size_t colonPos = headerLine.find(':');
		if (colonPos != std::string::npos)
		{
			std::string headerName = headerLine.substr(0, colonPos);
			std::string headerValue = headerLine.substr(colonPos + 1);

			// Trim leading spaces TODO: remove trailing spaces??
			std::size_t firstNonSpacePos = headerValue.find_first_not_of(' ');
			if (firstNonSpacePos != std::string::npos)
			{
				headerValue = headerValue.substr(firstNonSpacePos);
			}

			rawHeaders.insert(
				std::pair<std::string, std::string>(headerName, headerValue)
			);
		}
		std::getline(requestStream, headerLine);
	}

	checkRawHeaders(rawHeaders);

	return rawHeaders;
}

/**
 * @brief Checks if the header name contains only allowed characters.
 *
 * Allowed characters are alphanumeric characters, hyphens (-), underscores (_),
 * and periods (.).
 *
 * @param headerName The header name to validate.
 * @return true if the header name is valid, false otherwise.
 */
bool HeaderParser::isValidHeaderName(std::string headerName)
{
	for (std::string::iterator it = headerName.begin(); it != headerName.end();
		 ++it)
	{
		if (std::isalnum(*it) == 0 && *it != '-' && *it != '_' && *it != '.')
			return false;
	}
	return true;
}

/**
 * @brief Checks if the header value contains only allowed characters.
 *
 * This function checks that the header value does not contain control
 * characters and that special characters are properly escaped.
 *
 * @param headerValue The header value to validate.
 * @return true if the header value is valid, false otherwise.
 */
bool HeaderParser::isValidHeaderValue(std::string headerValue)
{
	// TODO: check this rule and understand tokens. Check if needed
	// std::string specialChars = "()<>@,;:\"/[]?={} \t";
	std::string specialChars = "";

	for (std::string::const_iterator it = headerValue.begin();
		 it != headerValue.end();
		 ++it)
	{
		if (std::iscntrl(*it))
			return false;

		// Check for unescaped special characters
		if (specialChars.find(*it) != std::string::npos)
		{
			if (it == headerValue.begin())
			{
				return false;
			}
			else if (*(it - 1) != '\\')
			{
				return false;
			}
		}
	}
	return true;
}

/**
 * @brief Checks each header for syntax errors that should trigger an error
 * response from the server.
 *
 * These syntax errors include:
 * - Invalid characters in header name or value
 * - Unescaped special characters in header value
 * - Improper formatting such as:
 *   - Missing the colon (:) between header name and value
 *   - Leading or trailing whitespace in header name
 *
 * NOTE: empty list items are ignored. E.g., HEADER: value1,,value2,,
 *
 * @param headerLine The header line to check.
 * @throws HttpException if there are syntax errors in the header.
 */
void HeaderParser::checkSingleHeader(std::string &headerLine)
{
	std::string headerName;
	std::string headerValue;

	std::string::size_type colonPos = headerLine.find(':');
	if (colonPos == std::string::npos)
	{
		Logger::log(Logger::INFO)
			<< "Header does not contain colon (:). Header: " << headerLine
			<< std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
	headerName = headerLine.substr(0, colonPos);
	headerValue = headerLine.substr(colonPos + 1);

	// Trim leading and trailing whitespace from headerName
	size_t start = headerName.find_first_not_of(" \t");
	size_t end = headerName.find_last_not_of(" \t");
	if (start != 0 || end != headerName.size() - 1)
	{
		Logger::log(Logger::INFO)
			<< "Header name has leading or trailing whitespace. Header name: "
			<< headerName << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}

	// Trim leading and trailing whitespace from headerValue
	start = headerValue.find_first_not_of(" \t");
	end = headerValue.find_last_not_of(" \t");
	if (start != std::string::npos && end != std::string::npos)
	{
		headerValue = headerValue.substr(start, end - start + 1);
	}

	if (!isValidHeaderName(headerName))
	{
		Logger::log(Logger::INFO)
			<< "Header name is malformed. Header name: " << headerName
			<< std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
	if (!isValidHeaderValue(headerValue))
	{
		Logger::log(Logger::INFO)
			<< "Header value is malformed. Header value: " << headerValue
			<< std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
}

/**
 * @brief Checks the raw headers for specific conditions.
 *
 * This function checks for the presence of the Host header, ensures that the
 * Host header appears only once, and verifies that non-repeatable headers do
 * not appear more than once.
 *
 * @param rawHeaders The multimap of raw headers to check.
 * @throws HttpException if there are errors in the headers.
 */
void HeaderParser::checkRawHeaders(
	const std::multimap<std::string, std::string> &rawHeaders
)
{
	bool													hasHost = false;
	std::map<std::string, int>								headerCounts;
	std::multimap<std::string, std::string>::const_iterator it;

	for (std::multimap<std::string, std::string>::const_iterator it
		 = rawHeaders.begin();
		 it != rawHeaders.end();
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
		if (rawHeaders.count(it->first) > 0)
		{
			headerCounts[it->first]++;
			if (headerCounts[it->first] > 1)
			{
				// TODO: turn into method check repeated header or smth
				for (int i = 0; i < REPEATABLEHEADERS_N; ++i)
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
