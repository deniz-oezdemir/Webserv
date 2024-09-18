   ## To Do
   - [ ] Example task
   - [ ] Change all private methods to use _ at end across whole program
   - [ ] Implement: ensure all header fields get transformed to upper-case for normalization
   - [ ] Check: what to do with unfolded headers (headers that continue on next line for readability. The next line starts with whitespace. They should be appended.)
   - [ ] Check: URI schemes
   - [ ] Check: Set-Cookie header may be repeated and is a special case
   - [ ] Implement: filter parser to only check for accpted methods 
   - [ ] Implement: check presence of Content-Length header in POST and its absense in GET and DELETE (double-check these rules!!) 

   ## In Progress

   ## Done
   - [x] Implement: measure body length and set in private attr, and compare with header if any and also add check in response handler for if it exceeds limit in config file
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
