#include "request_parser/HttpHeaders.hpp"

/* WARNING: change length macros if headers are changed */

/* Array of headeres accpeted by webserv. These are the only headers for which
 * syntax checks will be made.
*/
const std::string acceptedHeaders[ACCEPTED_HEADERS_N] = {
	"Host",
	"User-Agent",
	"Accept",
	"Connection",
	"Content-Length",
	"Cookie",
	"Transfer-Encoding",
	"Content-Type"
};


/* Headers allowed to appear more than once per request
*/
const std::string repeatableHeaders[REPEATABLE_HEADERS_N]
	= {"Accept",
	   "Accept-Charset",
	   "Accept-Encoding",
	   "Accept-Language",
	   "Allow",
	   "Cache-Control",
	   "Connection",
	   "Content-Encoding",
	   "Content-Language",
	   "Expect",
	   "Pragma",
	   "Proxy-Authenticate",
	   "TE",
	   "Trailer",
	   "Transfer-Encoding",
	   "Upgrade",
	   "Vary",
	   "Via",
	   "Warning",
	   "WWW-Authenticate"};

/* Headers that use semicolon as separator for values
*/
const std::string semicolonSeparated[SEMICOLON_SEPARATED_N]
	= {"Content-Type",
	   "Set-Cookie",
	   "Cache-Control",
	   "Cookie",
	   "Content-Disposition"};

const std::string delimeterChars = "(),/:;<=>?@[\\]{}\"";

std::map<std::string, std::string> createHeaderAcceptedChars()
{
    std::map<std::string, std::string> headerAcceptedChars;
    headerAcceptedChars["Host"] = ":/";
    headerAcceptedChars["User-Agent"] = "()<>@,;:\\\"/[]?={} \t";
    headerAcceptedChars["Accept"] = "=/;";
    headerAcceptedChars["Connection"] = "";
    headerAcceptedChars["Content-Length"] = "";
    headerAcceptedChars["Content-Type"] = "()<>@,;:\\\"/[]?={} \t";
    headerAcceptedChars["Cookie"] = "=;,";
    return headerAcceptedChars;
}

/* String of tolerated delimiter characters for tokens in each header. If token contains
 * charaters not present in the allowed list, it should raise an error.
 * delimiter characters according to RFC 9110: DQUOTE and "(),/:;<=>?@[\]{}"
*/
const std::map<std::string, std::string> headerAcceptedChars
	= createHeaderAcceptedChars();
