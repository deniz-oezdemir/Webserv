#include "./test.hpp"

Test(RequestParser, testRequestParser)
{
	char** argv = new char*[2];
	argv[0] = (char*)"./server";
	argv[1] = (char*)"test.config";
	std::string						   method("GET");
	std::string						   httpVersion("HTTP/1.1");
	std::string						   target("localhost:8080");
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "application/json";
	headers["Authorization"] = "Bearer token";
	std::vector<char> body({'h', 'e', 'l', 'l', 'o'});

	std::string requestStr =
		"GET / HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: "
		"curl/7.64.1\r\nAccept: */*\r\n\r\n";

	// create simple request object
	HttpRequest request = RequestParser::parseRequest(requestStr);

	cr_assert_str_eq(request.getMethod().c_str(), "GET");
	// cr_assert_str_eq(request.getHttpVersion().c_str(), "HTTP/1.1");
	// cr_assert_str_eq(request.getTarget().c_str(), "localhost:8080");
	// cr_assert_str_eq(
	// 	request.getHeaders().at("Content-Type").c_str(), "application/json"
	// );
	// cr_assert_str_eq(
	// 	request.getHeaders().at("Authorization").c_str(), "Bearer token"
	// );
	// cr_assert_eq(request.getBody(), body);
}
