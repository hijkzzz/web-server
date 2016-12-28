#include <http/HttpHandler.h>

#include <http/HttpRequest.h>
#include <http/HttpResponse.h>
#include <http/HttpServer.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

void defaultHttpHandler(const HttpRequest &req, HttpResponse *resp, HttpServer *server) {
    std::string fileName = server->root() + req.path();
    if(req.path() == "/") {
        fileName.append("index.html");
    }

    struct stat sbuf;
    if(::stat(fileName.c_str(), &sbuf) < 0) {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        return;
    }

    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
        resp->setStatusCode(HttpResponse::k403Forbidden);
        resp->setStatusMessage("Forbidden");
        return;
    }


}
