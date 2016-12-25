#ifndef NET_HTTPRESPONSE_H
#define NET_HTTPRESPONSE_H

#include <map>
#include <string>

class Buffer;

class HttpResponse {
public:
    enum HttpStatusCode {
        kUnknown,
        k200Ok               = 200,
        k301MovedPermanently = 301,
        k400BadRequest       = 400,
        k404NotFound         = 404,
    };

    explicit HttpResponse(bool close)
            : statusCode_(kUnknown),
              closeConnection_(close) {
    }

    void setStatusCode(HttpStatusCode code) { statusCode_ = code; }
    void setStatusMessage(const std::string &message) { statusMessage_ = message; }
    void setContentType(const std::string &contentType) { setHeader("Content-Type", contentType); }
    void setHeader(const std::string &key, const std::string &value) { headers_[key] = value; }
    void setBody(const std::string &body) { body_ = body; }

    void setCloseConnection(bool on) { closeConnection_ = on; }
    bool closeConnection() const { return closeConnection_; }

    void appendToBuffer(Buffer *output) const;

private:
    std::map<std::string, std::string> headers_;
    HttpStatusCode                     statusCode_;
    std::string                        statusMessage_;
    bool                               closeConnection_;
    std::string                        body_;
};

#endif
