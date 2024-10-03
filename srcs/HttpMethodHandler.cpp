#include "HttpMethodHandler.hpp"
#include "HttpErrorHandler.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

std::string HttpMethodHandler::handleRequest(
	HttpRequest const &request,
	Server const	  &server,
	std::string const &method
)
{
	if (method == "GET")
	{
		Logger::log(Logger::DEBUG)
			<< "Handling GET request: " << request.getUri() << std::endl;
		return handleGetRequest_(request, server);
	}
	else if (method == "POST")
	{
		Logger::log(Logger::DEBUG)
			<< "Handling POST request: " << request.getUri() << std::endl;
		return handlePostRequest_(request, server);
	}
	else if (method == "DELETE")
	{
		Logger::log(Logger::DEBUG)
			<< "Handling DELETE request: " << request.getUri() << std::endl;
		return handleDeleteRequest_(request, server);
	}
	return HttpErrorHandler::getErrorPage(501, true);
}

std::string HttpMethodHandler::handleGetRequest_(
	const HttpRequest &request,
	Server const	  &server
)
{
	std::string uri = request.getUri();
	bool		keepAlive = request.getKeepAlive();

	// clang-format off
	std::map<std::string, std::vector<std::string> > location; // clang-format on
	if (server.isThisLocation(uri))
		location = server.getThisLocation(uri);
	else
	{
		return HttpErrorHandler::getErrorPage(404, keepAlive);
	}

	// Check for redirections
	std::string redirection = handleRedirection_(location, keepAlive);
	if (!redirection.empty())
		return redirection;
	// Check for authorized methods
	if (!validateMethod_(location, "GET"))
		return HttpErrorHandler::getErrorPage(405, keepAlive);
	// Check for max body size
	if (!validateBodySize_(location, request, server))
		return HttpErrorHandler::getErrorPage(413, keepAlive);

	std::string filepath = getFilePath_(uri, location, server);
	std::string rootdir = getRootDir_(location, server);

	if (isCgiRequest_(location, uri))
		return handleCgiRequest_(
			filepath, getCgiInterpreter_(location), request, keepAlive
		);
	// Check if the request is for a directory and handle autoindex
	if (isDirectory_(filepath))
	{
		if (isAutoIndexEnabled_(location))
			return handleAutoIndex_(rootdir, uri, keepAlive);
		// Search for index file in the directory
		filepath = findIndexFile_(filepath, location, server);
	}

	// Check if the file exists and return the response
	return createFileGetResponse_(filepath, rootdir, server, keepAlive);
}

// TODO: implement check for file/directory, coordinate with Seba
std::string HttpMethodHandler::handleDeleteRequest_(
	const HttpRequest &request,
	Server const	  &server
)
{
	(void)request;
	(void)server;

	std::string uri = request.getUri();
	bool		keepAlive = request.getKeepAlive();

	// clang-format off
	std::map<std::string, std::vector<std::string> > location; // clang-format on
	if (server.isThisLocation(uri))
		location = server.getThisLocation(uri);
	else
	{
		return HttpErrorHandler::getErrorPage(404, keepAlive);
	}

	// Check for redirections
	std::string redirection = handleRedirection_(location, keepAlive);
	if (!redirection.empty())
		return redirection;
	// Check for authorized methods
	if (!validateMethod_(location, "GET"))
		return HttpErrorHandler::getErrorPage(405, keepAlive);
	// Check for max body size
	if (!validateBodySize_(location, request, server))
		return HttpErrorHandler::getErrorPage(413, keepAlive);

	// TODO: replace filepath with actual filepath when Seba added
	// getilePath to config
	std::string rootdir = getRootDir_(location, server);
	std::string filepath = rootdir; // getFilePath_(uri, location, server);

	if (isCgiRequest_(location, uri))
		return handleCgiRequest_(
			filepath, getCgiInterpreter_(location), request, keepAlive
		);

	return createDeleteResponse_(filepath, rootdir, server, keepAlive);
}

std::string HttpMethodHandler::handlePostRequest_(
	const HttpRequest &request,
	Server const	  &server
)
{
	std::string uri = request.getUri();
	bool		keepAlive = request.getKeepAlive();

	// clang-format off
	std::map<std::string, std::vector<std::string> > location; // clang-format on
	if (server.isThisLocation(uri))
		location = server.getThisLocation(uri);
	else
	{
		return HttpErrorHandler::getErrorPage(404, keepAlive);
	}

	// Check for redirections
	std::string redirection = handleRedirection_(location, keepAlive);
	if (!redirection.empty())
		return redirection;
	// Check for authorized methods
	if (!validateMethod_(location, "POST"))
		return HttpErrorHandler::getErrorPage(405, keepAlive);
	// Check for max body size
	if (!validateBodySize_(location, request, server))
		return HttpErrorHandler::getErrorPage(413, keepAlive);

	// TODO: replace uploadpath with actual uploadpath when Seba added
	// getUploadPath to config
	std::string rootdir = getRootDir_(location, server);
	std::string uploadpath = rootdir; // getUploadPath_(uri, location, server);
	std::string pyscriptpath = rootdir + uri;

	Logger::log(Logger::INFO) << "Received POST request for URI: " << uri << std::endl;
	if (!isCgiRequest_(location, uri)) //TODO: Deniz reverse with Seba! Seba how should this work? i only set it to ! to test the cgi
	{
		Logger::log(Logger::INFO) << "Handling CGI request" << std::endl;
		return handleCgiRequest_(pyscriptpath, "/usr/bin/python3"/*getCgiInterpreter_(location)*/, request, keepAlive);
	} else {
		return createFilePostResponse_(
			request, uploadpath, rootdir, server, keepAlive
		);
	}
}

// clang-format off
std::string HttpMethodHandler::handleRedirection_(
	std::map<std::string, std::vector<std::string> > const &location,
	bool const											  &keepAlive
)
{
	std::map<std::string, std::vector<std::string> >::const_iterator it
		= location.find("return"); // clang-format on
	if (it != location.end() && !it->second.empty())
		return handleReturnDirective_(it->second, keepAlive);
	return "";
}

std::string HttpMethodHandler::handleReturnDirective_(
	std::vector<std::string> const &returnDirective,
	bool const					   &keepAlive
)
{
	HttpResponse response;
	int			 statusCode = 301;

	if (returnDirective.size() == 2)
	{
		statusCode = std::atoi(returnDirective[0].c_str());
		response.setHeader("Location", returnDirective[1]);
	}
	else if (returnDirective.size() == 1)
	{
		response.setHeader("Location", returnDirective[0]);
	}

	Logger::log(Logger::DEBUG)
		<< "Handling return directive: [" << statusCode << "] "
		<< ft::getStatusCodeReason(statusCode)
		<< "To: " << response.getHeader("Location") << std::endl;

	response.setStatusCode(statusCode);
	response.setReasonPhrase(ft::getStatusCodeReason(statusCode));
	response.setHeader("Server", SERVER_NAME);
	response.setHeader("Date", ft::createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	response.setHeader("Content-Length", "0");
	if (keepAlive)
		response.setHeader("Connection", "keep-alive");
	else
		response.setHeader("Connection", "close");

	return response.toString();
}

// clang-format off
bool HttpMethodHandler::validateMethod_(
	std::map<std::string, std::vector<std::string> > const &location,
	std::string const									  &method
)
{
	std::map<std::string, std::vector<std::string> >::const_iterator it
		= location.find("limit_except"); // clang-format on

	if (it != location.end() && !it->second.empty())
	{
		if (std::find(it->second.begin(), it->second.end(), method)
			== it->second.end())
			return false;
	}
	return true;
}

// clang-format off
bool HttpMethodHandler::validateBodySize_(
	std::map<std::string, std::vector<std::string> > const &location,
	HttpRequest	const								  &request,
	Server const									  &server
)
{
	std::map<std::string, std::vector<std::string> >::const_iterator it
		= location.find("client_body_size"); // clang-format on
	//
	if (it != location.end() && !it->second.empty())
	{
		if (request.getBody().size() > ft::stringToULong(it->second[0]))
			return false;
	}
	else if (request.getBody().size() > server.getClientMaxBodySize())
		return false;
	return true;
}

// clang-format off
std::string HttpMethodHandler::getFilePath_(
	std::string const									  &uri,
	std::map<std::string, std::vector<std::string> > const &location,
	Server const										  &server
) // clang-format on
{
	std::string rootdir = location.find("root") != location.end()
									&& !location.at("root").empty()
								? location.at("root")[0]
								: server.getRoot();
	return rootdir + uri;
}

// clang-format off
std::string HttpMethodHandler::getRootDir_(
	std::map<std::string, std::vector<std::string> > const &location,
	Server const										  &server
) // clang-format on
{
	std::string rootdir = location.find("root") != location.end()
									&& !location.at("root").empty()
								? location.at("root")[0]
								: server.getRoot();
	return rootdir;
}

// clang-format off
bool HttpMethodHandler::isCgiRequest_(
	std::map<std::string, std::vector<std::string> > const &location,
	std::string const									  &uri
)
{
	std::map<std::string, std::vector<std::string> >::const_iterator it
		= location.find("cgi"); // clang-format on

	if (it != location.end() && !it->second.empty())
	{
		std::string cgiExtension = it->second[0]; // ".py"
		return uri.find(cgiExtension) != std::string::npos;
	}
	return false;
}

// clang-format off
std::string HttpMethodHandler::getCgiInterpreter_(
	std::map<std::string, std::vector<std::string> > const &location
) // clang-format on
{
	return location.find("cgi")->second[1]; // "/usr/bin/python3"
}

std::string HttpMethodHandler::handleCgiRequest_(
	std::string const &filepath,
	std::string const &interpreter,
	HttpRequest const &request,
	bool const		  &keepAlive
)
{
	Logger::log(Logger::INFO) << "Filepath: " << filepath << std::endl;
	Logger::log(Logger::INFO) << "Interpreter: " << interpreter << std::endl;

	int pipefd[2];
	if (pipe(pipefd) == -1)
	{
		Logger::log(Logger::ERROR, true) << "Pipe creation failed" << std::endl;
		return HttpErrorHandler::getErrorPage(500);
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		Logger::log(Logger::ERROR, true) << "Fork failed" << std::endl;
		return HttpErrorHandler::getErrorPage(500);
	}
	else if (pid == 0)
	{
		//child process
		dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to the read end of the pipe to receive body from parent process
		close(pipefd[0]);

		std::string cgiDir = ft::getDirectory(filepath);
		if (chdir(cgiDir.c_str()) == -1)
		{
			Logger::log(Logger::ERROR, true)
				<< "Failed to change directory to: " << cgiDir << std::endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			Logger::log(Logger::INFO, true)
			<< "Changed directory to: " << cgiDir << std::endl;
		}

		std::vector<std::string> envVariables;
		envVariables.push_back("GATEWAY_INTERFACE=CGI/1.1");
		envVariables.push_back("SERVER_PROTOCOL=HTTP/1.1");
		envVariables.push_back("REQUEST_METHOD=" + request.getMethod());
		envVariables.push_back("SCRIPT_FILENAME=" + filepath);
		envVariables.push_back("CONTENT_LENGTH=" + std::to_string(request.getBody().size()));

		std::map<std::string, std::vector<std::string>> headers = request.getHeaders();
		for (const auto &header : headers) {
			for (const auto &value : header.second) {
				Logger::log(Logger::DEBUG) << "Debug Headers: " << header.first << ": " << value << std::endl;
				envVariables.push_back(header.first + "=" + value);
			}
		}

		// There are two values for content-type in the header, combine them for the CONTENT_TYPE=
		std::map<std::string, std::vector<std::string>> headers2 = request.getHeaders();
		auto contentTypeIt = headers2.find("Content-Type");
		if (contentTypeIt != headers2.end() && !contentTypeIt->second.empty()) {
			std::string combinedContentType = contentTypeIt->second[0];
			for (size_t i = 1; i < contentTypeIt->second.size(); ++i) {
				combinedContentType += "; " + contentTypeIt->second[i];
			}
			envVariables.push_back("CONTENT_TYPE=" + combinedContentType);
		}

		envVariables.push_back("PATH_INFO=" + request.getUri()); //is upload path for post request

		std::vector<char *> envp;
		for (std::vector<std::string>::iterator it = envVariables.begin();
			 it != envVariables.end();
			 ++it)
		{
			envp.push_back(const_cast<char *>(it->c_str()));
		}
		envp.push_back(NULL);

		Logger::log(Logger::INFO) << "Filepath before argv: " << filepath << std::endl;

	//TODO: Deniz fix hardcoded path with Seba
		char *argv[]
			= {const_cast<char *>(interpreter.c_str()),
				 const_cast<char *>("/home/denizozd/Webserv/www/instagram-clone/cgi/post.py"), //filepath.c_str()), //reverse
				 NULL};

		dup2(pipefd[1], STDOUT_FILENO);

		if (execve(interpreter.c_str(), argv, &envp[0]) == -1) {
			Logger::log(Logger::ERROR, true) << "Failed to execute CGI script: " << filepath << std::endl;
			close(pipefd[1]);
		}

		close(pipefd[1]);
		exit(EXIT_FAILURE);
	}
	else
	{
		// parent process

		// Write the request body to the pipe for the child process
		const std::vector<char> &requestBody = request.getBody();
		write(pipefd[1], requestBody.data(), requestBody.size());
		close(pipefd[1]); // Close the write end of the pipe after writing

		int status;
		waitpid(pid, &status, 0);
		if (status != 0)
		{
			Logger::log(Logger::ERROR, true)
				<< "CGI script execution failed" << std::endl;
			return HttpErrorHandler::getErrorPage(500);
		}

		char			  buffer[1024];
		std::stringstream output;
		ssize_t			  bytesRead;

		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
			output.write(buffer, bytesRead);

		close(pipefd[0]); // Close the read end of the pipe after reading

		Logger::log(Logger::DEBUG) << "CGI Output: " << output.str() << std::endl;

		HttpResponse response;
		response.setStatusCode(200);
		response.setReasonPhrase("OK");
		response.setHeader("Server", SERVER_NAME);
		response.setHeader("Date", ft::createTimestamp());
		response.setHeader("Content-Type", "text/html; charset=UTF-8");
		response.setHeader("Content-Length", ft::toString(output.str().size()));
		if (keepAlive)
			response.setHeader("Connection", "keep-alive");
		else
			response.setHeader("Connection", "close");
		response.setBody(output.str());

		return response.toString();
	}
}

bool HttpMethodHandler::isDirectory_(std::string const &filepath)
{
	struct stat fileStat;
	if (stat(filepath.c_str(), &fileStat) == 0)
		return S_ISDIR(fileStat.st_mode);
	return false;
}

// clang-format off
bool HttpMethodHandler::isAutoIndexEnabled_(
	const std::map<std::string, std::vector<std::string> > &location
) // clang-format on
{
	return location.find("autoindex") != location.end()
			 && location.at("autoindex")[0] == "on";
}

std::string HttpMethodHandler::handleAutoIndex_(
	std::string const &root,
	std::string const &uri,
	bool const		  &keepAlive
)
{
	Logger::log(Logger::DEBUG)
		<< "Handling auto index on: " << root << uri << std::endl;

	std::string body = generateAutoIndexPage_(root, uri);

	HttpResponse response;
	response.setStatusCode(200);
	response.setReasonPhrase("OK");
	response.setHeader("Server", SERVER_NAME);
	response.setHeader("Date", ft::createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	response.setHeader("Content-Length", ft::toString(body.size()));
	if (keepAlive)
		response.setHeader("Connection", "keep-alive");
	else
		response.setHeader("Connection", "close");
	response.setBody(body);
	return response.toString();
}

std::string HttpMethodHandler::generateAutoIndexPage_(
	std::string const &root,
	std::string const &uri
)
{
	std::stringstream html;

	html << "<!DOCTYPE html>\n<html>\n<head><title>Index of " << uri
		 << "</title></head>\n<body><h1>Index of " << uri << "</h1>\n";
	html << "<ul>\n";

	if (uri != "/")
		html << "<li><a href=\"" << uri + "../\">Parent Directory</a></li>\n";

	DIR *dir = opendir((root + uri).c_str());
	if (dir == NULL)
	{
		Logger::log(Logger::ERROR, true)
			<< "Failed to open directory: " << root + uri << std::endl;
		return HttpErrorHandler::getErrorPage(404, true);
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string filename(entry->d_name);

		// Skip "." and ".." directories
		if (filename == "." || filename == "..")
			continue;

		std::string fullPath = root + uri + "/" + filename;

		// Check if it is a directory
		struct stat fileStat;
		if (stat(fullPath.c_str(), &fileStat) == -1)
		{
			Logger::log(Logger::ERROR, true)
				<< "Failed to get file stats: " << fullPath << "Error: ["
				<< errno << "] " << strerror(errno) << std::endl;
			continue;
		}
		else if (S_ISDIR(fileStat.st_mode))
			filename += "/";

		// Generate links for each file/directory
		html << "<li><a href=\"" << uri << filename << "\">" << filename
			 << "</a></li>\n";
	}
	closedir(dir);
	html << "</ul>\n</body>\n</html>\n";

	return html.str();
}

// clang-format off
std::string HttpMethodHandler::findIndexFile_(
	const std::string									  &filepath,
	const std::map<std::string, std::vector<std::string> > &location,
	const Server										  &server
) // clang-format on
{
	std::vector<std::string> indexFiles
		= location.find("index") != location.end()
					&& !location.at("index").empty()
				? location.at("index")
				: server.getIndex();

	struct stat fileStat;
	for (std::vector<std::string>::const_iterator it = indexFiles.begin();
		 it != indexFiles.end();
		 ++it)
	{
		std::string indexFilePath = filepath + "/" + *it;
		if (stat(indexFilePath.c_str(), &fileStat) == 0
			&& S_ISREG(fileStat.st_mode))
		{
			return indexFilePath;
		}
	}
	return filepath;
}

std::string HttpMethodHandler::createFileGetResponse_(
	std::string const &filepath,
	std::string const &rootdir,
	Server const	  &server,
	bool const		  &keepAlive
)
{
	HttpResponse  response;
	std::ifstream file(filepath);

	if (file.is_open())
	{
		Logger::log(Logger::DEBUG) << "Handling GET: file opened" << std::endl;

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string body = buffer.str();

		response.setStatusCode(200);
		response.setReasonPhrase("OK");
		response.setHeader("Server", SERVER_NAME);
		response.setHeader("Date", ft::createTimestamp());
		response.setHeader("Content-Type", ft::getMimeType(filepath));
		response.setHeader("Content-Length", std::to_string(body.size()));
		if (keepAlive)
			response.setHeader("Connection", "keep-alive");
		else
			response.setHeader("Connection", "close");
		response.setBody(body);
	}
	else
	{
		Logger::log(Logger::DEBUG)
			<< "Handling GET: file not found" << std::endl;
		std::string errorResponse
			= HttpErrorHandler::getErrorPage(server, 404, rootdir, keepAlive);
		if (!errorResponse.empty())
			return errorResponse;
		else
			return HttpErrorHandler::getErrorPage(404, keepAlive);
	}

	Logger::log(Logger::DEBUG) << "Handling GET: responding" << std::endl;
	return response.toString();
}

std::string HttpMethodHandler::createFilePostResponse_(
	HttpRequest const &request,
	std::string const &uploadpath,
	std::string const &rootdir,
	Server const	  &server,
	bool const		  &keepAlive
)
{
	(void)rootdir;
	(void)server;

	HttpResponse response;

	// Open the file for writing
	std::string	  uploadpathtmp = "./" + uploadpath + "/dummyfile";
	std::ofstream outFile;
	outFile.open(uploadpathtmp.c_str());
	if (!outFile)
	{
		Logger::log(Logger::ERROR)
			<< "Handling Post: failed to open file for writing: "
			<< uploadpathtmp << std::endl;
		return HttpErrorHandler::getErrorPage(500, keepAlive);
	}

	// Write the request body to the file
	const std::vector<char> &requestBody = request.getBody();
	outFile.write(requestBody.data(), requestBody.size());
	if (!outFile)
	{
		Logger::log(Logger::ERROR)
			<< "handling Post: failed to write to file: " << uploadpathtmp
			<< std::endl;
		return HttpErrorHandler::getErrorPage(500, keepAlive);
	}

	outFile.close();

	// Generate a success response
	response.setStatusCode(200);
	response.setReasonPhrase("OK");
	response.setHeader("Server", SERVER_NAME);
	response.setHeader("Date", ft::createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	std::string responseBody = "<h1>File Uploaded Successfully</h1>\n";
	response.setHeader("Content-Length", std::to_string(responseBody.size()));
	if (keepAlive)
		response.setHeader("Connection", "keep-alive");
	else
		response.setHeader("Connection", "close");
	response.setBody(responseBody);

	Logger::log(Logger::DEBUG) << "Handling POST: responding" << std::endl;
	return response.toString();
}

std::string HttpMethodHandler::createDeleteResponse_(
	const std::string &filepath,
	const std::string &rootdir,
	const Server	  &server,
	bool			   keepAlive
)
{
	(void)rootdir;
	(void)server;

	std::string	 body;
	HttpResponse response;

	std::string filepathtmp = "./" + filepath + "/dummyfile";

	if (remove(filepathtmp.c_str()) == 0)
	{
		response.setStatusCode(200);
		response.setReasonPhrase("OK");
		body = "<h1>File Deleted Successfully</h1>\n";
	}
	else
	{
		response.setStatusCode(404);
		response.setReasonPhrase("Not Found");
		body = "<h1>File Not Found</h1>\n";
	}

	response.setHeader("Server", SERVER_NAME);
	response.setHeader("Date", ft::createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	response.setHeader("Content-Length", std::to_string(body.size()));
	if (keepAlive)
		response.setHeader("Connection", "keep-alive");
	else
		response.setHeader("Connection", "close");
	response.setBody(body);

	Logger::log(Logger::DEBUG) << "Handling DELETE: responding" << std::endl;

	return response.toString();
}
