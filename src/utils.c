#include "../include/request.h"
#include "../include/response.h"
#include <stdio.h>

void read_html_file(HttpRequest *request, HttpResponse *response) {

  char file_name[20];
  snprintf(file_name, 20, "../assets%s.html", request->path);
  FILE *file_to_read = fopen(file_name, "r");

  if (!file_to_read) {
    perror("failed to load the file you have given");
  }
}
