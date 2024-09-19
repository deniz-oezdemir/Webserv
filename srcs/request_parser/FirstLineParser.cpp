#include "request_parser/FirstLineParser.hpp"
#include "HttpException.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include <algorithm>

/**
 * @brief Checks the start line of the HTTP request.
 *
 * This function validates the start line of the HTTP request. It ensures that
 * the start line is not empty, removes any trailing carriage return character,
 * and splits the start line into method, URI, and HTTP version. It then calls
 * other functions to validate each of these components.
 *
 * @param startLine The start line of the HTTP request.
 * @param method Pointer to a string to store the HTTP method.
 * @param uri Pointer to a string to store the URI.
 * @param httpVersion Pointer to a string to store the HTTP version.
 *
 * @throws HttpException if the start line is empty or malformed.
 */
void ParseReqFirstLine::checkStartLine(
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
	checkMethod_(*method);
	checkUri(*uri);
	checkHttpVersion(*httpVersion);
}

/**
 * @brief Checks the HTTP method.
 *
 * This function validates the HTTP method against a list of accepted methods.
 *
 * @param method The HTTP method to validate.
 */
void ParseReqFirstLine::checkMethod_(std::string &method)
{
	std::string array[] = HTTP_ACCEPTED_METHODS;
	size_t		arraySize = sizeof(array) / sizeof(std::string);
	if (std::find(array, array + arraySize, method) == array + arraySize)
	{
		Logger::log(Logger::INFO)
			<< "Method not found: " << method << std::endl;
	}
}

/**
 * @brief Checks the URI.
 *
 * This function validates the URI. It ensures that the URI is either '*' or
 * starts with a '/' and contains only valid characters. It also checks for
 * valid percent-encoded characters.
 *
 * @param uri The URI to validate.
 *
 * @throws HttpException if the URI is invalid.
 */
void ParseReqFirstLine::checkUri(std::string &uri)
{
	if (uri == "*")
		return;

	if (uri[0] != '/'
		&& (uri.find("http://") != 0 && uri.find("https://") != 0))
	{
		Logger::log(Logger::INFO) << "Uri is not \'*\' and does not start with "
									 "/, http://, or https://: "
								  << uri << std::endl;
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

/**
 * @brief Checks the HTTP version.
 *
 * This function validates the HTTP version. It ensures that the HTTP version
 * is 'HTTP/1.1'.
 *
 * @param httpVersion The HTTP version to validate.
 *
 * @throws HttpException if the HTTP version is not 'HTTP/1.1'.
 */
void ParseReqFirstLine::checkHttpVersion(std::string &httpVersion)
{
	if (httpVersion != "HTTP/1.1")
	{
		Logger::log(Logger::INFO)
			<< "HTTP version is not \'HTTP/1.1\': " << httpVersion << std::endl;
		throw HttpException(HTTP_501_CODE, HTTP_501_REASON);
	}
}
