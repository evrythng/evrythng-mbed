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
#ifndef EVRYTHNGAPI_H
#define EVRYTHNGAPI_H

#include "EthernetInterface.h"
#include <string>

#include <stdint.h>

enum HttpMethod {
    GET, PUT, POST, DELETE
};


/*
 * Class to communicate with EVRYTHNG engine.
 */
class EvrythngApi
{
public:

    /*
     * Constructor
     */
    EvrythngApi(const std::string& token, const std::string& host = "api.evrythng.com", int port = 80);

    /*
     * Destructor
     */
    virtual ~EvrythngApi();
    
    /*
     * Reads the current value of a thng's property. The value read is put
     * in the value parameter.
     * Returns 0 on success, or an error code on error. Error codes are
     * described in evry_error.h.
     */
    int getThngPropertyValue(const std::string& thngId, const std::string& key, std::string& value);
    
    /*
     * Sets the value of a thng's property.
     * Returns 0 on success, or an error code on error. Error codes are
     * described in evry_error.h.
     */
    int setThngPropertyValue(const std::string& thngId, const std::string& key, const std::string& value, int64_t timestamp);
    
private:
    std::string token;
    std::string host;
    int port;
    
    int httpRequest(HttpMethod method, const std::string& path, const std::string& content, std::string& out, int& codeOut);
    
    int httpPut(const std::string& path, const std::string& json, std::string& out, int& codeOut);
    int httpGet(const std::string& path, std::string& out, int& codeOut);
    int httpPost(const std::string& path, const std::string& json, std::string& out, int& codeOut);
    int httpDelete(const std::string& path, std::string& out, int& codeOut);
};

#endif