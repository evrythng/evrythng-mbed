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
#include "JsonParser.h"
#include "string.h"
#include "stdlib.h"
#include "errno.h"
#include "util.h"

//#define DEBUG_JSONTOKENIZER
//#define DEBUG_JSONPARSER
//#define DEBUG_JSONGET

using namespace std;

bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

JsonParser::JsonParser()
{
    this->pDocument = NULL;
}

JsonParser::~JsonParser()
{
    if (this->pDocument) {
        delete this->pDocument;
    }
}

int JsonParser::parse(const char* json)
{
    this->json = json;
    this->json_len = strlen(json);

    if (this->pDocument) {
        delete this->pDocument;
        this->pDocument = NULL;
    }

    ctPos = 0;
    ctLen = 0;
    if (goToNextToken() != 0) return -1;
    bool ok = true;
    if (json[ctPos] == '{') {
        if (parseObject(&pDocument) != 0) {
            ok = false;
        }
    } else {
        if (parseArray(&pDocument) != 0) ok = false;
    }
    if (ct != TOKEN_EOS) {
        ok = false;
    }

    if (!ok) {
        delete pDocument;
        pDocument = NULL;
        return -1;
    }

    return 0;
}

JsonValue* JsonParser::getDocument()
{
    return pDocument;
}

int JsonParser::goToNextToken()
{

#ifdef DEBUG_JSONTOKENIZER

    dbg.printf("Token: ");

#endif

    ctPos += ctLen;

    // Skip whitespaces
    while (ctPos < json_len &&
            (json[ctPos] == ' ' || json[ctPos] == '\t' ||
             json[ctPos] == '\r' || json[ctPos] == '\n'))
        ++ctPos;

    if (ctPos < json_len) {
        if (json[ctPos] == '"') {
            ct = TOKEN_STRING;
            int i = ctPos+1;
            while (i < json_len && json[i] != '"') {
                if (json[i] == '\\') {
                    if (i+1 < json_len) {
                        switch (json[i+1]) {
                            case '\\':
                            case '"':
                            case '/':
                            case 'b':
                            case 'f':
                            case 'n':
                            case 'r':
                            case 't':
                                ++i;
                                break;
                            case 'u':
                                i+= 5;
                                break;
                            default:
                                return -1;
                        }
                    } else {
                        return -1;
                    }
                }
                ++i;
            }
            if (i >= json_len || json[i] != '"') return -1;
            ctLen = i - ctPos + 1;
        } else if (isDigit(json[ctPos]) || json[ctPos] == '-') {
            ct = TOKEN_NUMBER;
            char* e;
            errno = 0;
            ctNumberVal = strtod(json+ctPos, &e);
            if (errno || e - json <= 0) return -1;
            ctLen = (e - json) - ctPos;

        } else if (strncmp(json+ctPos,"true",4) == 0) {
            ct = TOKEN_TRUE;
            ctLen = 4;
        } else if (strncmp(json+ctPos,"false",5) == 0) {
            ct = TOKEN_FALSE;
            ctLen = 5;
        } else if (strncmp(json+ctPos,"null",4) == 0) {
            ct = TOKEN_NULL;
            ctLen = 4;
        } else {
            ct = TOKEN_DELIMITER;
            ctLen = 1;
        }
    } else {
        ct = TOKEN_EOS;
        ctLen = 0;
    }

#ifdef DEBUG_JSONTOKENIZER

    switch (ct) {
        case TOKEN_DELIMITER:
            dbg.printf("Delimtier - ");
            break;
        case TOKEN_EOS:
            dbg.printf("End of stream");
            break;
        case TOKEN_NUMBER:
            dbg.printf("Number %g - ", ctNumberVal);
            break;
        case TOKEN_STRING:
            dbg.printf("String - ");
            break;
        case TOKEN_FALSE:
            dbg.printf("False - ");
            break;
        case TOKEN_TRUE:
            dbg.printf("True - ");
            break;
        case TOKEN_NULL:
            dbg.printf("Null - ");
            break;
    }
    if (ct != TOKEN_EOS) {
        for (int i = 0; i < ctLen; ++i)
            dbg.printf("%c", json[ctPos+i]);
    }
    dbg.printf(" (%d,%d)\r\n", ctPos, ctLen);

#endif

    return 0;
}

int JsonParser::parseObject(JsonValue** object)
{

#ifdef DEBUG_JSONPARSER
    dbg.printf("Enter parseObject\r\n");
#endif

    *object = JsonValue::createMap();
    map<string,JsonValue*>* m = (*object)->value.map;

    if (ct != TOKEN_DELIMITER || json[ctPos] != '{') return -1;
    if (goToNextToken() != 0) return -1;

    if (ct == TOKEN_STRING) {

        string key;
        key.assign(json+ctPos+1, ctLen-2);

        if (goToNextToken() != 0) return -1;

        if (ct != TOKEN_DELIMITER || json[ctPos] != ':') return -1;
        if (goToNextToken() != 0) return -1;

        JsonValue* pValue;
        if (parseValue(&pValue) != 0) {
            delete pValue;
            return -1;
        }
        (*m)[key] = pValue;

        while (ct == TOKEN_DELIMITER && json[ctPos] == ',') {
            if (goToNextToken() != 0) return -1;

            if (ct != TOKEN_STRING) return -1;
            key.assign(json+ctPos+1, ctLen-2);
            if (goToNextToken() != 0) return -1;

            if (ct != TOKEN_DELIMITER || json[ctPos] != ':') return -1;
            if (goToNextToken() != 0) return -1;

            if (parseValue(&pValue) != 0) {
                delete pValue;
                return -1;
            }
            (*m)[key] = pValue;
        }
    }

    if (ct != TOKEN_DELIMITER || json[ctPos] != '}') return -1;
    if (goToNextToken() != 0) return -1;

#ifdef DEBUG_JSONPARSER
    dbg.printf("Exit parseObject\r\n");
#endif
    return 0;
}

int JsonParser::parseValue(JsonValue** value)
{

#ifdef DEBUG_JSONPARSER
    dbg.printf("Enter parseValue\r\n");
#endif

    switch (ct) {
        case TOKEN_STRING:
            *value = JsonValue::createString(json+ctPos+1,ctLen-2);
            if (goToNextToken() != 0) return -1;
            break;
        case TOKEN_NUMBER:
            *value = JsonValue::createDouble(ctNumberVal);
            if (goToNextToken() != 0) return -1;
            break;
        case TOKEN_NULL:
            *value = JsonValue::createNull();
            if (goToNextToken() != 0) return -1;
            break;
        case TOKEN_FALSE:
            *value = JsonValue::createBoolean(false);
            if (goToNextToken() != 0) return -1;
            break;
        case TOKEN_TRUE:
            *value = JsonValue::createBoolean(true);
            if (goToNextToken() != 0) return -1;
            break;
        case TOKEN_DELIMITER:
            if (json[ctPos] == '{') {
                if (parseObject(value) != 0) return -1;
            } else if (json[ctPos] == '[') {
                if (parseArray(value) != 0) return -1;
            }
            break;
        default:
            *value = JsonValue::createNull();
            return -1;
    }

#ifdef DEBUG_JSONPARSER
    dbg.printf("Exit parseValue\r\n");
#endif

    return 0;
}

int JsonParser::parseArray(JsonValue** array)
{

#ifdef DEBUG_JSONPARSER
    dbg.printf("Enter parseArray\r\n");
#endif

    *array = JsonValue::createVector();
    vector<JsonValue*>* vec = (*array)->value.vec;

    if (ct != TOKEN_DELIMITER || json[ctPos] != '[') return -1;
    if (goToNextToken() != 0) return -1;

    if (ct != TOKEN_DELIMITER || json[ctPos] != ']') {

        JsonValue* pValue;
        if (parseValue(&pValue) != 0) {
            delete pValue;
            return -1;
        };
        vec->push_back(pValue);

        while (ct == TOKEN_DELIMITER && json[ctPos] == ',') {
            if (goToNextToken() != 0) return -1;

            if (parseValue(&pValue) != 0) {
                delete pValue;
                return -1;
            };
            vec->push_back(pValue);
        }
    }

    if (ct != TOKEN_DELIMITER || json[ctPos] != ']') return -1;
    if (goToNextToken() != 0) return -1;

#ifdef DEBUG_JSONPARSER
    dbg.printf("Exit parseArray\r\n");
#endif

    return 0;
}

int inst = 0;

JsonValue::JsonValue(const char* buffer, int len)
{
    type = VT_CHAR_PTR;
    value.s = new char[len+1];
    strncpy(value.s, buffer, len);
    value.s[len] = '\0';
}

JsonValue::JsonValue(double d)
{
    type = VT_DOUBLE;
    value.d = d;
}

JsonValue::JsonValue(bool b)
{
    type = b ? VT_CST_TRUE : VT_CST_FALSE;

}

JsonValue::JsonValue()
{
}

JsonValue::~JsonValue()
{
    switch (type) {
        case VT_CHAR_PTR:
            delete[] value.s;
            break;
        case VT_MAP_PTR:
            for (map<string,JsonValue*>::iterator itr = value.map->begin(); itr != value.map->end(); itr++)
                delete ((*itr).second);
            delete value.map;
            break;
        case VT_VEC_PTR:
            for (vector<JsonValue*>::iterator itr = value.vec->begin(); itr != value.vec->end(); itr++)
                delete (*itr);
            delete value.vec;
            break;
        default:
            break;
    }
}

JsonValue* JsonValue::createString(const char* buffer, int len)
{
    return new JsonValue(buffer, len);
}

JsonValue* JsonValue::createDouble(double d)
{
    return new JsonValue(d);
}

JsonValue* JsonValue::createBoolean(bool b)
{
    return new JsonValue(b);
}

JsonValue* JsonValue::createMap()
{
    JsonValue* ret = new JsonValue();
    ret->type = VT_MAP_PTR;
    ret->value.map = new map<string,JsonValue*>();
    return ret;
}

JsonValue* JsonValue::createVector()
{
    JsonValue* ret = new JsonValue();
    ret->type = VT_VEC_PTR;
    ret->value.vec = new vector<JsonValue*>();
    return ret;
}

JsonValue* JsonValue::createNull()
{
    JsonValue* ret = new JsonValue();
    ret->type = VT_CST_NULL;
    return ret;
}

void JsonValue::print()
{
    bool c = false;
    switch (type) {
        case VT_CHAR_PTR:
            dbg.printf("\"%s\"", value.s);
            break;
        case VT_MAP_PTR:
            dbg.printf("{");
            for (map<string,JsonValue*>::iterator itr = value.map->begin(); itr != value.map->end(); itr++) {
                if (c) dbg.printf(",");
                else c = true;
                dbg.printf("\"%s\":",(*itr).first.c_str());
                (*itr).second->print();
            }
            dbg.printf("}");
            break;
        case VT_VEC_PTR:
            dbg.printf("[");
            for (vector<JsonValue*>::iterator itr = value.vec->begin(); itr != value.vec->end(); itr++) {
                if (c) dbg.printf(",");
                else c = true;
                (*itr)->print();
            }
            dbg.printf("]");
            break;
        case VT_DOUBLE:
            dbg.printf("%g", value.d);
            break;
        case VT_CST_TRUE:
            dbg.printf("true");
            break;
        case VT_CST_FALSE:
            dbg.printf("false");
            break;
        case VT_CST_NULL:
            dbg.printf("null");
            break;
        default:
            break;
    }
}

JsonValue* JsonValue::get(const char* path)
{
    JsonValue* pValue = this;
    int pos = 0;
    while (path != NULL && path[pos] != '\0') {

#ifdef DEBUG_JSONGET
        dbg.printf("::get ");
        pValue->print();
        dbg.printf("\r\n");
#endif

        const char* start = path+pos;
        const char* pSl = strchr(start, '/');
        int len = pSl == NULL ? strlen(start) : pSl - start;
        if (len <= 0) return NULL;
        if (pValue->type == VT_VEC_PTR) {
            int v = atoi(start);
            if (v == 0 && path[pos] != '0') {
                return NULL;
            }
            if (v < 0 || v >= pValue->value.vec->size()) {
                return NULL;
            }
            pValue = (*pValue->value.vec)[v];
        } else if (pValue->type == VT_MAP_PTR) {
            char* pKey = new char[len+1];
            strncpy(pKey, start, len);
            pKey[len] = '\0';
            pValue = (*pValue->value.map)[pKey];
            delete[] pKey;
            if (pValue == NULL) {
                return NULL;
            }
        } else {
            return NULL;
        }
        pos += len;
        if (pSl) ++pos;
    }

#ifdef DEBUG_JSONGET
        dbg.printf("::get ");
        pValue->print();
        dbg.printf("\r\n");
#endif

    return pValue;
}

const double* JsonValue::getDouble(const char* path)
{
    JsonValue* pV = get(path);
    if (pV != NULL && pV->type == VT_DOUBLE) {
        return &(pV->value.d);
    } else {
        return NULL;
    }
}

const char* JsonValue::getString(const char* path)
{
    JsonValue* pV = get(path);
    if (pV != NULL && pV->type == VT_CHAR_PTR) {
        return pV->value.s;
    } else {
        return NULL;
    }
}
const std::vector<JsonValue*>* JsonValue::getVector(const char* path)
{
    JsonValue* pV = get(path);
    if (pV != NULL && pV->type == VT_VEC_PTR) {
        return pV->value.vec;
    } else {
        return NULL;
    }
}
const std::map<std::string,JsonValue*>* JsonValue::getMap(const char* path)
{
    JsonValue* pV = get(path);
    if (pV != NULL && pV->type == VT_MAP_PTR) {
        return pV->value.map;
    } else {
        return NULL;
    }
}

static const bool FALSE = false;
static const bool TRUE = true;

const bool* JsonValue::getBoolean(const char* path)
{
    JsonValue* pV = get(path);
    if (pV != NULL && (pV->type == VT_CST_TRUE || pV->type == VT_CST_FALSE)) {
        return (pV->type == VT_CST_TRUE) ? &TRUE : &FALSE;
    } else {
        return NULL;
    }
}
bool JsonValue::isNull(const char* path)
{
    JsonValue* pV = get(path);
    return (pV != NULL) && (pV->type == VT_CST_NULL);
}

