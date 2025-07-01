#pragma once

typedef struct {
  char *flag;
  char *color;
} LOG_TYPE;

typedef struct {
  LOG_TYPE error;
  LOG_TYPE info;
  LOG_TYPE debug;
  LOG_TYPE user;
  LOG_TYPE success;
  LOG_TYPE warning;
} LOGS;

extern const LOGS logs;

void log_to_console(const LOG_TYPE *log_type, char *log_message, int arg,
                    int client_id);

void log_to_debug(const LOG_TYPE *log_type, char *log_message, char *arg,
                  int client_id);

int validate_content_type(char *file_extension, char *file_name);
