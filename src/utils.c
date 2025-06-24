#include "../include/utils.h"
#include "../include/response.h"
#include <string.h>

extern const struct MimeTypeMapping mime_types[];

int validate_content_type(char *file_extension) {

  int result = -1;
  for (int i = 0; mime_types[i].extension != NULL; i++) {
    result = strcmp(mime_types[i].extension, file_extension);
    if (result == 0) {
      break;
    }
  }

  return result;
}

const LOGS logs = {{"ERROR", "31"}, {"INFO", "32"}, {"DEBUG", "33"}, {"USER", "37"}, {"SUCCESS", "34"}, {"WARNING", "36"}};

void log_to_console(const LOG_TYPE *log_type, char *log_message, int arg,
                    int client_id) {

  size_t message_size = strlen(log_message) + 100;
  char message_buffer[message_size];
  snprintf(message_buffer, message_size, log_message, arg);

  if (client_id == 0) {

    printf("\e[%sm[%s] %s\e[0m\n", log_type->color, log_type->flag,
           message_buffer);
  } else {

    printf("\e[%sm[%s] [Client: %d] %s\e[0m\n", log_type->color, log_type->flag,
           client_id, message_buffer);
  }
}

void log_to_debug(const LOG_TYPE *log_type, char *log_message, char *arg,
                  int client_id) {

  size_t message_size = strlen(log_message) + 100;
  char message_buffer[message_size];
  snprintf(message_buffer, message_size, log_message, arg);

  if (!client_id) {

    printf("\e[%sm[%s] %s\e[0m\n", log_type->color, log_type->flag,
           message_buffer);
  } else {

    printf("\e[%sm[%s] [Client: %d] %s\e[0m\n", log_type->color, log_type->flag,
           client_id, message_buffer);
  }
}
