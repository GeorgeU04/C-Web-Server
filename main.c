#include "server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8082
#define BUFFER_SIZE 1024

int32_t main(void) {
  struct sockaddr_in address, peer_address;
  socklen_t addr_len = sizeof(address);
  socklen_t peer_addr_len = sizeof(peer_address);
  int32_t server, new_connection;
  char buffer[BUFFER_SIZE] = {0};

  if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("[ERROR]:");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server, (struct sockaddr *)&address, addr_len) < 0) {
    perror("[ERROR]:");
    close(server);
    exit(EXIT_FAILURE);
  }

  if (listen(server, 10) < 0) {
    perror("[ERROR]:");
    close(server);
    exit(EXIT_FAILURE);
  }

  printf("Server is listening on port %d\n", PORT);

  while (1) {
    if ((new_connection = accept(server, (struct sockaddr *)&peer_address,
                                 &peer_addr_len)) < 0) {
      close(server);
      perror("[ERROR]:");
      exit(EXIT_FAILURE);
    }

    int32_t bytes_read = read(new_connection, buffer, sizeof(buffer));
    if (bytes_read < 0) {
      perror("[ERROR]:");
      close(new_connection);
      continue;
    }

    printf("Request received:\n%s\n", buffer);

    char method[16], path[256];
    sscanf(buffer, "%s %s", method, path);
    if (strcmp(path, "/") == 0) {
      strcpy(path, "/index.html");
    }

    char full_path[512];
    snprintf(full_path, sizeof(full_path), ".%s", path);
    if (send_file_response(new_connection, full_path) < 0) {
      perror("[ERROR]:");
      close(new_connection);
      continue;
    }

    close(new_connection);
  }

  close(server);
  return 0;
}
