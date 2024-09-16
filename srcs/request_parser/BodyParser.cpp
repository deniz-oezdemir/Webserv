#include "request_parser/BodyParser.hpp"
#include <cstdlib>
#include <map>
#include "Logger.hpp"
#include "macros.hpp"
#include "HttpException.hpp"

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
		body->push_back('\n'); // Preserve the newline character
	}

	checkBody_(method, headers, *body);
}

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
		Logger::log(Logger::INFO)
			<< "Content-Length does not match actual body length." << std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}

	if (headers.count("Transfer-Encoding") > 0
		&& headers.at("Transfer-Encoding")[0].find("chunked")
			   != std::string::npos)
	{
		// TODO: Check that the body is in the chunked transfer coding format
	}
}
