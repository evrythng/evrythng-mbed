/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 *
 * --- DISCLAIMER ---
 *
 * EVRYTHNG provides this source code "as is" and without warranty of any kind,
 * and hereby disclaims all express or implied warranties, including without
 * limitation warranties of merchantability, fitness for a particular purpose,
 * performance, accuracy, reliability, and non-infringement.
 *
 * Author: Michel Yerly
 *
 */
#include "EvrythngApi.h"

#include <string>

#include "util.h"
#include "evry_error.h"
#include "JsonParser.h"

//#define DEBUG_EVRYTHNG_API

using namespace std;

const int HTTP_OK = 200;

const char* THNG_PATH = "/thngs/";
const char* THNG_PROP_PATH = "/properties/";

EvrythngApi::EvrythngApi(const string& token, const string& host, int port)
{
    this->token = token;
    this->host = host;
    this->port = port;
}

EvrythngApi::~EvrythngApi()
{
}

int EvrythngApi::getThngPropertyValue(const string& thngId, const string& key, string& value)
{
    string path = THNG_PATH;
    path += thngId;
    path += THNG_PROP_PATH;
    path += key;
    path += "?from=latest";
    string res;
    int err;
    int code;
    if ((err = httpGet(path, res, code)) != 0) return err;
    if (code != HTTP_OK) return EVRY_ERR_UNEXPECTEDHTTPSTATUS;
    JsonParser json;
    json.parse(res.c_str());
    JsonValue* doc = json.getDocument();
    const char* v = doc->getString("0/value");
    if (v) {
        value.assign(v);
        return 0;
    } else {
        return -1;
    }
}

int EvrythngApi::setThngPropertyValue(const std::string& thngId, const std::string& key, const std::string& value, int64_t timestamp)
{
    char strTimestamp[21];
    char* end;
    sprinti64(strTimestamp, timestamp, &end);
    *end = '\0';

    string path = THNG_PATH;
    path += thngId;
    path += THNG_PROP_PATH;
    path += key;

    string json = "[{\"timestamp\":";
    json += strTimestamp;
    json += ",\"value\":\"";
    json += value;
    json += "\"}]";

    string res;
    int err;
    int code;
    if ((err = httpPut(path, json, res, code)) != 0) return err;
    if (code != HTTP_OK) return EVRY_ERR_UNEXPECTEDHTTPSTATUS;

    return 0;
}


int EvrythngApi::httpRequest(HttpMethod method, const string& path, const string& content, string& out, int& codeOut)
{
    int ret;

    const char* strMethod;
    switch (method) {
        case GET:
            strMethod = "GET";
            break;
        case PUT:
            strMethod = "PUT";
            break;
        case POST:
            strMethod = "POST";
            break;
        case DELETE:
            strMethod = "DELETE";
            break;
        default:
            return EVRY_ERR_UNSUPPORTED;
    }

    char contentLength[16];
    snprintf(contentLength, sizeof(contentLength), "%d", content.size());

    string req = strMethod;
    req += " ";
    req += path;
    req += " HTTP/1.0\r\n"
           "Host: ";
    req += host;
    req += "\r\n"
           "Accept: application/json\r\n"
           "Content-Length: ";
    req += contentLength;
    req += "\r\n"
           "Content-Type: application/json\r\n"
           "Connection: close\r\n"
           "Authorization: ";
    req += token;
    req += "\r\n\r\n";

    req += content;

#ifdef DEBUG_EVRYTHNG_API
    dbg.printf("%s\r\n\r\n", req.c_str());
#endif

    TCPSocketConnection socket;

    out.clear();

    string res;

    if (socket.connect(host.c_str(), port) == 0) {

        char* snd = new char[req.size()+1];
        req.copy(snd, req.size());
        snd[req.size()]='\0';
        bool sent = socket.send_all(snd, req.size()) >= 0;
        delete[] snd;

        if (sent) {

            char rcv[256];

            int r;
            while (true) {
                r = socket.receive(rcv, sizeof(rcv));
                if (r <= 0)
                    break;
                res.append(rcv, r);
            }

            ret = EVRY_ERR_OK;

        } else {
            ret = EVRY_ERR_CANTSEND;
        }

        socket.close();

    } else {
        ret = EVRY_ERR_CANTCONNECT;
    }
    
#ifdef DEBUG_EVRYTHNG_API
    dbg.printf("%s", res.c_str());
#endif

    if (res.compare(0,5,"HTTP/") != 0) {
        return EVRY_ERR_UNKNOWN;
    }

    int spPos = res.find(' ', 5);
    if (spPos == string::npos) {
        return EVRY_ERR_UNKNOWN;
    }

    // TODO: check str length
    int code = atoi(res.c_str()+spPos+1);
    if (code < 100 || code > 999) {
        return EVRY_ERR_UNKNOWN;
    }
    codeOut = code;

    int startContent = res.find("\r\n\r\n");
    if (startContent != string::npos) {
        out.append(res.substr(startContent+4,res.size()-startContent-4));
    }

    return ret;
}

int EvrythngApi::httpPut(const string& path, const string& json, string& out, int& codeOut)
{
    return httpRequest(PUT, path, json, out, codeOut);
}

int EvrythngApi::httpGet(const string& path, string& out, int& codeOut)
{
    return httpRequest(GET, path, "", out, codeOut);
}

int EvrythngApi::httpPost(const string& path, const string& json, string& out, int& codeOut)
{
    return httpRequest(POST, path, json, out, codeOut);
}

int EvrythngApi::httpDelete(const string& path, string& out, int& codeOut)
{
    return httpRequest(DELETE, path, "", out, codeOut);
}