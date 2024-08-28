# Webserv

## Usage

### Compilation

`make`

### Execution

`` ./webserv [OPTIONAL: flags] [OPTIONAL: config_file]` ``

#### Flags

| Flag          | Description                                                  |
| ------------- | ------------------------------------------------------------ |
| -h, --help    | Display help message                                         |
| -v, --version | Display WebServ version                                      |
| -V, --Version | Display WebServ version and extra info                       |
| -t, --test    | Check the configuration file and exit the server             |
| -T, --Test    | Check and print the configuration file, than exit the server |

### Execute Tests

#### Run all tests

`make test`

#### Run specific test

`make test T=SpecificTestName`
