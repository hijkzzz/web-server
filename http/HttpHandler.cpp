#include <http/HttpHandler.h>

#include <http/HttpRequest.h>
#include <http/HttpResponse.h>

void defaultHttpHandler(const HttpRequest &, HttpResponse *resp) {
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
}
