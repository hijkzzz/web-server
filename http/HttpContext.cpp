#include <http/HttpContext.h>
#include <net/Buffer.h>

const char CR = '\r';
const char LF = '\n';
const char *CRLFCRLF =  "\r\n\r\n";

bool HttpContext::processRequestLine(Buffer *buf, std::string &uri) {

    const char *method_start;
    const char *uri_start;
    int major_version;
    int minor_version;

    enum {
        sw_start = 0,
        sw_method,
        sw_spaces_before_uri,
        sw_after_slash_in_uri,
        sw_http,
        sw_http_H,
        sw_http_HT,
        sw_http_HTT,
        sw_http_HTTP,
        sw_first_major_digit,
        sw_major_digit,
        sw_first_minor_digit,
        sw_minor_digit,
        sw_spaces_after_digit,
        sw_almost_done
    } state;

    state = sw_start;

    const char *pi;
    char ch;

    for (pi = buf->peek() ; pi < buf->beginWrite(); pi++) {
        ch = *pi;

        switch (state) {
        case sw_start:
            if (ch == CR || ch == LF) {
                break;
            }
            method_start = pi;
            if ((ch < 'A' || ch > 'Z') && ch != '_') {
               goto error;
            }
            state = sw_method;
            break;

        case sw_method:
            if (ch == ' ') {
                if (request_.setMethod(method_start, pi) == false) {
                    goto error;
                }
                state = sw_spaces_before_uri;
                break;
            }
            if ((ch < 'A' || ch > 'Z') && ch != '_') {
                goto error;
            }
            break;

        case sw_spaces_before_uri:
            if (ch == '/') {
                uri_start = pi;
                state = sw_after_slash_in_uri;
                break;
            }
            switch (ch) {
                case ' ':
                    break;
                default:
                    goto error;
            }
            break;

        case sw_after_slash_in_uri:
            switch (ch) {
            case ' ':
                uri = std::string(uri_start, pi);
                state = sw_http;
                break;
            default:
                break;
            }
            break;

        case sw_http:
            switch (ch) {
            case ' ':
                break;
            case 'H':
                state = sw_http_H;
                break;
            default:
                goto error;
            }
            break;

        case sw_http_H:
            switch (ch) {
            case 'T':
                state = sw_http_HT;
                break;
            default:
                goto error;
            }
            break;

        case sw_http_HT:
            switch (ch) {
            case 'T':
                state = sw_http_HTT;
                break;
            default:
                goto error;
            }
            break;

        case sw_http_HTT:
            switch (ch) {
            case 'P':
                state = sw_http_HTTP;
                break;
            default:
                goto error;
            }
            break;

        case sw_http_HTTP:
            switch (ch) {
            case '/':
                state = sw_first_major_digit;
                break;
            default:
                goto error;
            }
            break;

        case sw_first_major_digit:
            if (ch < '1' || ch > '9') {
                goto error;
            }
            major_version = ch - '0';
            state = sw_major_digit;
            break;

        case sw_major_digit:
            if (ch == '.') {
                state = sw_first_minor_digit;
                break;
            }
            if (ch < '0' || ch > '9') {
                goto error;
            }
            major_version = major_version * 10 + ch - '0';
            break;

        case sw_first_minor_digit:
            if (ch < '0' || ch > '9') {
                goto error;
            }
            minor_version = ch - '0';
            state = sw_minor_digit;
            break;

        case sw_minor_digit:
            if (ch == CR) {
                state = sw_almost_done;
                break;
            }
            if (ch == LF) {
                goto done;
            }
            if (ch == ' ') {
                state = sw_spaces_after_digit;
                break;
            }
            if (ch < '0' || ch > '9') {
                goto error;
            }
            minor_version = minor_version * 10 + ch - '0';
            break;

        case sw_spaces_after_digit:
            switch (ch) {
            case ' ':
                break;
            case CR:
                state = sw_almost_done;
                break;
            case LF:
                goto done;
            default:
                goto error;
            }
            break;

        case sw_almost_done:
            switch (ch) {
            case LF:
                goto done;
            default:
                goto error;
            }
        }
    }

error:
    buf->retrieve(pi - buf->peek());
    return false;

done:
    buf->retrieve(pi - buf->peek());
    if (request_.setVersion(major_version, minor_version) == false) {
        return false;
    }
    state_ = kExpectHeaders;
    return true;
}

bool HttpContext::processRequestBody(Buffer *buf) {

    const char *header_key_start;
    const char *header_key_end;
    const char *header_value_start;
    const char *header_value_end;

    enum {
        sw_start = 0,
        sw_key,
        sw_spaces_before_colon,
        sw_spaces_after_colon,
        sw_value,
        sw_cr,
        sw_crlf,
        sw_crlfcr
    } state;

    state = sw_start;

    char ch;
    const char *pi;

    for (pi = buf->peek(); pi < buf->beginWrite(); pi++) {
        ch = *pi;

        switch (state) {
        case sw_start:
            if (ch == CR || ch == LF) {
                break;
            }
            header_key_start = pi;
            state = sw_key;
            break;

        case sw_key:
            if (ch == ' ') {
                header_key_end = pi;
                state = sw_spaces_before_colon;
                break;
            }
            if (ch == ':') {
                header_key_end = pi;
                state = sw_spaces_after_colon;
                break;
            }
            break;

        case sw_spaces_before_colon:
            if (ch == ' ') {
                break;
            } else if (ch == ':') {
                state = sw_spaces_after_colon;
                break;
            } else {
                goto error;
            }
        case sw_spaces_after_colon:
            if (ch == ' ') {
                break;
            }
            state = sw_value;
            header_value_start = pi;
            break;

        case sw_value:
            if (ch == CR) {
                header_value_end = pi;
                state = sw_cr;
            }
            if (ch == LF) {
                header_value_end = pi;
                state = sw_crlf;
            }
            break;

        case sw_cr:
            if (ch == LF) {
                state = sw_crlf;
                request_.setHeader(header_key_start, header_key_end, header_value_start, header_value_end);
                break;
            } else {
                goto error;
            }

        case sw_crlf:
            if (ch == CR) {
                state = sw_crlfcr;
            } else {
                header_key_start = pi;
                state = sw_key;
            }
            break;

        case sw_crlfcr:
            switch (ch) {
            case LF:
                goto done;
            default:
                goto error;
            }
            break;
        }
    }

error:
    buf->retrieve(pi - buf->peek());
    return false;

done:
    buf->retrieve(pi - buf->peek());
    state_ = kExpectBody;
    return true;
}


bool HttpContext::parseRequest(Buffer *buf, Clock::time_point receiveTime) {
    std::string uri;
    if (!processRequestLine(buf, uri) || !processRequestBody(buf))  {
        return false;
    }

    // process uri
    std::string::size_type pos = uri.find_first_of('?');
    if (pos == std::string::npos) {
        request_.setPath(uri.data(), uri.data() + uri.size());
    } else {
        request_.setPath(uri.data(), uri.data() + pos);
        request_.setQuery(uri.data() + pos + 1, uri.data() + uri.size());
    }

    request_.setReceiveTime(receiveTime);
    state_ = kGotAll;

    return true;
}
