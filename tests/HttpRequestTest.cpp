#include "./test.hpp"

Test(HttpRequest, HttpRequestBasicTest)
{
	char **argv = new char *[2];
	argv[0] = (char *)"./server";
	argv[1] = (char *)"test.config";
	std::string								method("GET");
	std::string								httpVersion("HTTP/1.1");
	std::string								uri("localhost:8080");
	std::string								host("www.example.com");
	std::string								target(host + '/' + uri);
	std::multimap<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("Host", host));
	headers.insert(
		std::pair<std::string, std::string>("Content-Type", "application/json")
	);
	headers.insert(
		std::pair<std::string, std::string>("Authorization", "Bearer token")
	);
	std::vector<char> body({'h', 'e', 'l', 'l', 'o'});

	// create simple request object
	HttpRequest request(method, httpVersion, uri, headers, body);

	cr_assert_str_eq(request.getMethod().c_str(), "GET");
	cr_assert_str_eq(request.getHttpVersion().c_str(), "HTTP/1.1");
	cr_assert_str_eq(request.getTarget().c_str(), target.c_str());
	cr_assert_str_eq(
		request.getHeaders().at("Content-Type")[0].c_str(), "application/json"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Authorization")[0].c_str(), "Bearer token"
	);
	cr_assert_eq(request.getBody(), body);
}

Test(HttpRequest, testRepeatedHeaders)
{
	char **argv = new char *[2];
	argv[0] = (char *)"./server";
	argv[1] = (char *)"test.config";
	std::string								method("GET");
	std::string								httpVersion("HTTP/1.1");
	std::string								uri("localhost:8080");
	std::string								host("www.example.com");
	std::string								target(host + '/' + uri);
	std::multimap<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("Host", host));
	headers.insert(
		std::pair<std::string, std::string>("Content-Type", "application/json")
	);
	headers.insert(
		std::pair<std::string, std::string>("Content-Type", "text/html")
	); // Repeated header
	headers.insert(
		std::pair<std::string, std::string>("Authorization", "Bearer token")
	);
	headers.insert(
		std::pair<std::string, std::string>("Set-Cookie", "cookie1=value1")
	);
	headers.insert(
		std::pair<std::string, std::string>("Set-Cookie", "cookie2=value2")
	); // Repeated header
	std::vector<char> body({'h', 'e', 'l', 'l', 'o'});

	// create simple request object
	HttpRequest request(method, httpVersion, uri, headers, body);

	cr_assert_str_eq(request.getMethod().c_str(), "GET");
	cr_assert_str_eq(request.getHttpVersion().c_str(), "HTTP/1.1");
	cr_assert_str_eq(request.getTarget().c_str(), target.c_str());
	cr_assert_str_eq(
		request.getHeaders().at("Content-Type")[0].c_str(), "application/json"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Content-Type")[1].c_str(), "text/html"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Authorization")[0].c_str(), "Bearer token"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Set-Cookie")[0].c_str(), "cookie1=value1"
	);
	cr_assert_str_eq(
		request.getHeaders().at("Set-Cookie")[1].c_str(), "cookie2=value2"
	);
	cr_assert_eq(request.getBody(), body);

	delete[] argv;
}

Test(HttpRequest, testHeadersWithMultipleValues)
{
    char **argv = new char *[2];
    argv[0] = (char *)"./server";
    argv[1] = (char *)"test.config";
    std::string method("GET");
    std::string httpVersion("HTTP/1.1");
    std::string uri("localhost:8080");
    std::string host("www.example.com");
    std::string target(host + '/' + uri);
    std::multimap<std::string, std::string> headers;
    headers.insert(std::pair<std::string, std::string>("Host", host));
    headers.insert(std::pair<std::string, std::string>("Content-Type", "application/json"));
    headers.insert(std::pair<std::string, std::string>("Authorization", "Bearer token"));
    headers.insert(std::pair<std::string, std::string>("Accept", "application/json, text/html")); // Multiple values
    std::vector<char> body({'h', 'e', 'l', 'l', 'o'});

    // create simple request object
    HttpRequest request(method, httpVersion, uri, headers, body);

    cr_assert_str_eq(request.getMethod().c_str(), "GET");
    cr_assert_str_eq(request.getHttpVersion().c_str(), "HTTP/1.1");
    cr_assert_str_eq(request.getTarget().c_str(), target.c_str());
    cr_assert_str_eq(
        request.getHeaders().at("Content-Type")[0].c_str(), "application/json"
    );
    cr_assert_str_eq(
        request.getHeaders().at("Authorization")[0].c_str(), "Bearer token"
    );
    cr_assert_str_eq(
        request.getHeaders().at("Accept")[0].c_str(), "application/json"
    );
    cr_assert_str_eq(
        request.getHeaders().at("Accept")[0].c_str(), "text/html"
    );
    cr_assert_eq(request.getBody(), body);

    delete[] argv;
}
