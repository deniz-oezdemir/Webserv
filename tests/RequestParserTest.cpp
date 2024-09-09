#include "./test.hpp"

std::string createRequestString(
	const std::string							  &method,
	const std::string							  &uri,
	const std::string							  &httpVersion,
	const std::string							  &host,
	const std::multimap<std::string, std::string> &headers,
	const std::vector<char>						  &body
)
{
	std::string requestStr = method + " " + uri + " " + httpVersion + "\r\n";
	requestStr += "Host: " + host + "\r\n";

	for (std::multimap<std::string, std::string>::const_iterator it
		 = headers.begin();
		 it != headers.end();
		 ++it)
	{
		requestStr += it->first + ": " + it->second + "\r\n";
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
	std::string								method("GET");
	std::string								httpVersion("HTTP/1.1");
	std::string								uri("/localhost:8080");
	std::string								host("www.example.com");
	std::string								target(host + uri);
	std::multimap<std::string, std::string> headers;
	headers.insert(
		std::pair<std::string, std::string>("User-Agent", "telnet/12.21")
	);
	headers.insert(std::pair<std::string, std::string>("Accept", "*/*"));
	std::vector<char> body;

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, host, headers, body);
	std::cout << "request string: " << std::endl << requestStr << std::endl;

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
	std::string								method("GET");
	std::string								httpVersion("HTTP/1.1");
	std::string								uri("/localhost:8080");
	std::string								host("www.example.com");
	std::string								target(host + uri);
	std::multimap<std::string, std::string> headers;
	headers.insert(
		std::pair<std::string, std::string>("Content-Type", "application/json")
	);
	headers.insert(
		std::pair<std::string, std::string>("Authorization", "Bearer token")
	);
	std::vector<char> body;

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, host, headers, body);
	std::cout << "request string: " << std::endl << requestStr << std::endl;

	// Parse request
	HttpRequest request = RequestParser::parseRequest(requestStr);

	// Assertions for headers
	for (std::multimap<std::string, std::string>::const_iterator it
		 = headers.begin();
		 it != headers.end();
		 ++it)
	{
		cr_assert_str_eq(
			request.getHeaders().at(it->first)[0].c_str(), it->second.c_str()
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
	std::string								method("POST");
	std::string								httpVersion("HTTP/1.1");
	std::string								uri("/index.html");
	std::string								host("www.example.com");
	std::string								target(host + uri);
	std::multimap<std::string, std::string> headers;
	headers.insert(
		std::pair<std::string, std::string>("User-Agent", "telnet/12.21")
	);
	headers.insert(std::pair<std::string, std::string>("Accept", "*/*"));
    headers.insert(std::pair<std::string, std::string>("Content-Type", "application/json"));
	
	// Body
    std::string bodyStr = "{\n"
                          "    \"name\": \"John Doe\",\n"
                          "    \"email\": \"john.doe@example.com\",\n"
                          "    \"age\": 30\n"
                          "	}";
    std::vector<char> body(bodyStr.begin(), bodyStr.end());

	// Create request string
	std::string requestStr
		= createRequestString(method, uri, httpVersion, host, headers, body);
	// std::cout << "request string: " << std::endl << requestStr << std::endl;
    std::string reqBodyStr(body.begin(), body.end());
    std::cout << "Request body: " << std::endl << reqBodyStr << std::endl;

	// Parse request
	HttpRequest request = RequestParser::parseRequest(requestStr);
	
    // Print the actual body returned by request.getBody()
    std::vector<char> actualBody = request.getBody();
    std::string actualBodyStr(actualBody.begin(), actualBody.end());
    std::cout << "Actual body: " << std::endl << actualBodyStr << std::endl;

	// Assertions for body
	cr_assert_eq(request.getBody(), body);

	delete[] argv;
}

