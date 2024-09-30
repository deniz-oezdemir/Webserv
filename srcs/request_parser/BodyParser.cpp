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
	// GET and DELETE methods should not have a body
	// GET and DELETE methods should not have a Content-Length or
	// Transfer-Encoding header 
	// POST should always have Content-Length header
	if (method == "GET" || method == "DELETE")
	{
		if (headers.count("Content-Length") > 0
			|| headers.count("content-length") > 0
			|| headers.count("Transfer-Encoding") > 0
			|| headers.count("transfer-encoding") > 0)
		{
			Logger::log(Logger::INFO)
				<< "GET or DELETE method should not have Content-Length header "
				   "present. Method:"
				<< method << std::endl;
			throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
		}
		if (!body.empty())
		{
			Logger::log(Logger::INFO)
				<< "Body should be empty for GET or DELETE requests."
				<< std::endl;
			throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
		}
	}
	else if (method == "POST")
	{
		if (headers.count("Content-Length") < 1
			&& headers.count("content-length") < 1
			&& headers.count("Transfer-Encoding") < 1
			&& headers.count("transfer-encoding") < 1)
		{
			Logger::log(Logger::INFO) << "POST method requires Content-Length "
										 "or Transfer-Encoding header."
									  << std::endl;
			throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
		}
	}

	// Check actual body length matches Content-Length header
	if ((headers.count("Content-Length") > 0
		 && (unsigned long)std::atol(headers.at("Content-Length")[0].c_str())
				!= body.size())
		|| (headers.count("content-length") > 0
			&& (unsigned long)std::atol(headers.at("content-length")[0].c_str())
				   != body.size()))
	{
		Logger::log(Logger::INFO
		) << "Content-Length does not match actual body length. Stated length: "
		  << (unsigned long)std::atol(headers.at("Content-Length")[0].c_str())
		  << " Actual length: " << body.size() << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
}
