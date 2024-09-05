#include "./test.hpp"

Test(HttpRequest, testHttpRequest)
{
	char **argv = new char *[2];
	argv[0] = (char *)"./server";
	argv[1] = (char *)"test.config";
	std::string								method("GET");
	std::string								httpVersion("HTTP/1.1");
	std::string								target("localhost:8080");
	std::multimap<std::string, std::string> headers;
	headers.insert(
		std::pair<std::string, std::string>("Host", "www.example.com")
	);
	headers.insert(
		std::pair<std::string, std::string>("Content-Type", "application/json")
	);
	headers.insert(
		std::pair<std::string, std::string>("Authorization", "Bearer token")
	);
	std::vector<char> body({'h', 'e', 'l', 'l', 'o'});

	// create simple request object
	HttpRequest request(method, httpVersion, target, headers, body);

	cr_assert_str_eq(request.getMethod().c_str(), "GET");
	cr_assert_str_eq(request.getHttpVersion().c_str(), "HTTP/1.1");
	cr_assert_str_eq(request.getTarget().c_str(), "localhost:8080");
	cr_assert_str_eq(
		request.getHeaders().find("Content-Type")->second.c_str(),
		"application/json"
	);
	cr_assert_str_eq(
		request.getHeaders().find("Authorization")->second.c_str(), "Bearer token"
	);
	cr_assert_eq(request.getBody(), body);
}
