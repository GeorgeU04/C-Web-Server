#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8082
#define BUFFER_SIZE 1000

int main(void) {
  struct sockaddr_in address, peer_address;
  socklen_t addr_len = sizeof(address);
  socklen_t peer_addr_len = sizeof(peer_address);
  int32_t server, new_connection;
  char buffer[BUFFER_SIZE] = {0};
  char response[BUFFER_SIZE] = {0};
  // Create socket
  if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("[ERROR]:");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Bind socket to address
  if (bind(server, (struct sockaddr *)&address, addr_len) < 0) {
    perror("[ERROR]:");
    close(server);
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(server, 10) < 0) {
    perror("[ERROR]:");
    close(server);
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Accept a new connection
    if ((new_connection = accept(server, (struct sockaddr *)&peer_address,
                                 &peer_addr_len)) < 0) {
      perror("[ERROR]:");
      exit(EXIT_FAILURE);
    }

    // Read the request
    if (read(new_connection, buffer, sizeof(buffer)) < 0) {
      perror("[ERROR]:");
      close(new_connection);
      continue;
    }

    printf("Request received:\n%s\n", buffer);

    FILE *file = fopen("index.html", "r");
    if (file == NULL) {
      perror("[ERROR]: could not open file");
      close(new_connection);
      continue;
    }

    fseek(file, 0, SEEK_END);
    int64_t file_size = ftell(file);
    rewind(file);

    char *file_content = malloc(file_size + 1);
    if (file_content == NULL) {
      perror("[ERROR]:");
      fclose(file);
      close(new_connection);
      continue;
    }

    fread(file_content, 1, file_size, file);
    file_content[file_size] = '\0';
    fclose(file);

    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %ld\r\n"
             "\r\n%s",
             file_size, file_content);

    // Send the response
    if (write(new_connection, response, strlen(response)) < 0) {
      perror("[ERROR]:");
      close(new_connection);
      close(server);
      exit(EXIT_FAILURE);
    }

    printf("Response sent\n");

    // Close connections
    close(new_connection);
  }

  close(server);
  return 0;
}
