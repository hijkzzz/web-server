#ifndef HTTP_HTTPHANDLER_H
#define HTTP_HTTPHANDLER_H

class HttpRequest;
class HttpResponse;
class HttpServer;

void defaultHttpHandler(const HttpRequest &req, HttpResponse *resp, HttpServer *server);

#endif
