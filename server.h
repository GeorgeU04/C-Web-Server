#ifndef SERVER_H
#define SERVER_H
#include <stdint.h>
int32_t send_file_response(int32_t client_socket, const char *path);
int32_t send_response(int32_t client_socket, const char *status,
                      const char *content_type, const char *body);
const char *get_content_type(const char *path);
#endif // !SERVER_H
