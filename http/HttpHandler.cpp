#include <http/HttpHandler.h>

#include <http/HttpRequest.h>
#include <http/HttpResponse.h>

namespace {
    std::string requestString(const HttpRequest &req) {
        std::string body;
        body.append(req.methodString() + " "
                    + req.path() + " "
                    + req.query() + " HTTP/"
                    + req.versionString() + "\n");
        for (auto i : req.headers()) {
            body.append(i.first + ": " + i.second + "\n");
        }
        return body;
    }
}

void defaultHttpHandler(const HttpRequest &req, HttpResponse *resp, HttpServer *server) {
    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setBody(requestString(req));
}

void webServerHandler(const HttpRequest &req, HttpResponse *resp, HttpServer *server) {

}
