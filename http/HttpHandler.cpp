#include <http/HttpHandler.h>

#include <HttpRequest.h>
#include <HttpResponse.h>

void defaultHttpHandler(const HttpRequest &, HttpResponse *resp) {
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
}
