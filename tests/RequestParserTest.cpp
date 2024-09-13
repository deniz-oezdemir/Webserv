#include "./test.hpp"
#include <map>
#include <vector>
#include <string>

std::string createRequestString(
	const std::string &method,
	const std::string &uri,
	const std::string &httpVersion,
	const std::map<std::string, std::vector<std::string> > &headers,
	const std::vector<char> &body
)
{
	std::string requestStr = method + " " + uri + " " + httpVersion + "\r\n";

	for (std::map<std::string, std::vector<std::string> >::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		for (std::vector<std::string>::const_iterator valIt = it->second.begin(); valIt != it->second.end(); ++valIt)
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
	// Setup code...
	char **argv = new char *[2];
	argv[0] = (char *)"./server";
	argv[1] = (char *)"test.config";
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("www.example.com");
	std::string target(host + uri);
	std::map<std::string, std::vector<std::string> > headers;
	headers["Host"].push_back(host);
	headers["User-Agent"].push_back("telnet/12.21");
	headers["Accept"].push_back("*/*");
	std::vector<char> body;

	// Create request string
	std::string requestStr = createRequestString(method, uri, httpVersion, headers, body);

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
	std::map<std::string, std::vector<std::string> > headers;
	headers["Host"].push_back(host);
	headers["Content-Type"].push_back("application/json");
	headers["Authorization"].push_back("Bearer token");
	std::vector<char> body;

	// Create request string
	std::string requestStr = createRequestString(method, uri, httpVersion, headers, body);
	std::cout << "request string: " << std::endl << requestStr << std::endl;

	// Parse request
	HttpRequest request = RequestParser::parseRequest(requestStr);

	// Assertions for headers
	for (std::map<std::string, std::vector<std::string> >::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		// TODO: add test for multiple values in the value vector
		cr_assert_str_eq(request.getHeaders().at(it->first)[0].c_str(), it->second[0].c_str());
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
	std::map<std::string, std::vector<std::string> > headers;
	headers["Host"].push_back(host);
	headers["User-Agent"].push_back("telnet/12.21");
	headers["Accept"].push_back("*/*");
	headers["Content-Type"].push_back("application/json");

	// Body
	std::string bodyStr = "{\n"
						  "    \"name\": \"John Doe\",\n"
						  "    \"email\": \"john.doe@example.com\",\n"
						  "    \"age\": 30\n"
						  "	}";
	std::vector<char> body(bodyStr.begin(), bodyStr.end());

	// Create request string
	std::string requestStr = createRequestString(method, uri, httpVersion, headers, body);
	// std::cout << "request string: " << std::endl << requestStr << std::endl;
	std::string reqBodyStr(body.begin(), body.end());

	// Parse request
	HttpRequest request = RequestParser::parseRequest(requestStr);

	// Print the actual body returned by request.getBody()
	std::vector<char> actualBody = request.getBody();
	std::string actualBodyStr(actualBody.begin(), actualBody.end());

	// Assertions for body
	cr_assert(std::equal(body.begin(), body.end(), request.getBody().begin()), "The request body does not match the expected body.");

	delete[] argv;
}

Test(RequestParser, testInvalidMethod)
{
	std::string method("INVALID_METHOD");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("www.example.com");
	std::map<std::string, std::vector<std::string> > headers;
	std::vector<char> body;

	std::string requestStr = createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testInvalidHttpVersion)
{
	std::string method("GET");
	std::string httpVersion("INVALID_HTTP_VERSION");
	std::string uri("/localhost:8080");
	std::string host("www.example.com");
	std::map<std::string, std::vector<std::string> > headers;
	std::vector<char> body;

	std::string requestStr = createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testInvalidURI)
{
	std::string method("GET");
	std::string httpVersion("INVALID_HTTP_VERSION");
	std::string uri("localhost:8080");
	std::string host("www.example.com");
	std::map<std::string, std::vector<std::string> > headers;
	std::vector<char> body;

	std::string requestStr = createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

Test(RequestParser, testMissingHostHeader)
{
	std::string method("GET");
	std::string httpVersion("HTTP/1.1");
	std::string uri("/localhost:8080");
	std::string host("");
	std::map<std::string, std::vector<std::string> > headers;
	std::vector<char> body;

	std::string requestStr = createRequestString(method, uri, httpVersion, headers, body);

	cr_assert_throw(RequestParser::parseRequest(requestStr), HttpException);
}

