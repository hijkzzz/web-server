#include <http/HttpHandler.h>

#include <http/HttpRequest.h>
#include <http/HttpResponse.h>
#include <http/HttpServer.h>
#include <http/StringBuilder.h>

#include <net/Logging.h>

#include <fstream>

namespace {
    std::map<std::string, std::string> MimeTable =
                        {
                                {".html", "text/html"},
                                {".xml", "text/xml"},
                                {".xhtml", "application/xhtml+xml"},
                                {".txt", "text/plain"},
                                {".rtf", "application/rtf"},
                                {".pdf", "application/pdf"},
                                {".word", "application/msword"},
                                {".png", "image/png"},
                                {".gif", "image/gif"},
                                {".jpg", "image/jpeg"},
                                {".jpeg", "image/jpeg"},
                                {".au", "audio/basic"},
                                {".mpeg", "video/mpeg"},
                                {".mpg", "video/mpeg"},
                                {".avi", "video/x-msvideo"},
                                {".gz", "application/x-gzip"},
                                {".tar", "application/x-tar"},
                                {".css", "text/css"},
                        };

    const std::string defaultType = "text/plain";

    const std::string &getTypeString(const std::string &fileName) {
        auto pos = fileName.find_last_of('.');
        if (pos == std::string::npos) {
            return defaultType;
        }

        auto itr = MimeTable.find(fileName.substr(pos));
        if (itr == MimeTable.end()) {
            return defaultType;
        } else {
            return itr->second;
        }
    }
}

void defaultHttpHandler(const HttpRequest &req, HttpResponse *resp, HttpServer *server) {
    std::string fileName = server->root() + req.path();
    if(req.path() == "/") {
        fileName.append("index.html");
    }

    StringBuilder<char> sb;
    std::ifstream in(fileName , std::ios::binary);
    if (in.is_open()) {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");

        char buffer[65535];
        while (!in.eof() ) {
            in.read(buffer, 65535);
            sb.Append(std::string(buffer, in.gcount()));
        }

        resp->setContentType(getTypeString(fileName));
        resp->setBody(sb.ToString());
    } else {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
    }
}
