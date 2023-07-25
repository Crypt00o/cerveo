#include "cerveo.c"

void cerveo_handle_request(CerveoRequest *req,
                           CerveoResponse *res) {

  set_http_version(res, 1.1);
  set_status_code(res, 200);

  set_http_header(res, "Content-Type", "text/html");
  set_http_body(res,
                "<html><body><h1>Cerveo The Blazingly Fast Http Server In the "
                "earth !</h1> <h5>Powerd By : 0xCrpt00o </h5> </body></html>");
  write_http_response(res);
  end_http_response(res);
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    unsigned short port = atoi(argv[1]);
    if (port > 0 && port < 65535) {
      cerveo_serve_http(port);
    } else {
      printf("Invalid port number. Using default port 8080.\n");
      cerveo_serve_http(8080);
    }
  } else {
    cerveo_serve_http(8080);
  }

  return 0;
}
