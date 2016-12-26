#ifndef NET_HTTPREQUEST_H
#define NET_HTTPREQUEST_H

#include <net/Callbacks.h>

#include <string>
#include <chrono>
#include <map>

#include <assert.h>

class HttpRequest {
public:
    enum Method { kInvalid, kGet, kPost, kHead, kPut, kDelete };
    enum Version { kUnknown, kHttp10, kHttp11, kHttp20 };

    HttpRequest()
            : method_(kInvalid),
              version_(kUnknown) {
    }

    bool setVersion(int major, int minor) {
        switch (major) {
        case 1:
            switch (minor) {
            case 0:
                version_ = kHttp10;
                break;
            case 1:
                version_ = kHttp11;
                break;
            default:
                return false;
            }
            break;
        case 2:
            switch (minor) {
            case 0:
                version_ = kHttp20;
                break;
            default:
                return false;
            }
            break;
        default:
            return false;
        }
        return true;
    }
    Version version() const { return version_; }
    std::string versionString() const {
        switch (version_) {
            case kHttp10:
                return "1.0";
            case kHttp11:
                return "1.1";
            case kHttp20:
                return "2.0";
            default:
                return "UNKNOWN";
        }
    }

    bool setMethod(const char *start, const char *end) {
        assert(method_ == kInvalid);
        std::string m(start, end);
        if (m == "GET") {
            method_ = kGet;
        } else if (m == "POST") {
            method_ = kPost;
        } else if (m == "HEAD") {
            method_ = kHead;
        } else if (m == "PUT") {
            method_ = kPut;
        } else if (m == "DELETE") {
            method_ = kDelete;
        } else {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }
    Method method() const { return method_; }

    std::string methodString() const {
        std::string result = "UNKNOWN";
        switch (method_) {
            case kGet:
                result = "GET";
                break;
            case kPost:
                result = "POST";
                break;
            case kHead:
                result = "HEAD";
                break;
            case kPut:
                result = "PUT";
                break;
            case kDelete:
                result = "DELETE";
                break;
            default:
                break;
        }
        return result;
    }

    void setPath(const char *start, const char *end) {
        path_.assign(start, end);
    }
    const std::string &path() const { return path_; }

    void setQuery(const char *start, const char *end) {
        query_.assign(start, end);
    }
    const std::string &query() const { return query_; }

    void setReceiveTime(Clock::time_point t) { receiveTime_ = t; }
    Clock::time_point receiveTime() const { return receiveTime_; }

    void setHeader(const char *key_start, const char *key_end, const char *value_start, const char *value_end) {
        std::string field(key_start, key_end);
        headers_[field] = std::string(value_start, value_end);
    }
    std::string header(const std::string &field) const {
        std::string                                   result;
        std::map<std::string, std::string>::const_iterator it = headers_.find(field);
        if (it != headers_.end()) {
            result = it->second;
        }
        return result;
    }
    const std::map <std::string, std::string> &headers() const { return headers_; }

    void swap(HttpRequest &that) {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        std::swap(receiveTime_, that.receiveTime_);
        headers_.swap(that.headers_);
    }

private:
    Method                             method_;
    Version                            version_;
    std::string                        path_;
    std::string                        query_;
    Clock::time_point                  receiveTime_;
    std::map<std::string, std::string> headers_;
};


#endif
