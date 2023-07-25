#include "cerveo.h"



void set_http_version(CerveoResponse *res, float http_version) {
  res->http_version = http_version;
}

const char *get_status_message(unsigned short status_code) {
  switch (status_code) {
    case 100:
      return "Continue";
    case 200:
      return "OK";
    case 201:
      return "Created";
    case 204:
      return "No Content";
    case 400:
      return "Bad Request";
    case 401:
      return "Unauthorized";
    case 403:
      return "Forbidden";
    case 404:
      return "Not Found";
    case 500:
      return "Internal Server Error";
    default:
      return "Unknown Status";
  }
}

void set_status_code(CerveoResponse *res, unsigned short status_code) {
  res->status_code = status_code;
  res->status_message = get_status_message(status_code);
}

static void prepare_http_res(CerveoResponse *res) {
  if (res->http_version == 0) {
    res->http_version = 1.1;
  }
  if (res->status_code == 0) {
    res->status_code = 200;
  }
  if (res->status_message == NULL) {
    res->status_message = get_status_message(200);
  }
}



int set_http_header(CerveoResponse *res, char *key, char *value) {
  char header_key_value[BUFSIZE];
  snprintf(header_key_value, BUFSIZE, "%s: %s\r\n", key, value);

  if (res->headers == NULL) {
    res->headers = (char *)malloc(strlen(header_key_value) + 1);
    if (res->headers == NULL)
      return -1;
    strcpy(res->headers, header_key_value);
  } else {
    res->headers = (char *)realloc(
        res->headers, strlen(res->headers) + strlen(header_key_value) + 1);
    if (res->headers == NULL)
      return -1;

    strcat(res->headers, header_key_value);
  }

  return 0;
}

int set_http_body(CerveoResponse *res, char *body) {
  unsigned long int body_length = strlen(body) ;
  res->body = (char *)malloc((body_length+1) * sizeof(char));
  if (res->body == NULL)
    return -1;

  char body_length_str[32];
  snprintf(body_length_str, 32, "%ld", body_length);
  set_http_header(res, "Content-Length", body_length_str);
  snprintf(res->body, BUFSIZE, "%s", body);
  return 0;
}

char *cerveo_response_to_string(CerveoResponse *res) {
  char *res_str = (char *)malloc(strlen(res->headers) + strlen(res->body) +
                                 strlen(res->status_message) + 128);
  if (res_str == NULL)
    return NULL;

  sprintf(res_str, "HTTP/%2.1f %d %s\r\n%s\r\n%s", res->http_version,
          res->status_code, res->status_message, res->headers, res->body);
  return res_str;
}

int write_http_response(CerveoResponse *res) {
  prepare_http_res(res);
  char *res_str = cerveo_response_to_string(res);
  if (res_str == NULL)
    return -1;

  send(*res->client_socket, res_str, strlen(res_str), 0);
  free(res_str);
  return 0;
}

int end_http_response(CerveoResponse *res){ 
	  close(*res->client_socket);
	  return 0;
}


int send_http_response(CerveoResponse *res) {
  prepare_http_res(res);
  write_http_response(res);
  end_http_response(res);
  return 0;
}



void *handle_client(void *sockptr) {
  int client_socket = *(int *)sockptr;
  char buffer[BUFSIZE] = {0}, request_path[BUFSIZE] = {0};
  int bytesread = recv(client_socket, buffer, sizeof(buffer), 0);
  if (bytesread <= 0) {
    close(client_socket);
    pthread_exit(NULL);
  }

  CerveoResponse *res =
      (CerveoResponse *)malloc(sizeof(CerveoResponse));

  CerveoRequest *req =
      (CerveoRequest *)malloc(sizeof(CerveoRequest));

  if (res == NULL || req == NULL) {
    close(client_socket);
    return NULL;
  }

  res->headers = NULL;
  res->body = NULL;
  res->client_socket = &client_socket;
  res->http_version = 0;
  res->status_message = NULL;
  res->status_code = 0;

  cerveo_handle_request(req, res);

  if (res->headers)
    free(res->headers);
  if (res->body)
    free(res->body);
  free(res);
  free(req);

  return NULL;
}


int cerveo_serve_http(unsigned short port) {
  int server_socket;

  if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0) {
    perror("Socket Creation Failed");
    return -1;
  }

  int opt = 1;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("Can,t Set Socket Options");
    return -1;
  }

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Socket Binding Failed");
    return -1;
  }

  if (listen(server_socket, 100) < 0) {
    perror("Socket Listening Failed");
    return -1;
  }

  int new_socket, addrlen = sizeof(struct sockaddr_in);
  while (1) {
    if ((new_socket = accept(server_socket, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) == 0) {
      perror("Socket Accepting Failed");
      return -1;
    } else {
      pthread_t thread;
      pthread_create(&thread, NULL, handle_client, (void *)&new_socket);
    }
  }

  return 0;
}
