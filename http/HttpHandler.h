#ifndef HTTP_HTTPHANDLER_H
#define HTTP_HTTPHANDLER_H

class HttpRequest;
class HttpResponse;

void defaultHttpHandler(const HttpRequest &, HttpResponse *resp);

#endif
