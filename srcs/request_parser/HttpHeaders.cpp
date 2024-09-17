#include "request_parser/HttpHeaders.hpp"

const std::string repeatableHeaders[REPEATABLEHEADERS_N]
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

const std::string semicolonSeparated[SEMICOLONSEPARATE_N]
	= {"Content-Type",
	   "Set-Cookie",
	   "Cache-Control",
	   "Cookie",
	   "Content-Disposition"};

std::map<std::string, std::string> createHeaderAcceptedChars()
{
	std::map<std::string, std::string> headerAcceptedChars;
	headerAcceptedChars["Authorization"] = "";
	headerAcceptedChars["Content-Type"] = "";
	headerAcceptedChars["Content-Disposition"] = "\"";
	headerAcceptedChars["Set-Cookie"] = "";
	headerAcceptedChars["Cache-Control"] = "";
	headerAcceptedChars["Cookie"] = "";
	headerAcceptedChars["If-Modified-Since"] = "";
	headerAcceptedChars["If-None-Match"] = "\"";
	headerAcceptedChars["User-Agent"] = "\"(),/:;<=>?@[\\]{}";
	headerAcceptedChars["Referer"] = "\"";
	headerAcceptedChars["Location"] = "\"";
	return headerAcceptedChars;
}

const std::map<std::string, std::string> headerAcceptedChars
	= createHeaderAcceptedChars();
