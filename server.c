#include "server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

const char *get_content_type(const char *path) {
  if (strstr(path, ".html"))
    return "text/html";
  if (strstr(path, ".css"))
    return "text/css";
  if (strstr(path, ".js"))
    return "application/javascript";
  if (strstr(path, ".jpg"))
    return "image/jpeg";
  if (strstr(path, ".png"))
    return "image/png";
  return "text/plain";
}

int32_t send_response(int32_t client_socket, const char *status,
                      const char *content_type, const char *body) {
  char response[BUFFER_SIZE];
  snprintf(response, sizeof(response),
           "HTTP/1.1 %s\r\n"
           "Content-Type: %s\r\n"
           "Content-Length: %ld\r\n"
           "\r\n%s",
           status, content_type, strlen(body), body);
  if (write(client_socket, response, strlen(response)) < 0) {
    return -1;
  }
  return 0;
}

int32_t send_file_response(int32_t client_socket, const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    send_response(client_socket, "404 Not Found", "text/plain",
                  "404 Not Found");
    return -1;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  char *file_content = malloc(file_size + 1);
  if (!file_content) {
    fclose(file);
    send_response(client_socket, "500 Internal Server Error", "text/plain",
                  "500 Internal Server Error");
    return -1;
  }

  fread(file_content, 1, file_size, file);
  file_content[file_size] = '\0';
  fclose(file);

  const char *content_type = get_content_type(path);
  if (send_response(client_socket, "200 OK", content_type, file_content) < 0) {
    return -1;
  }

  free(file_content);
  return 0;
}
