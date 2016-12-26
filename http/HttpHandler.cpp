#include <http/HttpHandler.h>

#include <http/HttpRequest.h>
#include <http/HttpResponse.h>

void defaultHttpHandler(const HttpRequest &req, HttpResponse *resp) {
    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");

    std::string body;
    for (auto i : req.headers()) {
        body.append(i.first + " : " + i.second + "\n");
    }

    resp->setBody(std::move(body));
}
