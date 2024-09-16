#include "./test.hpp"
#include <map>
#include <string>
#include <vector>

std::string createRequestString(
	const std::string &method,
	const std::string &uri,
	const std::string &httpVersion,
	// clang-format off
	const std::map<std::string, std::vector<std::string> > &headers,
	// clang-format on
	const std::vector<char> &body
)
{
	std::string requestStr = method + " " + uri + " " + httpVersion + "\r\n";

	// clang-format off
	for (std::map<std::string, std::vector<std::string> >::const_iterator it
		 = headers.begin();
		 it != headers.end();
		 ++it)
	// clang-format on
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
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("www.example.com");
	std::string target(host + uri);
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	headers["Host"].push_back(host);
	headers["User-Agent"].push_back("telnet/12.21");
	headers["Accept"].push_back("*/*");
	std::vector<char> body;

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	// Parse request
	HttpRequest request = RequestParser::parseRequest(requestStr);

	// Assertions for start line
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
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("www.example.com");
	std::string target(host + uri);
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	headers["Host"].push_back(host);
	headers["Content-Type"].push_back("application/json");
	headers["Authorization"].push_back("Bearer token");
	std::vector<char> body;

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);
	std::cout << "request string: " << std::endl << requestStr << std::endl;

	// Parse request
	HttpRequest request = RequestParser::parseRequest(requestStr);

	// Assertions for headers
	// clang-format off
	for (std::map<std::string, std::vector<std::string> >::const_iterator it
		 = headers.begin();
		 it != headers.end();
		 ++it)
	// clang-format on
	{
		// TODO: add test for multiple values in the value vector
		cr_assert_str_eq(
			request.getHeaders().at(it->first)[0].c_str(), it->second[0].c_str()
		);
	}

	delete[] argv;
}

Test(RequestParser, testBody)
{
	// Setup code...
	char **argv = new char *[2];
	argv[0] = (char *)"./server";
	argv[1] = (char *)"test.config";
	std::string method("POST");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/index.html");
	std::string host("www.example.com");
	std::string target(host + uri);
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	headers["Host"].push_back(host);
	headers["User-Agent"].push_back("telnet/12.21");
	headers["Accept"].push_back("*/*");
	headers["Content-Type"].push_back("application/json");

	// Body
	std::string		  bodyStr = "{\n"
								"    \"name\": \"John Doe\",\n"
								"    \"email\": \"john.doe@example.com\",\n"
								"    \"age\": 30\n"
								"	}";
	std::vector<char> body(bodyStr.begin(), bodyStr.end());

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);
	// std::cout << "request string: " << std::endl << requestStr << std::endl;
	std::string reqBodyStr(body.begin(), body.end());

	// Parse request
	HttpRequest request = RequestParser::parseRequest(requestStr);

	// Print the actual body returned by request.getBody()
	std::vector<char> actualBody = request.getBody();
	std::string		  actualBodyStr(actualBody.begin(), actualBody.end());

	// Assertions for body
	cr_assert(
		std::equal(body.begin(), body.end(), request.getBody().begin()),
		"The request body does not match the expected body."
	);

	delete[] argv;
}

Test(RequestParser, testInvalidMethod)
{
	std::string method("INVALID_METHOD");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("www.example.com");
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	std::vector<char> body;

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
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	std::vector<char> body;

	headers["Host"].push_back(host);

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testInvalidURI)
{
	std::string method("GET");
	std::string httpVersion("INVALID_HTTP_VERSION");
	std::string uri("localhost:8080");
	std::string host("www.example.com");
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	std::vector<char> body;

	headers["Host"].push_back(host);

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testMissingHostHeader)
{
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("");
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	std::vector<char> body;

	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testHeadersWithListValues)
{
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string host("");
	std::string uri("/localhost:8080");
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	headers["Host"].push_back("www.example.com");
	headers["Accept"].push_back(
		"text/html,application/xhtml+xml,application/xml"
	);
	headers["Accept-Encoding"].push_back("gzip, deflate, br");
	std::vector<char> body;

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	// Parse request
	HttpRequest request = RequestParser::parseRequest(requestStr);

	// Assertions for headers
	cr_assert_eq(request.getHeaders().at("Accept").size(), 3);
	cr_assert_str_eq(request.getHeaders().at("Accept")[0].c_str(), "text/html");
	cr_assert_str_eq(
		request.getHeaders().at("Accept")[1].c_str(), "application/xhtml+xml"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Accept")[2].c_str(), "application/xml"
	);

	cr_assert_eq(request.getHeaders().at("Accept-Encoding").size(), 3);
	cr_assert_str_eq(
		request.getHeaders().at("Accept-Encoding")[0].c_str(), "gzip"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Accept-Encoding")[1].c_str(), "deflate"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Accept-Encoding")[2].c_str(), "br"
	);
}

/**
 * @brief Test for headers with list values and repeated headers.
 */
Test(RequestParser, testHeadersWithListValuesAndRepeatedHeaders)
{
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("");
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	headers["Host"].push_back("www.example.com");
	headers["Accept"].push_back("text/html");
	headers["Accept"].push_back("application/xhtml+xml,application/xml");
	headers["Accept-Encoding"].push_back("gzip");
	headers["Accept-Encoding"].push_back("deflate, br");
	std::vector<char> body;

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	// Parse request
	HttpRequest request = RequestParser::parseRequest(requestStr);

	// Assertions for headers
	cr_assert_eq(request.getHeaders().at("Accept").size(), 3);
	cr_assert_str_eq(request.getHeaders().at("Accept")[0].c_str(), "text/html");
	cr_assert_str_eq(
		request.getHeaders().at("Accept")[1].c_str(), "application/xhtml+xml"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Accept")[2].c_str(), "application/xml"
	);

	cr_assert_eq(request.getHeaders().at("Accept-Encoding").size(), 3);
	cr_assert_str_eq(
		request.getHeaders().at("Accept-Encoding")[0].c_str(), "gzip"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Accept-Encoding")[1].c_str(), "deflate"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Accept-Encoding")[2].c_str(), "br"
	);
}

/**
 * @brief Test for headers with values separated by semicolon.
 */
Test(RequestParser, testHeadersWithSemicolonSeparatedValues)
{
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("");
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	headers["Cookie"].push_back("name=value; name2=value2; name3=value3");
	std::vector<char> body;

	headers["Host"].push_back("www.example.com"); // Add this line

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	// Parse request
	HttpRequest request = RequestParser::parseRequest(requestStr);

	// Assertions for headers
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

/**
 * @brief Test for repeated headers that are not allowed to be repeated.
 */
Test(RequestParser, testRepeatedHeadersNotAllowed)
{
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("www.example.com");
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	headers["Host"].push_back(host);
	headers["Content-Type"].push_back("application/json");
	headers["Content-Type"].push_back("text/html"); // Repeated header not allowed
	std::vector<char> body;

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

/**
 * @brief Test for a header containing list values but the header is not allowed to have list values.
 */
Test(RequestParser, testHeaderWithListValuesNotAllowed)
{
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("www.example.com");
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	headers["Host"].push_back(host);
	headers["Content-Length"].push_back("123,456"); // List values not allowed
	std::vector<char> body;

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

/**
 * @brief Test for a header that is using the wrong separator for tokens in its values.
 */
Test(RequestParser, testHeaderWithWrongSeparator)
{
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("www.example.com");
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers;
	// clang-format on
	headers["Host"].push_back(host);
	headers["Accept"].push_back("text/html; application/xhtml+xml; application/xml"); // Wrong separator
	std::vector<char> body;

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}
