#include "request_parser/TokenValidator.hpp"
#include "HttpException.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include "request_parser/HttpHeaders.hpp"
#include <map>
#include <string>
#include <vector>

void TokenValidator::validateTokens(
	// clang-format off
	std::map<std::string, std::vector<std::string> > &headers
	// clang-format on
)
{
	// clang-format off
	typedef std::map<std::string, std::vector<std::string> > HeadersMap;
	// clang-format on
	typedef std::vector<std::string>	TokensVector;
	typedef std::string::const_iterator CharIterator;

	for (HeadersMap::const_iterator header = headers.begin();
		 header != headers.end();
		 ++header)
	{
		for (TokensVector::const_iterator token = header->second.begin();
			 token != header->second.end();
			 ++token)
		{
			for (CharIterator c = token->begin(); c != token->end(); ++c)
			{
				// If character is within the delimeter chars and also not
				// whithin the exceptions allowed for its header
				if (delimeterChars.find(*c) != std::string::npos
					&& headerAcceptedChars.find(header->first)->second.find(*c)
					== std::string::npos)
				{
					Logger::log(Logger::INFO)
						<< "Header contains token with invalid delimeter "
						   "character Header: "
						<< header->first << " | failed token: " << *token
						<< " | invalid delimeter char: " << *c << std::endl;
					throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
				}
			}
		}
	}
}
