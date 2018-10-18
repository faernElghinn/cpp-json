/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "json.h"

#include <elladan/Exception.h>
#include <elladan/Stringify.h>
#include <stdlib.h>
#include <cstring>
#include <sstream>
#include <utility>
#include <cassert>

#include "serializer/BsonSerializer.h"
#include "serializer/JsonSerializer.h"

bool operator !=(const elladan::json::Json_t& left, const elladan::json::Json_t& right) {
    if (!left.get() && !right.get()) return false;
    if (!left.get() ^ !right.get()) return true;
    return left->cmp(right.get()) != 0;
}

bool operator ==(const elladan::json::Json_t& left, const elladan::json::Json_t& right) {
    if (!left.get() && !right.get()) return true;
    if (!left.get() ^ !right.get()) return false;
    return left->cmp(right.get()) == 0;
}

bool operator <(const elladan::json::Json_t& left, const elladan::json::Json_t& right) {
    if (!left.get() && !right.get()) return false;
    if (!left.get() || !right.get()) return !left.get();
    return left->cmp(right.get()) < 0;
}

bool operator >(const elladan::json::Json_t& left, const elladan::json::Json_t& right) {
    if (!left.get() && !right.get()) return false;
    if (!left.get() || !right.get()) return !right.get();
    return left->cmp(right.get()) > 0;
}

namespace elladan { namespace json {

Json::Json() {
}
Json::~Json() {
}

// - * Match any (map to .*)
// - ** skip any number of level.
static void getChildInternal (const Json_t& ele, size_t deepness, std::vector<std::string> parts, std::vector<Json_t>& retVal) {
    /*
     *
     * The objective:
     * We got a path, ie : /some/0/**\/child/*\/amigo/\**
     * We need to traverse json tree from the current object to the targets defined bu the path.
     * There are some edge case :
     * - all path start by "/".
     *   - Unless it's an empty string, in which case return the current item. => Generalise to empty == current item.
     * - The numbers are either object key, or array idx.
     * - A single * mean any value;
     * - A double ** mean any value, recursive.
     *
     * As this function is recursive, use deepness to know where we are in the path
     */


    // No element? don't process
    if (!ele) {}

    // We are past the path parts : this is the one we are looking for.
    else if (deepness >= parts.size())
        retVal.push_back(ele);

    // Got an object.
    else if (ele->getType() == json::JSON_OBJECT) {
        for (auto ite : ele->toObject()->value) {
            bool match;
            int inc = 1;

            // The complicated case : recursive any.
            if (parts[deepness] == "**") {
                // The current element always match path.
                match = true;

                // Increase deepness only if the child match the following element.
                inc = 2*(deepness + 1 < parts.size() && parts[deepness+1] == ite.first);
            }

            // Else select this item if it match the looked for name (or "any").
            else
                match = parts[deepness] == "*" || parts[deepness] == ite.first;

            if (match)
                getChildInternal(ite.second, deepness+inc, parts, retVal);
        }
    }

    // The array is the same as object, just with native int as index.
    else if (ele->getType() == json::JSON_ARRAY) {
        for (int i = 0; i < ele->toArray()->value.size(); i++) {
            bool match;
            int inc = 1;

            if (parts[deepness] == "**") {
                match = true;
                inc = 2*(deepness + 1 < parts.size() && parts[deepness+1] == std::to_string(i));
            }
            else
                match = parts[deepness] == "*" || parts[deepness] == std::to_string(i);

            if (match)
                getChildInternal(ele->toArray()->value[i], deepness+inc, parts, retVal);
        }
    }
}

std::vector<Json_t> Json::getChild(const Json_t& ele, const std::string& path){
    std::vector<Json_t> retVal;
    getChildInternal(ele, 1, tokenize(path, "/"), retVal);
    return retVal;
}

JsonType Json::getType() const {
    return JSON_NONE;
}
int Json::cmp(const Json* rhs) const {
    if (getType() != rhs->getType())
        return getType() < rhs->getType() ? -1 : 1;
    return 0;
}
Json_t Json::deep_copy() const {
    return std::make_shared<Json>();
}

void Json::write(std::ostream* out, EncodingOption flags, StreamFormat format){
    switch (format) {
        case StreamFormat::JSON:    JsonSerializer::write(out, this, flags);            break;
        case StreamFormat::BSON:    BsonSerializer::write(out, this, flags);            break;
        default:                    throw Exception("Unknown stream format");
    }
}

Json_t Json::read(std::istream* input, DecodingOption flags, StreamFormat format){
    switch (format) {
        case StreamFormat::JSON:     return JsonSerializer::read(input, flags);
        case StreamFormat::BSON:    return BsonSerializer::read(input, flags);
        default:                    throw Exception("Unknown stream format");
    }
}

std::vector<Json_t> Json::extract(std::istream* input, DecodingOption flags, StreamFormat format, const std::string& path){
    switch (format) {
        case StreamFormat::JSON:     return JsonSerializer::extract(input, flags, path);
        case StreamFormat::BSON:    return BsonSerializer::extract(input, flags, path);
        default:                    throw Exception("Unknown stream format");
    }
}


#define TO(Type, TYPE) \
Json##Type* Json::to##Type() { assert(getType() == TYPE); return static_cast<Json##Type*>(this); }\
const Json##Type* Json::to##Type() const { assert(getType() == TYPE); return static_cast<const Json##Type*>(this); }
TO(Bool  , JSON_BOOL);
TO(Int   , JSON_INTEGER);
TO(Double, JSON_DOUBLE);
TO(String, JSON_STRING);
TO(Array , JSON_ARRAY);
TO(Object, JSON_OBJECT);
TO(Binary, JSON_BINARY);
TO(UUID  , JSON_UUID);
#undef TO


JsonType JsonNull::getType() const {
    return JSON_NULL;
}

Json_t JsonNull::deep_copy() const{
    return std::make_shared<JsonNull>();
}


JsonBool::JsonBool() : Json(), value(false) {}
JsonBool::JsonBool(bool val): Json(), value(val) {}
JsonType JsonBool::getType() const {
    return JSON_BOOL;
}
int JsonBool::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;
    if (value ^ ((JsonBool*) right)->value) return value ? -1 : 1;
    return 0;
}
Json_t JsonBool::deep_copy() const{
    return std::make_shared<JsonBool>(value);
}


JsonInt::JsonInt() : Json(), value(0) {}
JsonInt::JsonInt(long int val): Json(), value(val) {}

JsonType JsonInt::getType() const {
    return JSON_INTEGER;
}
int JsonInt::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;
    if (value < ((JsonInt*) right)->value) return -1;
    return value != ((JsonInt*) right)->value ;
}
Json_t JsonInt::deep_copy() const{
    return std::make_shared<JsonInt>(value);
}

JsonDouble::JsonDouble() : Json(), value(0) {}
JsonDouble::JsonDouble(double val): Json(), value(val) {}

JsonType JsonDouble::getType() const {
    return JSON_DOUBLE;
}
int JsonDouble::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;
    if (value < ((JsonDouble*) right)->value) return -1;
    return value != ((JsonDouble*) right)->value ;
}
Json_t JsonDouble::deep_copy() const{
    return std::make_shared<JsonDouble>(value);
}


JsonString::JsonString() : Json() {}
JsonString::JsonString(const std::string& val): Json(), value(val) {}

JsonType JsonString::getType() const {
    return JSON_STRING;
}
int JsonString::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;
    return value.compare(((JsonString*) right)->value);
}
Json_t JsonString::deep_copy() const{
    return std::make_shared<JsonString>(value);
}

JsonType JsonArray::getType() const {
    return JSON_ARRAY;
}
int JsonArray::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;

    JsonArray* oth = (JsonArray*) right;
    if (value.size() != oth->value.size())
        return value.size() < oth->value.size() ? -1 : 1;

    for (int i = 0; i < value.size(); i++) {
        retVal = value.at(i)->cmp(oth->value.at(i).get());
        if (retVal != 0) return retVal;
    }

    return 0;
}
Json_t JsonArray::deep_copy() const{
    JsonArray_t array = std::make_shared<JsonArray>();
    for (auto ite : value)
        array->value.push_back(ite->deep_copy());
    return array;
}

JsonType JsonObject::getType() const {
    return JSON_OBJECT;
}
int JsonObject::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;

    JsonObject* oth = (JsonObject*) right;
    if (value.size() != oth->value.size())
        return value.size() < oth->value.size() ? -1 : 1;

    auto ite1 = value.begin();
    auto ite2 = oth->value.begin();
    for ( ; ite1 != value.end(), ite2 != oth->value.end(); ++ite1, ++ite2) {
        // If both name differ.
        int nameCmp = ite1->first.compare(ite2->first);
        if (nameCmp != 0)
            return nameCmp;

        retVal = ite1->second->cmp(ite2->second.get());
        if (retVal != 0) return retVal;
    }

    return 0;
}
Json_t JsonObject::deep_copy() const{
    JsonObject_t array = std::make_shared<JsonObject>();
    for (auto ite : value)
        array->value[ite.first] = ite.second->deep_copy();
    return array;
}




Binary::Binary() :
        size(0), data(nullptr) {
}
Binary::Binary(size_t s) :
        size(s) {
    data = malloc(size);
    assert(data != nullptr);
}
Binary::Binary(void* d, size_t s) :
        size(s), data(d) {
}
Binary::~Binary() {
    free(data);
}
Binary::Binary(const std::string& str) :
        Binary (str.size() / 2)
{
    const char* inp = str.c_str();
    char* dst = (char*)data;

    for (int i = 0; i < size; i++)
        *dst++ = (charToHex(*inp++, 16) << 4) + charToHex(*inp++, 16);
    // IMP: this drop the remaining odd char, if any.
}
std::string Binary::toHex() const{
    std::stringstream str;

    for (int i = 0; i < size; ++i)
        str << hexToChar(((char*)data)[i]);

    return str.str();
}
int Binary::cmp (const Binary* rhs) const{
    if (size != rhs->size) return size - rhs->size;
    return memcmp(data, rhs->data, size);
}
bool Binary::operator != (const Binary* rhs) const{ return cmp(rhs) != 0; }
bool Binary::operator  < (const Binary* rhs) const{ return cmp(rhs)  < 0; }
bool Binary::operator <= (const Binary* rhs) const{ return cmp(rhs) <= 0; }
bool Binary::operator == (const Binary* rhs) const{ return cmp(rhs) == 0; }
bool Binary::operator >= (const Binary* rhs) const{ return cmp(rhs) >= 0; }
bool Binary::operator  > (const Binary* rhs) const{ return cmp(rhs)  > 0; }



JsonType JsonBinary::getType() const {
    return JSON_BINARY;
}
int JsonBinary::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;

    JsonBinary* oth = (JsonBinary*) right;
    if (value.get() == oth->value.get()) return 0;
    if (!value) return -1;
    if (!oth->value) return 1;
    return value->cmp(oth->value.get());
}
Json_t JsonBinary::deep_copy() const{
    JsonBinary_t bin = std::make_shared<JsonBinary>();
    bin->value = std::make_shared<Binary>(value->size);
    memcpy(bin->value->data, value->data, value->size);
    return bin;
}
JsonBinary::JsonBinary() {}
JsonBinary::JsonBinary(Binary_t binary) {value = binary;}


JsonType JsonUUID::getType() const {
    return JSON_UUID;
}
int JsonUUID::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;

    JsonUUID* oth = (JsonUUID*) right;
    return value.cmp(oth->value);
}
Json_t JsonUUID::deep_copy() const{
    return std::make_shared<JsonUUID>(value);
}
JsonUUID::JsonUUID() {}
JsonUUID::JsonUUID(const elladan::UUID& uid) : value(uid) {}

void fromJson_imp (Json_t ele, std::string& out) {
    if (ele->getType() != JSON_STRING) throw Exception("Invalid format, expected JSON_STRING" );
    out = std::dynamic_pointer_cast<JsonString>(ele)->value;
}
void fromJson_imp (Json_t ele, json::Json_t& out) {
    out = ele;
}
void fromJson_imp (Json_t ele, Binary_t& out) {
    if (ele->getType() == JSON_BINARY)      out = std::dynamic_pointer_cast<JsonBinary>(ele)->value;
    else if (ele->getType() == JSON_STRING) out = std::make_shared<Binary>(std::dynamic_pointer_cast<JsonString>(ele)->value);
    else                                    throw Exception("Invalid format, expeced JSON_UUID");
}
void fromJson_imp (Json_t ele, elladan::UUID& out) {
    if (ele->getType() == JSON_UUID)        out = std::dynamic_pointer_cast<JsonUUID>(ele)->value;
    else if (ele->getType() == JSON_STRING) out = elladan::UUID::fromString(std::dynamic_pointer_cast<JsonString>(ele)->value);
    else                                    throw Exception("Invalid format, expeced JSON_UUID");
}

} } // namespace elladan::json

namespace std {


std::string to_string (elladan::json::Json_t val)
{
    if (!val) return "none";
    std::stringstream str;
    val->write(&str, elladan::json::EncodingFlags(), elladan::json::StreamFormat::JSON);
    return str.str();
}

std::string to_string (elladan::json::JsonType type){
    switch (type) {
        case elladan::json::JSON_NONE:      return "JsonNone";
        case elladan::json::JSON_NULL:      return "JsonNull";
        case elladan::json::JSON_BOOL:      return "JsonBool";
        case elladan::json::JSON_INTEGER:   return "JsonInt";
        case elladan::json::JSON_DOUBLE:    return "JsonDouble";
        case elladan::json::JSON_STRING:    return "JsonString";
        case elladan::json::JSON_ARRAY:     return "JsonArray";
        case elladan::json::JSON_OBJECT:    return "JsonObject";
        case elladan::json::JSON_BINARY:    return "JsonBinary";
    }
    return "JsonUnknown";
}

std::string to_string (elladan::json::Binary_t bin){
    if (!bin.get()) return "(NULL)";
    return bin->toHex();
}

}  // namespace std
