   ## To Do
   - [ ] Example task
   - [ ] Change all private methods to use _ at end across whole program
   - [ ] Implement: ensure all header fields get transformed to upper-case for normalization
   - [ ] Test: headers with multiple values
   - [ ] Check: what to do with unfolded headers
   - [ ] Check: URI schemes
   - [ ] Check: Set-Cookie header may be repeated and is a special case
   - [ ] Implement: measure body length and set in private attr, and compare with header if any and also add check in response handler for if it exceeds limit in config file
   - [ ] Implement: host_ var in HttpRequest

   ## In Progress
   - [x] Implement: accept HTTP headers with multiple values
            - [ ] headers separated by semicolons

   ## Done
            - [x] check headerName syntax
            - [x] decide how to handle empty list items
            - [x] check values special chars are escaped
   - [x] Refactor: all HTTP request normalization into RequestParser
