#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFSIZE 1024



typedef struct {
  char *headers;
  char *body;
} CerveoRequest;

typedef struct {
  int *client_socket;
  float http_version;
  unsigned short status_code;
  const char *status_message;
  char *headers;
  char *body;
} CerveoResponse;

void set_http_version(CerveoResponse *res, float http_version);
void set_status_code(CerveoResponse *res, unsigned short status_code);
const char *get_status_message(unsigned short status_code);
int set_http_header(CerveoResponse *res, char *key, char *value);
int set_http_body(CerveoResponse *res, char *body);
char *cerveo_response_to_string(CerveoResponse *res);
int write_http_response(CerveoResponse *res);
int end_http_response(CerveoResponse *res);
int send_http_response(CerveoResponse *res);
void cerveo_handle_request(CerveoRequest *req, CerveoResponse *res);
void *cerveo_handle_client(void *sockptr);
int cerveo_serve_http(unsigned short port);
