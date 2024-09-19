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
void HeaderParser::parseHeaders(
	std::istream &requestStream,
	// clang-format off
	std::map<std::string, std::vector<std::string> > *headers
	// clang-format on
)
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

		checkSingleHeader_(headerLine);

		std::size_t colonPos = headerLine.find(':');
		if (colonPos != std::string::npos)
		{
			std::string headerName = headerLine.substr(0, colonPos);
			std::string headerValue = headerLine.substr(colonPos + 1);

			// Check if header is accepted by our proram or should be ignored
			if (checkIfHeaderAccepted_(headerName) == false)
			{
				Logger::log(Logger::DEBUG)
					<< "Header ignored: " << headerName << std::endl;
				std::getline(requestStream, headerLine);
				continue;
			}

			std::size_t firstNonSpacePos = headerValue.find_first_not_of(' ');
			if (firstNonSpacePos != std::string::npos)
			{
				headerValue = headerValue.substr(firstNonSpacePos);
			}

			// Trim trailing spaces from header value
			std::size_t lastNonSpacePos = headerValue.find_last_not_of(' ');
			if (lastNonSpacePos != std::string::npos)
			{
				headerValue = headerValue.substr(0, lastNonSpacePos + 1);
			}

			rawHeaders.insert(
				std::pair<std::string, std::string>(headerName, headerValue)
			);
		}
		std::getline(requestStream, headerLine);
	}

	checkRawHeaders_(rawHeaders);
	*headers = unifyHeaders_(rawHeaders);
}

bool HeaderParser::checkIfHeaderAccepted_(std::string &headerName)
{
	for (int i = 0; i < ACCEPTED_HEADERS_N; ++i)
	{
		if (acceptedHeaders[i].find(headerName) != std::string::npos)
		{
			return true;
		}
	}
	return false;
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
bool HeaderParser::isValidHeaderName_(std::string headerName)
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
bool HeaderParser::isValidHeaderValue_(std::string headerValue)
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
void HeaderParser::checkSingleHeader_(std::string &headerLine)
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

	if (!isValidHeaderName_(headerName))
	{
		Logger::log(Logger::INFO)
			<< "Header name is malformed. Header name: " << headerName
			<< std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
	if (!isValidHeaderValue_(headerValue))
	{
		Logger::log(Logger::INFO)
			<< "Header value is malformed. Header value: " << headerValue
			<< std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
}
/**
 * @brief Checks if the given header is allowed to be repeated.
 *
 * This function checks if the specified header is in the list of headers
 * that are allowed to appear more than once in an HTTP request.
 *
 * @param header The name of the header to check.
 * @return true if the header is allowed to be repeated, false otherwise.
 */
bool HeaderParser::checkRepeatedHeaderAllowed_(std::string header)
{
	for (int i = 0; i < REPEATABLE_HEADERS_N; ++i)
	{
		if (repeatableHeaders[i] == header)
		{
			return true;
		}
	}
	return false;
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
void HeaderParser::checkRawHeaders_(
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
				if (!checkRepeatedHeaderAllowed_(it->first))
				{
					Logger::log(Logger::INFO)
						<< "Non-repeatable header appears more than once. "
						   "Header: "
						<< it->first << std::endl;
					throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
				}
			}
		}
	}

	if (!hasHost)
	{
		Logger::log(Logger::INFO) << "Absent Host header." << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
}

/**
 * @brief Splits the header value into a vector of strings based on the
 * separator.
 *
 * This function splits the given header value into multiple values based on the
 * separator, which can be either a comma or a semicolon depending on the header
 * name. It also trims leading and trailing whitespace from each value.
 *
 * @param headerName The name of the header.
 * @param headerValue The value of the header to be split.
 * @return std::vector<std::string> A vector containing the split values.
 */
std::vector<std::string> HeaderParser::splitHeaderValue_(
	const std::string &headerName,
	const std::string &headerValue
)
{
	char separator = ',';
	for (int i = 0; i < SEMICOLON_SEPARATED_N; i++)
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

/**
 * @brief Unifies the headers from a multimap into a map of vectors.
 * 
 * This function converts a multimap of headers into a map where each header name
 * maps to a vector of its values. It handles both single and repeated headers,
 * splitting the values as necessary.
 * 
 * @param multimap The multimap containing the headers.
 * @return std::map<std::string, std::vector<std::string>> A map containing the unified headers.
 */
std::map<std::string, std::vector<std::string> >
HeaderParser::unifyHeaders_(std::multimap<std::string, std::string> multimap)
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
