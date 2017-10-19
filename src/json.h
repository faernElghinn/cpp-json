#pragma once

#include <elladan/UUID.h>
#include <elladan/FlagSet.h>
#include <elladan/Exception.h>
#include <stddef.h>
#include <stdint.h>
#include <bitset>
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <vector>

namespace elladan {

class UUID;

namespace json {

enum JsonType {
    JSON_NONE,
    JSON_NULL,
    JSON_BOOL,
    JSON_INTEGER,
    JSON_DOUBLE,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
    JSON_BINARY,
    JSON_UUID,
};

enum DecodingFlags {
    DF_REJECT_DUPLICATE    = 1 << 0,
    DF_IGNORE_COMMENT      = 1 << 1,
    DF_ALLOW_NULL          = 1 << 2,
    DF_ALLOW_COMMA_ERR     = 1 << 3,
};
enum EncodingFlags {
    EF_JSON_ENSURE_ASCII   = 1 << 0,
    EF_JSON_ESCAPE_SLASH   = 1 << 1,
};
enum class StreamFormat : uint8_t {
    JSON = 0,
    BSON = 1
};

typedef FlagSet DecodingOption;

struct EncodingOption : public FlagSet
{
    EncodingOption() : real_prec(10), ident(0) {  }
    EncodingOption(EncodingFlags flag) : EncodingOption() {set((int)flag);}

    static constexpr int MAX_INDENT_AS_TAB = 15;
    void setIndent(int size){
        ident = size < 0 ?  0 : (size > MAX_INDENT_AS_TAB ? MAX_INDENT_AS_TAB : size);
    }
    int getIndent(){
        return ident;
    }

    static constexpr int MAX_FLOAT = 31;
    void setRealPrec(int size){
        size -= 1;
        real_prec = size < 0 ?  0 : (size > MAX_FLOAT ? MAX_FLOAT : size);
    }
    int getRealPrec(){
        return real_prec +1;
    }
    uint8_t real_prec;
    uint8_t ident;
};

#define DEF(Class) class Class;  typedef std::shared_ptr<Class> Class##_t
DEF(Json      );
DEF(JsonNull  );
DEF(JsonBool  );
DEF(JsonInt   );
DEF(JsonDouble);
DEF(JsonString);
DEF(JsonArray );
DEF(JsonObject);
DEF(JsonBinary);
DEF(JsonUUID);
#undef DEF

class Json
{
public:
    Json();
    virtual ~Json();

    static Json_t read(std::istream* input, DecodingOption flags, StreamFormat format);
    static std::vector<Json_t> extract(std::istream* input, DecodingOption flags, StreamFormat format, const std::string& path);
    void write(std::ostream* out, EncodingOption flags, StreamFormat format);
    static std::vector<Json_t> getChild(const Json_t& ele, const std::string& path);

    virtual JsonType getType() const;
    virtual int cmp (const Json* rigth) const;
    virtual Json_t deep_copy() const;
};


class JsonNull: public Json
{
public:
    JsonType getType() const;
    Json_t deep_copy() const;
    bool dump(std::ostream& out, size_t flags);
};

class JsonBool: public Json
{
public:
    JsonBool();
    JsonBool(bool val);
    JsonType getType() const;
    int cmp (const Json* right) const;
    Json_t deep_copy() const;
    bool getValue() const { return asBool; }

    bool asBool;
};

class JsonInt: public Json
{
public:
    JsonInt();
    JsonInt(long int val);
    JsonType getType() const;
    int cmp (const Json* right) const;
    Json_t deep_copy() const;
    int64_t getValue() const { return asInt; }

    int64_t asInt;
};

class JsonDouble: public Json
{
public:
    JsonDouble();
    JsonDouble(double val);
    JsonType getType() const;
    int cmp (const Json* right) const;
    Json_t deep_copy() const;
    double getValue() const { return asDouble; }

    double asDouble;
};

class JsonString: public Json
{
public:
    ~JsonString(){}
    JsonString();
    JsonString(const std::string& val);
    JsonType getType() const;
    int cmp (const Json* right) const;
    Json_t deep_copy() const;
    const std::string& getValue() const { return asString; }

    std::string asString;
};

class JsonArray: public Json, public std::vector<Json_t>
{
public:
    JsonType getType() const;
    int cmp (const Json* right) const;
    Json_t deep_copy() const;

    bool visited;
};

class JsonObject: public Json, public std::map<std::string, Json_t>
{
public:
    JsonType getType() const;
    int cmp (const Json* right) const;
    Json_t deep_copy() const;

    bool visited;
};

class Binary
{
public:
    Binary();
    virtual ~Binary();
    Binary(size_t size);
    Binary(void* data, size_t size);
    Binary(const std::string& str);

    std::string toHex() const;

    void* data;
    size_t size;
};
typedef std::shared_ptr<Binary> Binary_t;

class JsonBinary: public Json
{
public:
    JsonBinary();
    JsonBinary(Binary_t binary);
    JsonType getType() const;
    int cmp (const Json* right) const;
    Json_t deep_copy() const;
    Binary_t getValue() const { return asBinary; }

    Binary_t asBinary;
};

class JsonUUID: public Json
{
public:
    JsonUUID();
    JsonUUID(const elladan::UUID& uid);
    JsonType getType() const;
    int cmp (const Json* right) const;
    Json_t deep_copy() const;
    const elladan::UUID& getValue() const { return asUUID; }

    elladan::UUID asUUID;
};


struct cmpJson {
    bool operator()(const json::Json_t& lhs, const json::Json_t& rhs) const{
        return lhs < rhs;
    }
};

template<typename T>
typename std::enable_if<std::is_same<T, bool>::value, Json_t>::type
toJson_imp(T val) {
    return std::make_shared<json::JsonBool>(val);
}
template<typename T>
typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, Json_t>::type
toJson_imp(T val) {
    return std::make_shared<json::JsonInt>(val);
}
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, Json_t>::type
toJson_imp(T val) {
    return std::make_shared<json::JsonDouble>(val);
}
template<typename T>
typename std::enable_if<std::is_enum<T>::value, Json_t>::type
toJson_imp(T val) {
    typedef typename std::underlying_type<T>::type U;
    return toJson_imp<U>((U)val);
}
template <typename T>
Json_t toJson_imp (const std::string& val){
    return std::make_shared<json::JsonString>(val);
}
template <typename T>
Json_t toJson_imp (const char* val){
    return std::make_shared<json::JsonString>(std::string(val));
}
template <typename T>
Json_t toJson_imp (json::Json_t val){
    return val;
}
template <typename T>
Json_t toJson_imp (Binary_t& val){
    return std::make_shared<json::JsonBinary>(val);
}
template <typename T>
Json_t toJson_imp (const elladan::UUID& val){
    return std::make_shared<json::JsonUUID>(val);
}

template <typename T>
Json_t toJson(T value){
    return toJson_imp<T>(value);
}


template<typename T>
typename std::enable_if<std::is_same<T, bool>::value, void>::type
fromJson_imp(Json_t ele, T& out) {
    if (ele->getType() != JSON_BOOL) throw Exception("Invalid format, expected JSON_BOOL" );
    out = std::dynamic_pointer_cast<JsonBool>(ele)->asBool;
}
template<typename T>
typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, void>::type
fromJson_imp(Json_t ele, T& out) {
    if (ele->getType() != JSON_INTEGER) throw Exception("Invalid format, expected JSON_INTEGER" );
    out = std::dynamic_pointer_cast<JsonInt>(ele)->asInt;
}
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
fromJson_imp(Json_t ele, T& out) {
    if (ele->getType() != JSON_DOUBLE) throw Exception("Invalid format, expected JSON_DOUBLE" );
    out= std::dynamic_pointer_cast<JsonDouble>(ele)->asDouble;
}
template<typename T>
typename std::enable_if<std::is_enum<T>::value, void>::type
fromJson_imp(Json_t ele, T& out) {
    if (ele->getType() != JSON_INTEGER) throw Exception("Invalid format, expected JSON_INTEGER" );
    out = (T)std::dynamic_pointer_cast<JsonInt>(ele)->asInt;
}
void fromJson_imp (Json_t ele, json::Json_t& out);
void fromJson_imp (Json_t ele, std::string& out);
void fromJson_imp (Json_t ele, Binary_t& out);
void fromJson_imp (Json_t ele, elladan::UUID& out);

template <typename T>
T fromJson(Json_t ele){
    T out;
    fromJson_imp(ele, out);
    return out;
}


} } // namespace elladan::json

std::string toString (elladan::json::Json_t val);
std::string toString (elladan::json::JsonType type);


// Those does not work: smart_ptr simply compare the pointer and ignore those
bool operator != (const elladan::json::Json_t& left, const elladan::json::Json_t& right);
bool operator == (const elladan::json::Json_t& left, const elladan::json::Json_t& right);
bool operator < (const elladan::json::Json_t& left, const elladan::json::Json_t& right);
bool operator > (const elladan::json::Json_t& left, const elladan::json::Json_t& right);

#define CMP_JSON(TypeA, TypeB) \
    inline bool operator != (const elladan::json::TypeA& left, const elladan::json::TypeB& right){\
        return std::static_pointer_cast<elladan::json::Json>(left) != std::static_pointer_cast<elladan::json::Json>(right);\
    }\
    inline bool operator == (const elladan::json::TypeA& left, const elladan::json::TypeB& right){\
        return std::static_pointer_cast<elladan::json::Json>(left) == std::static_pointer_cast<elladan::json::Json>(right);\
    }\
    inline bool operator < (const elladan::json::TypeA& left, const elladan::json::TypeB& right){\
        return std::static_pointer_cast<elladan::json::Json>(left) < std::static_pointer_cast<elladan::json::Json>(right);\
    }\
    inline bool operator > (const elladan::json::TypeA& left, const elladan::json::TypeB& right){\
        return std::static_pointer_cast<elladan::json::Json>(left) > std::static_pointer_cast<elladan::json::Json>(right);\
    }

#define CMP_WITH_ALL_JSON(Type)\
    CMP_JSON(Json_t, Type) \
    CMP_JSON(Type, Json_t) \
    CMP_JSON(Type, JsonNull_t)\
    CMP_JSON(Type, JsonBool_t)\
    CMP_JSON(Type, JsonInt_t)\
    CMP_JSON(Type, JsonDouble_t)\
    CMP_JSON(Type, JsonArray_t)\
    CMP_JSON(Type, JsonObject_t)\
    CMP_JSON(Type, JsonBinary_t) \
    CMP_JSON(Type, JsonUUID_t)

CMP_WITH_ALL_JSON(JsonNull_t)
CMP_WITH_ALL_JSON(JsonBool_t)
CMP_WITH_ALL_JSON(JsonInt_t)
CMP_WITH_ALL_JSON(JsonDouble_t)
CMP_WITH_ALL_JSON(JsonArray_t)
CMP_WITH_ALL_JSON(JsonObject_t)
CMP_WITH_ALL_JSON(JsonBinary_t)
CMP_WITH_ALL_JSON(JsonUUID_t)

#undef CMP_WITH_ALL_JSON
#undef CMP_JSON

