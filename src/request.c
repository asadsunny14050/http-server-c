#include "../include/request.h"
#include <stdio.h>
#include <string.h>

void parse_headers(char *request_buffer, HttpRequest *request,
                   int buffer_length) {

  // the header key-value pair starts here minus the request line
  char *header_values = strstr(request_buffer, "\r\n");
  // parse request line
  char general_delimiter = ' ';
  char *start_of_path = strchr(request_buffer, general_delimiter) + 1;
  char *end_of_path = strchr(start_of_path, general_delimiter);
  *end_of_path = '\0';

  char *end_of_method = strchr(request_buffer, general_delimiter);
  *end_of_method = '\0';

  request->method = request_buffer;
  request->path = start_of_path;
  printf("method:%s\n", request->method);
  printf("path:%s\n\n", request->path);
  printf("header:\n%s\n\n", header_values);
}
