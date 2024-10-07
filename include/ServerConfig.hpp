#pragma once

#include <fstream>
#include <map>
#include <stack>
#include <string>
#include <vector>

#include "ConfigValue.hpp"

/**
 * @class ServerConfig
 * @brief ServerConfig class is used to parse and check the configuration file and store the values in a map.
 */
class ServerConfig
{
  public:
    /**
     * @brief Constructor that initializes the ServerConfig with the given file path.
     * @param filepath Path to the configuration file.
     */
    ServerConfig(const std::string &filepath);

    /**
     * @brief Copy constructor.
     * @param src Source ServerConfig object to copy from.
     */
    ServerConfig(const ServerConfig &src);

    /**
     * @brief Assignment operator.
     * @param src Source ServerConfig object to assign from.
     * @return Reference to the assigned ServerConfig object.
     */
    ServerConfig &operator=(const ServerConfig &src);

    /**
     * @brief Destructor.
     */
    ~ServerConfig();

    /**
     * @brief List of valid log levels.
     */
    static const std::vector<std::string> validLogLevels;

    /**
     * @brief Checks if the configuration is valid.
     * @return True if the configuration is valid, false otherwise.
     */
    bool isConfigOK() const;

    /**
     * @brief Parses the configuration file and stores the values in the map.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void parseFile(bool isTest = false, bool isTestPrint = false);

    /**
     * @brief Prints the configuration.
     */
    void printConfig();

    // Getters

    /**
     * @brief Gets the file stream of the configuration file.
     * @return Reference to the file stream.
     */
    std::ifstream &getFile();

    /**
     * @brief Gets the value of a key in the general configuration map.
     * @param key The key to look up.
     * @return The value associated with the key.
     */
    std::string getGeneralConfigValue(const std::string &key) const;

    /**
     * @brief Gets all servers stored in a vector of maps.
     * @param serversConfig Reference to the vector of maps to store the server configurations.
     * @return True if successful, false otherwise.
     */
    bool getAllServersConfig(
				// clang-format off
        std::vector<std::map<std::string, ConfigValue> > &serversConfig // clang-format on
    ) const;

    /**
     * @brief Gets all servers stored in a vector of maps.
     * @return Reference to the vector of maps containing the server configurations.
     */
		// clang-format off
    const std::vector<std::map<std::string, ConfigValue> > &
    getAllServersConfig() const; // clang-format on

    /**
     * @brief Gets the value of a key in a server configuration map.
     * @param serverIndex Index of the server.
     * @param key The key to look up.
     * @param value Reference to store the value associated with the key.
     * @return True if successful, false otherwise.
     */
    bool getServerConfigValue(
        unsigned int serverIndex,
        const std::string &key,
        ConfigValue &value
    ) const;

    // Setters

    /**
     * @brief Sets the root directory for all servers.
     * @param root The root directory to set.
     */
    void setRootToAllServers(const std::string &root);

  private:
    /**
     * @brief Default constructor.
     */
    ServerConfig();

    std::string filepath_; ///< Path to the configuration file.
    std::ifstream file_; ///< File stream for the configuration file.
    std::map<std::string, std::string> generalConfig_; ///< General configuration map.
		// clang-format off
    std::vector<std::map<std::string, ConfigValue> > serversConfig_; ///< Vector of maps for server configurations.
		// clang-format on
    bool isConfigOK_; ///< Flag indicating if the configuration is valid.

    /**
     * @brief Initializes the general configuration.
     */
    void initGeneralConfig_();

    /**
     * @brief Initializes the server configurations.
     */
    void initServersConfig_();

    /**
     * @brief Initializes the location configuration.
     * @param location Reference to the map of location configurations.
     */
		// clang-format off
    void initLocationConfig_(
        std::map<std::string, std::vector<std::string> > &location // clang-format on
    );

    /**
     * @brief Checks if the server listen directive is unique.
     * @param hoist The hoist value.
     * @param port The port value.
     * @param lineIndex Index of the line containing the directive.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     * @return True if the listen directive is unique, false otherwise.
     */
    bool checkServerListenUnique_(
        const std::string &host,
        const std::string &port,
        unsigned int lineIndex,
        bool isTest,
        bool isTestPrint
    );

    /**
     * @brief Checks if the server name directive is unique.
     * @param tokens The server name tokens.
     * @return True if the server name is unique, false otherwise.
     */
    bool checkServerNameUnique_(const std::string &tokens);

    /**
     * @brief Checks if the listen directive is unique.
     * @param itServer Iterator to the server configuration map.
     * @return True if the listen directive is unique, false otherwise.
     */
    bool checkListenUnique_(
		// clang-format off
        std::vector<std::map<std::string, ConfigValue> >::iterator &itServer // clang-format on
    );

    /**
     * @brief Parses a location block.
     * @param tokens Tokens of the location block.
     * @param line The line containing the location block.
     * @param lineIndex Index of the line containing the location block.
     * @param brackets Stack of brackets.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void parseLocationBlock_(
        std::vector<std::string> &tokens,
        std::string &line,
        unsigned int lineIndex,
        std::stack<bool> &brackets,
        bool isTest,
        bool isTestPrint
    );

    /**
     * @brief Checks the general configuration for validity.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void checkGeneralConfig_(bool isTest, bool isTestPrint);

    /**
     * @brief Checks the server configurations for validity.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void checkServersConfig_(bool isTest, bool isTestPrint);

    /**
     * @brief Sets the listen directive.
     * @param tokens Tokens of the listen directive.
     * @param lineIndex Index of the line containing the directive.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void setListenDirective_(
        const std::vector<std::string> &tokens,
        unsigned int lineIndex,
        bool isTest,
        bool isTestPrint
    );

    /**
     * @brief Handles a closing bracket.
     * @param brackets Stack of brackets.
     * @param lineIndex Index of the line containing the closing bracket.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void handleClosingBracket_(
        std::stack<bool> &brackets,
        unsigned int lineIndex,
        bool isTest,
        bool isTestPrint
    );

    /**
     * @brief Checks if the directive is a general directive.
     * @param directive The directive to check.
     * @return True if the directive is a general directive, false otherwise.
     */
    bool isGeneralDirective_(const std::string &directive);

    /**
     * @brief Handles a general directive.
     * @param tokens Tokens of the directive.
     * @param lineIndex Index of the line containing the directive.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void handleGeneralDirective_(
        std::vector<std::string> &tokens,
        unsigned int lineIndex,
        bool isTest,
        bool isTestPrint
    );

    /**
     * @brief Checks if the log level is valid.
     * @param logLevel The log level to check.
     * @return True if the log level is valid, false otherwise.
     */
    bool isValidLogLevel_(const std::string &logLevel);

    /**
     * @brief Checks if the directive is a block directive.
     * @param directive The directive to check.
     * @return True if the directive is a block directive, false otherwise.
     */
    bool isBlockDirective_(const std::string &directive);

    /**
     * @brief Handles a block directive.
     * @param tokens Tokens of the directive.
     * @param brackets Stack of brackets.
     * @param lineIndex Index of the line containing the directive.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void handleBlockDirective_(
        std::vector<std::string> &tokens,
        std::stack<bool> &brackets,
        unsigned int lineIndex,
        bool isTest,
        bool isTestPrint
    );

    /**
     * @brief Checks if the directive is a server directive.
     * @param directive The directive to check.
     * @return True if the directive is a server directive, false otherwise.
     */
    bool isServerDirective_(const std::string &directive);

    /**
     * @brief Handles a server directive.
     * @param tokens Tokens of the directive.
     * @param line The line containing the directive.
     * @param lineIndex Index of the line containing the directive.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void handleServerDirective_(
        std::vector<std::string> &tokens,
        std::string &line,
        unsigned int lineIndex,
        bool isTest,
        bool isTestPrint
    );

    /**
     * @brief Handles a multi-value directive.
     * @param tokens Tokens of the directive.
     * @param lineIndex Index of the line containing the directive.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void handleMultiValueDirective_(
        std::vector<std::string> &tokens,
        unsigned int lineIndex,
        bool isTest,
        bool isTestPrint
    );

    /**
     * @brief Handles a single-value directive.
     * @param tokens Tokens of the directive.
     * @param lineIndex Index of the line containing the directive.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void handleSingleValueDirective_(
        std::vector<std::string> &tokens,
        unsigned int lineIndex,
        bool isTest,
        bool isTestPrint
    );

    /**
     * @brief Handles an error page directive.
     * @param tokens Tokens of the directive.
     * @param lineIndex Index of the line containing the directive.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void handleErrorPageDirective_(
        std::vector<std::string> &tokens,
        unsigned int lineIndex,
        bool isTest,
        bool isTestPrint
    );

    /**
     * @brief Handles a location directive.
     * @param tokens Tokens of the directive.
     * @param line The line containing the directive.
     * @param lineIndex Index of the line containing the directive.
     * @param isTest If true, prints error messages without stopping the program.
     * @param isTestPrint If true, prints error messages without stopping the program.
     */
    void handleLocationDirective_(
        std::vector<std::string> &tokens,
        std::string &line,
        unsigned int lineIndex,
        bool isTest,
        bool isTestPrint
    );
};
