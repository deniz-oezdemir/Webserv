# Webserv

## Usage

### Compilation

```bash
make
```

### Execution

```bash
./webserv [OPTIONAL: flags] [OPTIONAL: config_file]
```

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

```bash
make test
```

#### Run specific test

```bash
make test
make test T=SpecificTestName
```

## Miscellaneous

- test non blocking behaviour from multiple terminal clients with different messages like `yes "Example message 1" | telnet localhost 8080`
