#include "../include/utils.h"
#include "../include/response.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zlib.h>

extern const struct MimeTypeMapping mime_types[];
extern char *public_directory;

int compress_to_gzip(char *file_name) {
  char file_path[30];
  snprintf(file_path, 30, "%s/%s", public_directory, file_name);

  FILE *file_to_read = fopen(file_path, "r");
  if (file_to_read == NULL) {
    log_to_console(&logs.error, "file failed to open for compress, sire", 0, 0);
    return -1;
  }

  struct stat file_statbuf;
  int stat_result = fstat(fileno(file_to_read), &file_statbuf);

  if (stat_result < 0) {
    log_to_console(&logs.error, "failed to get file stat for compress, sire!", 0, 0);
    fclose(file_to_read);
    return -1;
  }

  size_t required_file_size = file_statbuf.st_size;
  char *input_buffer = (char *)malloc(required_file_size);
  size_t bytes_read = fread(input_buffer, 1, required_file_size, file_to_read);
  if (bytes_read != required_file_size) {
    log_to_console(&logs.error, "failed to read for compress, sire!", 0, 0);
    free(input_buffer);
    fclose(file_to_read);
    return -1;
  }

  void *output_buffer = malloc(required_file_size);

  z_stream stream = {0};
  deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);

  stream.next_in = (unsigned char *)input_buffer;
  stream.avail_in = required_file_size;

  stream.next_out = output_buffer;
  stream.avail_out = required_file_size;

  deflate(&stream, Z_FINISH);
  size_t output_length = stream.total_out;

  deflateEnd(&stream);

  char new_dir_path[30];
  snprintf(new_dir_path, 30, "%s/compressed", public_directory);
  if (mkdir(new_dir_path, 0755) != 0) {

    if (errno != EEXIST) {
      log_to_console(&logs.error, "failed to create \"compressed\" directory, sire", 0, 0);
      free(input_buffer);
      free(output_buffer);
      fclose(file_to_read);

      return -1;
    }
  }
  char new_file_path[40];
  snprintf(new_file_path, 40, "%s/compressed/%s.gz", public_directory, file_name);

  FILE *file_to_write_to = fopen(new_file_path, "wb");
  if (file_to_write_to == NULL) {
    log_to_console(&logs.error, "failed to create file, sire", 0, 0);
    return -1;
  }

  if (fwrite(output_buffer, sizeof(output_buffer[0]), output_length, file_to_write_to) == 0) {
    log_to_console(&logs.error, "failed write operation, sire", 0, 0);
    goto cleanup;
  }

cleanup:
  fclose(file_to_write_to);
  free(input_buffer);
  free(output_buffer);
  return -1;

  return 0;
}

int validate_content_type(char *file_extension, char *file_name) {

  int result = -1;
  for (int i = 0; mime_types[i].extension != NULL; i++) {
    result = strcmp(mime_types[i].extension, file_extension);
    if (result == 0) {
      if ((strstr(mime_types[i].content_type, "text") != NULL) || (strstr(mime_types[i].content_type, "application") != NULL)) {
        log_to_debug(&logs.info, "File: \e[35m%s\e[0m is being compressed to gzip format for faster delivery", file_name, 0);
        if (compress_to_gzip(file_name) == 0) {
          exit(1);
        }
      }
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
