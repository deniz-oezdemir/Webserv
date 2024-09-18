#include "request_parser/BodyParser.hpp"
#include "HttpException.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include <cstdlib>
#include <map>

/**
 * @brief Parses the body of the HTTP request from the given request stream.
 *
 * This function reads the body from the provided request stream and stores it
 * in the provided body vector. It also preserves newline characters.
 *
 * @param requestStream The input stream containing the HTTP request body.
 * @param method The HTTP method of the request (e.g., GET, POST).
 * @param headers The headers of the HTTP request.
 * @param body A pointer to a vector where the parsed body will be stored.
 */
void BodyParser::parseBody(
	std::istream	  &requestStream,
	const std::string &method,
	// clang-format off
	const std::map<std::string, std::vector<std::string> > &headers,
	// clang-format on
	std::vector<char> *body
)
{

	std::string bodyLine;
	while (std::getline(requestStream, bodyLine))
	{
		body->insert(body->end(), bodyLine.begin(), bodyLine.end());
		if (!requestStream.eof(
			)) // Only add newline if not at the end of the stream
		{
			body->push_back('\n'); // Preserve the newline character
		}
	}

	checkBody_(method, headers, *body);
}

/**
 * @brief Checks the parsed body for specific conditions.
 *
 * This function performs several checks on the parsed body:
 * - Ensures that the body is empty for GET or DELETE requests.
 * - Verifies that the Content-Length header matches the actual body length.
 * - Checks for the presence of the Transfer-Encoding header and ensures that
 *   the body is in the chunked transfer coding format if necessary.
 *
 * @param method The HTTP method of the request (e.g., GET, POST).
 * @param headers The headers of the HTTP request.
 * @param body The parsed body of the HTTP request.
 * @throws HttpException if any of the checks fail.
 */
void BodyParser::checkBody_(
	const std::string &method,
	// clang-format off
	const std::map<std::string, std::vector<std::string> > &headers,
	// clang-format on
	const std::vector<char> &body
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
		&& (unsigned long)std::atol(headers.at("Content-Length")[0].c_str())
			   != body.size())
	{
		Logger::log(Logger::INFO
		) << "Content-Length does not match actual body length. Stated length: "
		  << (unsigned long)std::atol(headers.at("Content-Length")[0].c_str())
		  << " Actual length: " << body.size() << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}

	if (headers.count("Transfer-Encoding") > 0
		&& headers.at("Transfer-Encoding")[0].find("chunked")
			   != std::string::npos)
	{
		// TODO: Check that the body is in the chunked transfer coding format
	}
}
