## To Do
- [ ] Example task
- [ ] Check: what to do with unfolded headers (headers that continue on next line for readability. The next line starts with whitespace. They should be appended.)
- [ ] Check: Set-Cookie header may be repeated and is a special case
- [ ] Implement: check presence of Content-Length header in POST and its absense in GET and DELETE (double-check these rules!!) 
- [ ] Implement: compare request length with limit set in congif
- [ ] Change name of all abstrac classes to have A prefix
- [ ] Implement: keep connection alive variable in HttpRequest
- [ ] Implement: client fd reader
- [ ] Implement: add defaul port
- [ ] Check: check HttpRequest copy assignment and copy constructor


## In Progress

## Done
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
