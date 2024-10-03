#include "./test.hpp"
#include <map>
#include <string>
#include <vector>

std::string createRequestString(
	const std::string									  &method,
	const std::string									  &uri,
	const std::string									  &httpVersion,
	const std::map<std::string, std::vector<std::string>> &headers,
	const std::vector<char>								  &body
)
{
	std::string requestStr = method + " " + uri + " " + httpVersion + "\r\n";

	for (std::map<std::string, std::vector<std::string>>::const_iterator it
		 = headers.begin();
		 it != headers.end();
		 ++it)
	{
		for (std::vector<std::string>::const_iterator valIt
			 = it->second.begin();
			 valIt != it->second.end();
			 ++valIt)
		{
			requestStr += it->first + ": " + *valIt + "\r\n";
		}
	}

	requestStr += "\r\n";
	requestStr += std::string(body.begin(), body.end());

	return requestStr;
}

Test(RequestParser, testStartLine)
{
	char **argv = new char *[2];
	argv[0] = (char *)"./server";
	argv[1] = (char *)"test.config";
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/localhost:8080");
	std::string										host("www.example.com");
	std::string										target(host + uri);
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back(host);
	headers["User-Agent"].push_back("telnet/12.21");
	headers["Accept"].push_back("*/*");
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	HttpRequest request = RequestParser::parseRequest(requestStr);

	cr_assert_str_eq(request.getMethod().c_str(), method.c_str());
	cr_assert_str_eq(request.getHttpVersion().c_str(), httpVersion.c_str());
	cr_assert_str_eq(request.getTarget().c_str(), target.c_str());

	delete[] argv;
}

Test(RequestParser, testHeaders)
{
	char **argv = new char *[2];
	argv[0] = (char *)"./server";
	argv[1] = (char *)"test.config";
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/localhost:8080");
	std::string										host("www.example.com");
	std::string										target(host + uri);
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back(host);
	headers["User-Agent"].push_back("telnet/12.21");
	headers["Accept"].push_back("*/*");
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);
	std::cout << "request string: " << std::endl << requestStr << std::endl;

	HttpRequest request = RequestParser::parseRequest(requestStr);

	for (std::map<std::string, std::vector<std::string>>::const_iterator it
		 = headers.begin();
		 it != headers.end();
		 ++it)
	{
		cr_assert_str_eq(
			request.getHeaders().at(it->first)[0].c_str(), it->second[0].c_str()
		);
	}

	delete[] argv;
}

Test(RequestParser, testBody)
{
	char **argv = new char *[2];
	argv[0] = (char *)"./server";
	argv[1] = (char *)"test.config";
	std::string										method("POST");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/index.html");
	std::string										host("www.example.com");
	std::string										target(host + uri);
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back(host);
	headers["User-Agent"].push_back("telnet/12.21");
	headers["Accept"].push_back("*/*");

	std::string		  bodyStr = "{\n"
								"    \"name\": \"John Doe\",\n"
								"    \"email\": \"john.doe@example.com\",\n"
								"    \"age\": 30\n"
								"}";
	std::vector<char> body(bodyStr.begin(), bodyStr.end());

	// Calculate the correct Content-Length using stringstream
	std::stringstream ss;
	ss << body.size();
	headers["Content-Length"].push_back(ss.str());

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	HttpRequest request = RequestParser::parseRequest(requestStr);

	std::vector<char> actualBody = request.getBody();
	std::string		  actualBodyStr(actualBody.begin(), actualBody.end());

	cr_assert(
		std::equal(body.begin(), body.end(), request.getBody().begin()),
		"The request body does not match the expected body."
	);

	delete[] argv;
}

Test(RequestParser, testBodyLengthZero)
{
	char **argv = new char *[2];
	argv[0] = (char *)"./server";
	argv[1] = (char *)"test.config";
	std::string										method("POST");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/index.html");
	std::string										host("www.example.com");
	std::string										target(host + uri);
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back(host);
	headers["User-Agent"].push_back("telnet/12.21");
	headers["Accept"].push_back("*/*");

	std::string		  bodyStr;
	std::vector<char> body(bodyStr.begin(), bodyStr.end());

	// Calculate the correct Content-Length using stringstream
	std::stringstream ss;
	ss << body.size();
	headers["Content-Length"].push_back(ss.str());

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	HttpRequest request = RequestParser::parseRequest(requestStr);

	std::vector<char> actualBody = request.getBody();
	std::string		  actualBodyStr(actualBody.begin(), actualBody.end());

	cr_assert(
		std::equal(body.begin(), body.end(), request.getBody().begin()),
		"The request body does not match the expected body."
	);

	delete[] argv;
}

Test(RequestParser, testInvalidMethod)
{
	std::string										method("INVALID_METHOD");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/localhost:8080");
	std::string										host("www.example.com");
	std::map<std::string, std::vector<std::string>> headers;
	std::vector<char>								body;

	headers["Accept"].push_back("");

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testInvalidHttpVersion)
{
	std::string method("GET");
	std::string httpVersion("INVALID_HTTP_VERSION");
	std::string uri("/localhost:8080");
	std::string host("www.example.com");
	std::map<std::string, std::vector<std::string>> headers;
	std::vector<char>								body;

	headers["Host"].push_back(host);

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testInvalidURI)
{
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("ihttps://localhost:8080");
	std::string host("www.example.com");
	std::map<std::string, std::vector<std::string>> headers;
	std::vector<char>								body;

	headers["Host"].push_back(host);

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testMissingHostHeader)
{
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/localhost:8080");
	std::string										host("");
	std::map<std::string, std::vector<std::string>> headers;
	std::vector<char>								body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testHeadersWithListValues)
{
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										host("");
	std::string										uri("/localhost:8080");
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back("www.example.com");
	headers["Accept"].push_back(
		"text/html,application/xhtml+xml,application/xml"
	);
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	HttpRequest request = RequestParser::parseRequest(requestStr);

	cr_assert_eq(request.getHeaders().at("Accept").size(), 3);
	cr_assert_str_eq(request.getHeaders().at("Accept")[0].c_str(), "text/html");
	cr_assert_str_eq(
		request.getHeaders().at("Accept")[1].c_str(), "application/xhtml+xml"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Accept")[2].c_str(), "application/xml"
	);
}

Test(RequestParser, testHeadersWithListValuesAndRepeatedHeaders)
{
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/localhost:8080");
	std::string										host("");
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back("www.example.com");
	headers["Accept"].push_back("text/html");
	headers["Accept"].push_back("application/xhtml+xml,application/xml");
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	HttpRequest request = RequestParser::parseRequest(requestStr);

	cr_assert_eq(request.getHeaders().at("Accept").size(), 3);
	cr_assert_str_eq(request.getHeaders().at("Accept")[0].c_str(), "text/html");
	cr_assert_str_eq(
		request.getHeaders().at("Accept")[1].c_str(), "application/xhtml+xml"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Accept")[2].c_str(), "application/xml"
	);
}

Test(RequestParser, testHeadersWithSemicolonSeparatedValues)
{
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/localhost:8080");
	std::string										host("");
	std::map<std::string, std::vector<std::string>> headers;
	headers["Cookie"].push_back("name=value; name2=value2; name3=value3");
	std::vector<char> body;

	headers["Host"].push_back("www.example.com");

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	HttpRequest request = RequestParser::parseRequest(requestStr);

	cr_assert_eq(request.getHeaders().at("Cookie").size(), 3);
	cr_assert_str_eq(
		request.getHeaders().at("Cookie")[0].c_str(), "name=value"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Cookie")[1].c_str(), "name2=value2"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Cookie")[2].c_str(), "name3=value3"
	);
}

Test(RequestParser, testRepeatedHeadersNotAllowed)
{
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/localhost:8080");
	std::string										host("www.example.com");
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back(host);
	headers["Content-Length"].push_back("123");
	headers["Content-Length"].push_back("456"); // Repeated header not allowed
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testHeaderWithListValuesNotAllowed)
{
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/localhost:8080");
	std::string										host("www.example.com");
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back(host);
	headers["Content-Length"].push_back("123,456"); // List values not allowed
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testHeaderWithWrongSeparator)
{
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/localhost:8080");
	std::string										host("www.example.com");
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back(host);
	headers["Accept"].push_back(
		"text/html; application/xhtml+xml;@ application/xml"
	); // Wrong separator
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testIgnoredHeader)
{
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										host("");
	std::string										uri("/localhost:8080");
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back("www.example.com");
	headers["Proxy-Authenticate"].push_back("testcontent");
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	HttpRequest request = RequestParser::parseRequest(requestStr);

	cr_assert_eq(request.getHeaders().count("Proxy-Authenticate"), 0);
}

Test(RequestParser, testPortGetter)
{
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										host("");
	std::string										uri("/");
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back("www.example.com:8080");
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	HttpRequest request = RequestParser::parseRequest(requestStr);

	cr_assert_eq(request.getPort(), 8080);
}

Test(RequestParser, testCookieParsing)
{
	char **argv = new char *[2];
	argv[0] = (char *)"./server";
	argv[1] = (char *)"test.config";
	std::string										method("GET");
	std::string										httpVersion("HTTP/1.1");
	std::string										uri("/localhost:8080");
	std::string										host("www.example.com");
	std::string										target(host + uri);
	std::map<std::string, std::vector<std::string>> headers;
	headers["Host"].push_back(host);
	headers["User-Agent"].push_back("telnet/12.21");
	headers["Accept"].push_back("*/*");
	headers["Cookie"].push_back("sessionId=abc123");
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	HttpRequest request = RequestParser::parseRequest(requestStr);

	cr_assert_str_eq(request.getMethod().c_str(), method.c_str());
	cr_assert_str_eq(request.getHttpVersion().c_str(), httpVersion.c_str());
	cr_assert_str_eq(request.getTarget().c_str(), target.c_str());
	cr_assert(request.hasCookie(), "Request should have a cookie");
	cr_assert_str_eq(request.getCookie().c_str(), "sessionId=abc123");

	delete[] argv;
}
