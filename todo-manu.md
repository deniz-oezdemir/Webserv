## To Do
- [ ] Example task
- [ ] Implement: compare request length with limit set in congif
- [ ] Change name of all abstrac classes to have A prefix
- [ ] Implement: client fd reader


## In Progress
- [ ] Implement: keep connection alive variable in HttpRequest

## Done
- [x] Check: check HttpRequest copy assignment and copy constructor
- [x] Implement: add defaul port
- [x] Implement: check presence of port in Host header and normalize
- [x] Change all private methods to use _ at end across whole program
- [x] Implement: add port_ and corresponding getter to HttpRequest
- [x] Implement: Normalize URI.
- [x] Implement: URI schemes. 
- [x] Implement: filter parser to only check for accepted headers 
- [x] Implement: measure body length and set in private attr
- [x] Implement: host_ var in HttpRequest
- [x] Implement: token syntax 
    - [x] Implement: add check for header with wrong separator (, or ;) 
- [x] Implement: accept HTTP headers with multiple values
        - [x] headers separated by semicolons
        - [x] check headerName syntax
        - [x] decide how to handle empty list items
        - [x] check values special chars are escaped
- [x] Refactor: all HTTP request normalization into RequestParser
- [x] Test: headers with multiple values
- [x] Test: headers with multiple values with semicolon
