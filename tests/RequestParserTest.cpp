#include "./test.hpp"

Test(RequestParser, testStartLine)
{
    // Setup code...
    char** argv = new char*[2];
    argv[0] = (char*)"./server";
    argv[1] = (char*)"test.config";
    std::string method("GET");
    std::string httpVersion("HTTP/1.1");
    std::string target("/index.html");
    std::map<std::string, std::string> headers;
    headers["Host"] = "www.example.com";
    headers["User-Agent"] = "telnet/12.21";
    headers["Accept"] = "*/*";
    std::vector<char> body;

    // Create request string with start line
    std::string requestStr = method + " " + target + " " + httpVersion + "\r\n";

    // Parse request
    HttpRequest request = RequestParser::parseRequest(requestStr);

    // Assertions for start line
    cr_assert_str_eq(request.getMethod().c_str(), method.c_str());
    cr_assert_str_eq(request.getHttpVersion().c_str(), httpVersion.c_str());
    cr_assert_str_eq(request.getTarget().c_str(), target.c_str());
}

Test(RequestParser, testHeaders)
{
    // Setup code...
    char** argv = new char*[2];
    argv[0] = (char*)"./server";
    argv[1] = (char*)"test.config";
    std::string method("GET");
    std::string httpVersion("HTTP/1.1");
    std::string target("/index.html");
    std::map<std::string, std::string> headers;
    headers["Host"] = "www.example.com";
    headers["User-Agent"] = "telnet/12.21";
    headers["Accept"] = "*/*";
    std::vector<char> body;

    // Create request string with headers
    std::string requestStr = method + " " + target + " " + httpVersion + "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        requestStr += it->first + ": " + it->second + "\r\n";
    }

    // Parse request
    HttpRequest request = RequestParser::parseRequest(requestStr);

    // Assertions for headers
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        cr_assert_str_eq(request.getHeaders().at(it->first).c_str(), it->second.c_str());
    }
}

Test(RequestParser, testBody)
{
    // Setup code...
    char** argv = new char*[2];
    argv[0] = (char*)"./server";
    argv[1] = (char*)"test.config";
    std::string method("GET");
    std::string httpVersion("HTTP/1.1");
    std::string target("/index.html");
    std::map<std::string, std::string> headers;
    headers["Host"] = "www.example.com";
    headers["User-Agent"] = "telnet/12.21";
    headers["Accept"] = "*/*";
    std::vector<char> body;

    // Create request string with body
    std::string requestStr = method + " " + target + " " + httpVersion + "\r\n";
    requestStr += "\r\n" + std::string(body.begin(), body.end());

    // Parse request
    HttpRequest request = RequestParser::parseRequest(requestStr);

    // Assertions for body
    cr_assert_eq(request.getBody(), body);
}
