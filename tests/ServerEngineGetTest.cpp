#include "test.hpp"
#include <chrono>
#include <criterion/criterion.h>
#include <fstream>
#include <sstream>
#include <thread>

Test(ServerEngine, handleGetRequest_FileExists)
{
	// Read the request from getRequest.txt
	std::string requestStr = ft::readFile("getRequest.txt");

	// Parse the request string into an HttpRequest object
	HttpRequest request = RequestParser::parseRequest(requestStr);

	ServerConfig config("test.config");
	config.parseFile(false, false);
	// config.setRootToAllServers("../www/website");

	{
		// Create a ServerEngine object
		ServerEngine serverEngine(config.getAllServersConfig());
		// serverEngine.start();

		// Call the handleGetRequest_method
		std::string response = serverEngine.createResponse(request);

		std::cout << "\n\nTest Response:\n" << response << std::endl;
		// Assert the expected response
		cr_assert(
			response.find("200 OK") != std::string::npos,
			"Expected 200 OK response"
		);
		cr_assert(
			response.find("<!DOCTYPE html>") != std::string::npos,
			"Expected HTML content in response"
		);
	}
}

// Test for handling GET request when file is not found
Test(ServerEngine, handleGetRequest_FileNotFound)
{
	std::this_thread::sleep_for(std::chrono::seconds(1));

	// Read the request from nofileGetRequest.txt
	std::string requestStr = ft::readFile("nofileGetRequest.txt");

	// Parse the request string into an HttpRequest object
	HttpRequest request = RequestParser::parseRequest(requestStr);

	ServerConfig config("test.config");
	config.parseFile(false, false);
	// config.setRootToAllServers("../www/website");

	{
		// Create a ServerEngine object
		ServerEngine serverEngine(config.getAllServersConfig());
		// serverEngine.start();

		// Call the handleGetRequest_method
		std::string response = serverEngine.createResponse(request);

		std::cout << "\n\nTest Response:\n" << response << std::endl;
		// Assert the expected response
		cr_assert(
			response.find("404 Not Found") != std::string::npos,
			"Expected 404 Not Found response"
		);
		cr_assert(
			response.find("<!DOCTYPE html>") != std::string::npos,
			"Expected HTML content in response"
		);
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
}

// Test for handling GET request with unsupported method
Test(ServerEngine, handleGetRequest_NotImplemented)
{
	std::this_thread::sleep_for(std::chrono::seconds(2));

	// Create a request string with an unsupported method (e.g., TRACE)
	std::string requestStr = ft::readFile("traceRequest.txt");

	// Parse the request string into an HttpRequest object
	HttpRequest request = RequestParser::parseRequest(requestStr);

	ServerConfig config("test.config");
	config.parseFile(false, false);
	// config.setRootToAllServers("../www/website");

	{
		// Create a ServerEngine object
		ServerEngine serverEngine(config.getAllServersConfig());
		// serverEngine.start();

		std::cout << "\nTest Method:\n"
				  << request.getMethod() << "\n"
				  << std::endl;

		// Call the handleGetRequest_method
		std::string response = serverEngine.createResponse(request);

		std::cout << "\n\nTest Response:\n" << response << std::endl;
		// Assert the expected response
		cr_assert(
			response.find("501 Not Implemented") != std::string::npos,
			"Expected 501 Not Implemented response"
		);
		cr_assert(
			response.find("<!DOCTYPE html>") != std::string::npos,
			"Expected HTML content in response"
		);
	}
}

Test(ServerEngine, handleGetRequest_Root)
{
	std::this_thread::sleep_for(std::chrono::seconds(3));

	// Read the request from getRequest.txt
	std::string requestStr = ft::readFile("getRequestRoot.txt");

	// Parse the request string into an HttpRequest object
	HttpRequest request = RequestParser::parseRequest(requestStr);

	ServerConfig config("test.config");
	config.parseFile(false, false);
	// config.setRootToAllServers("../www/website");

	{
		// Create a ServerEngine object
		ServerEngine serverEngine(config.getAllServersConfig());
		// serverEngine.start();

		// Call the handleGetRequest_method
		std::string response = serverEngine.createResponse(request);

		std::cout << "\n\nTest Response:\n" << response << std::endl;
		// Assert the expected response
		cr_assert(
			response.find("200 OK") != std::string::npos,
			"Expected 200 OK response"
		);
		cr_assert(
			response.find("<!DOCTYPE html>") != std::string::npos,
			"Expected HTML content in response"
		);
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
}
