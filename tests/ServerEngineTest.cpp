#include "ServerEngine.hpp"
#include "HttpRequest.hpp"
#include "RequestParser.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <criterion/criterion.h>

std::string readFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

Test(ServerEngine, handleGetRequest_FileExists)
{
    // Read the request from getRequest.txt
    std::string requestStr = readFile("tests/getRequest.txt");

    // Parse the request string into an HttpRequest object
    HttpRequest request = RequestParser::parseRequest(requestStr);

    // Create a ServerEngine object
    ServerEngine serverEngine;

    // Call the handleGetRequest method
    std::string response = serverEngine.handleGetRequest(request);

    // Assert the expected response
    cr_assert(response.find("200 OK") != std::string::npos, "Expected 200 OK response");
    cr_assert(response.find("<html>") != std::string::npos, "Expected HTML content in response");
}

Test(ServerEngine, handleGetRequest_FileNotFound)
{
    // Read the request from nofileGetRequest.txt
    std::string requestStr = readFile("tests/nofileGetRequest.txt");

    // Parse the request string into an HttpRequest object
    HttpRequest request = RequestParser::parseRequest(requestStr);

    // Create a ServerEngine object
    ServerEngine serverEngine;

    // Call the handleGetRequest method
    std::string response = serverEngine.handleGetRequest(request);

    // Assert the expected response
    cr_assert(response.find("404 Not Found") != std::string::npos, "Expected 404 Not Found response");
    cr_assert(response.find("<html>") != std::string::npos, "Expected HTML content in response");
}

Test(ServerEngine, handleGetRequest_NotImplemented)
{
    // Create a request string with an unsupported method (e.g., TRACE)
    std::string requestStr = readFile("tests/getRequest.txt");

    // Parse the request string into an HttpRequest object
    HttpRequest request = RequestParser::parseRequest(requestStr);

    // Create a ServerEngine object
    ServerEngine serverEngine;

    // Call the handleGetRequest method
    std::string response = serverEngine.handleGetRequest(request);

    // Assert the expected response
    cr_assert(response.find("501 Not Implemented") != std::string::npos, "Expected 501 Not Implemented response");
    cr_assert(response.find("<html>") != std::string::npos, "Expected HTML content in response");
}
