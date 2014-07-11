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
#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <vector>
#include <map>
#include <string>

/*
 * Possible JSON value types
 */
enum ValueType {

    VT_DOUBLE,   // Number
    VT_CHAR_PTR, // String
    VT_VEC_PTR,  // Array
    VT_MAP_PTR,  // Object
    VT_CST_NULL, // Null
    VT_CST_TRUE, // True
    VT_CST_FALSE // False
};

/*
 * Class to hold a JSON value. A JSON value may be composed of other JSON
 * values.
 */
class JsonValue
{
public:

    /*
     * Creates a new string JSON value. The string is copied into the object.
     * WARNING: a new object is created. The caller is responsible of deleting
     * it.
     * buffer: the string to read from.
     * len: the number of characters to read.
     */
    static JsonValue* createString(const char* buffer, int len);
    
    /* Creates a new number JSON value.
     * WARNING: a new object is created. The caller is responsible of deleting
     * it.
     */
    static JsonValue* createDouble(double d);

    /* Creates a new object JSON value.
     * WARNING: a new object is created. The caller is responsible of deleting
     * it.
     */
    static JsonValue* createMap();

    /* Creates a new array JSON value.
     * WARNING: a new object is created. The caller is responsible of deleting
     * it.
     */
    static JsonValue* createVector();
    
    /* Creates a new true or false JSON value, depending on b.
     * WARNING: a new object is created. The caller is responsible of deleting
     * it.
     */
    static JsonValue* createBoolean(bool b);
    
    /* Creates a new null JSON value.
     * WARNING: a new object is created. The caller is responsible of deleting
     * it.
     */
    static JsonValue* createNull();

    /* 
     * Destructor.
     * The destructor also deletes nested JsonValues if any.
     */
    virtual ~JsonValue();

    /*
     * Gets a JSON value given a path.
     * If the path is NULL or an empty string the method returns this.
     * If there is an error, the method returns NULL.
     * The path is composed of strings (object keys) and numbers (array 
     * indices) separated by '/'.
     *
     * Example:
     *   JSON: {"xy":34,"ab":[{"r":true},{"s":false}]}
     *   Path: ab/1/s
     *   Returns: a JsonValue representing false.
     */
    JsonValue* get(const char* path);
    
    /*
     * Gets a pointer to a double given a path (see get()).
     * If there is an error, the method returns NULL.
     */
    const double* getDouble(const char* path);
    
    /*
     * Gets a pointer to a string given a path (see get()).
     * If there is an error, the method returns NULL.
     */
    const char* getString(const char* path);
    
    /*
     * Gets a pointer to an array given a path (see get()).
     * If there is an error, the method returns NULL.
     */
    const std::vector<JsonValue*>* getVector(const char* path);

    /*
     * Gets a pointer to a map given a path (see get()).
     * If there is an error, the method returns NULL.
     */
    const std::map<std::string,JsonValue*>* getMap(const char* path);

    /*
     * Gets a pointer to a boolean given a path (see get()).
     * If there is an error, the method returns NULL.
     */
    const bool* getBoolean(const char* path);

    /*
     * Determines if the value at the path (see get()) is null.
     * Return true if and only if the value is explicitely null.
     */
    bool isNull(const char* path);

    /*
     * Debug function.
     */
    void print();

    friend class JsonParser;

private:

    /*
     * Constructor that creates a string JSON value. The string is copied into
     * the object.
     * buffer: the string to read from.
     * len: the number of characters to read.
     */
    JsonValue(const char* buffer, int len);
    
    /*
     * Constructor that creates a number JSON value.
     */
    JsonValue(double d);
    
    /*
     * Constructor that creates either a True or a False JSON value.
     */
    JsonValue(bool b);
    
    /*
     * Constructor that creates a JSON value without specifying the type.
     */
    JsonValue();
    
    ValueType type;
    union {
        double d;
        char* s;
        std::vector<JsonValue*>* vec;
        std::map<std::string,JsonValue*>* map;
    } value;

};

enum TokenType {
    TOKEN_EOS,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_DELIMITER,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL
};

/*
 * Class to parse a JSON string.
 *
 * NOTE: The current implementation does only support ASCII encoding.
 */
class JsonParser
{
public:

    /*
     * Constructor.
     */
    JsonParser();
    
    /*
     * Destructor.
     * The destructor also deletes the parsed document (the one you get with
     * getDocument().
     */
    virtual ~JsonParser();
    
    /*
     * Parses the json string.
     * json: String that contains the json to parse.
     * Returns 0 on success, or -1 on error.
     *
     * NOTE: This method deletes the previously parsed document.
     */
    int parse(const char* json);
    
    /*
     * Returns the parsed document, or NULL if the json is not parsed.
     */
    JsonValue* getDocument();

private:
    const char* json;
    int json_len;
    TokenType ct;
    int ctPos;
    int ctLen;
    double ctNumberVal;
    JsonValue* pDocument;

    int goToNextToken();

    int parseObject(JsonValue** object);
    int parseValue(JsonValue** value);
    int parseArray(JsonValue** array);
};

#endif