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
static std::vector<Json_t> getChildInternal (const Json_t& ele, size_t deepness, std::vector<std::string> parts) {
    std::vector<Json_t> retVal;
    if (deepness > parts.size()){
        retVal.push_back(ele);
    }
    else if (ele->getType() == json::JSON_ARRAY) {
        for (auto ite : *std::dynamic_pointer_cast<json::JsonArray>(ele).get()){
            auto subRet = getChildInternal(ele, deepness, parts);
            retVal.insert(subRet.begin(), subRet.end(), retVal.end());
        }
    }
    else if (ele->getType() == json::JSON_OBJECT) {
        for (auto ite : *std::dynamic_pointer_cast<json::JsonObject>(ele).get()){
            bool match;
            bool inc = true;

            if (parts[deepness] == "**") {
                match = true;
                inc = false;
                deepness += (deepness + 1 < parts.size() && parts[deepness+1] == ite.first);
            }
            else {
                match = parts[deepness] == "*" || parts[deepness] == ite.first;
            }

            if (match && deepness+1 == parts.size()) {
                retVal.push_back(ite.second);
            }
            else if (match) {
                auto subRet = getChildInternal(ite.second, deepness+inc, parts);
                retVal.insert(retVal.end(), subRet.begin(), subRet.end());
            }
        }
    }
    return retVal;
}

std::vector<Json_t> Json::getChild(const Json_t& ele, const std::string& path){
    return getChildInternal(ele, 0, tokenize(path, "/"));
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
        case StreamFormat::JSON:     JsonSerializer::write(out, this, flags);            break;
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



JsonType JsonNull::getType() const {
    return JSON_NULL;
}

Json_t JsonNull::deep_copy() const{
    return std::make_shared<JsonNull>();
}


JsonBool::JsonBool() : Json(), asBool(false) {}
JsonBool::JsonBool(bool val): Json(), asBool(val) {}
JsonType JsonBool::getType() const {
    return JSON_BOOL;
}
int JsonBool::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;
    if (asBool ^ ((JsonBool*) right)->asBool) return asBool ? -1 : 1;
    return 0;
}
Json_t JsonBool::deep_copy() const{
    return std::make_shared<JsonBool>(asBool);
}


JsonInt::JsonInt() : Json(), asInt(0) {}
JsonInt::JsonInt(long int val): Json(), asInt(val) {}

JsonType JsonInt::getType() const {
    return JSON_INTEGER;
}
int JsonInt::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;
    if (asInt < ((JsonInt*) right)->asInt) return -1;
    return asInt != ((JsonInt*) right)->asInt ;
}
Json_t JsonInt::deep_copy() const{
    return std::make_shared<JsonInt>(asInt);
}

JsonDouble::JsonDouble() : Json(), asDouble(0) {}
JsonDouble::JsonDouble(double val): Json(), asDouble(val) {}

JsonType JsonDouble::getType() const {
    return JSON_DOUBLE;
}
int JsonDouble::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;
    if (asDouble < ((JsonDouble*) right)->asDouble) return -1;
    return asDouble != ((JsonDouble*) right)->asDouble ;
}
Json_t JsonDouble::deep_copy() const{
    return std::make_shared<JsonDouble>(asDouble);
}


JsonString::JsonString() : Json() {}
JsonString::JsonString(const std::string& val): Json(), asString(val) {}

JsonType JsonString::getType() const {
    return JSON_STRING;
}
int JsonString::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;
    return asString.compare(((JsonString*) right)->asString);
}
Json_t JsonString::deep_copy() const{
    return std::make_shared<JsonString>(asString);
}

JsonType JsonArray::getType() const {
    return JSON_ARRAY;
}
int JsonArray::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;

    JsonArray* oth = (JsonArray*) right;
    if (size() != oth->size())
        return size() < oth->size() ? -1 : 1;

    for (int i = 0; i < size(); i++) {
        retVal = at(i)->cmp(oth->at(i).get());
        if (retVal != 0) return retVal;
    }

    return 0;
}
Json_t JsonArray::deep_copy() const{
    JsonArray_t array = std::make_shared<JsonArray>();
    for (auto ite : *this)
        array->push_back(ite->deep_copy());
    return array;
}

JsonType JsonObject::getType() const {
    return JSON_OBJECT;
}
int JsonObject::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;

    JsonObject* oth = (JsonObject*) right;
    if (size() != oth->size())
        return size() < oth->size() ? -1 : 1;

    auto ite1 = begin();
    auto ite2 = oth->begin();
    for ( ; ite1 != end(), ite2 != oth->end(); ++ite1, ++ite2) {
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
    for (auto ite : *this)
        array->operator [](ite.first) = ite.second->deep_copy();
    return array;
}




Binary::Binary() :
        size(0), data(nullptr) {
}
Binary::Binary(size_t s) :
        size(s), data(malloc(size)) {
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

JsonType JsonBinary::getType() const {
    return JSON_BINARY;
}
int JsonBinary::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;

    JsonBinary* oth = (JsonBinary*) right;
    if (asBinary.get() == oth->asBinary.get()) return 0;
    if (!asBinary) return -1;
    if (!oth->asBinary) return 1;
    if (asBinary->size < oth->asBinary->size) return -1;
    if (asBinary->size > oth->asBinary->size) return 1;
    return memcmp(asBinary->data, oth->asBinary->data, asBinary->size);
}
Json_t JsonBinary::deep_copy() const{
    JsonBinary_t bin = std::make_shared<JsonBinary>();
    bin->asBinary = std::make_shared<Binary>(asBinary->size);
    memcpy(bin->asBinary->data, asBinary->data, asBinary->size);
    return bin;
}
JsonBinary::JsonBinary() {}
JsonBinary::JsonBinary(Binary_t binary) {asBinary = binary;}


JsonType JsonUUID::getType() const {
    return JSON_UUID;
}
int JsonUUID::cmp (const Json* right) const {
    int retVal = Json::cmp(right);
    if (retVal != 0) return retVal;

    JsonUUID* oth = (JsonUUID*) right;
    return asUUID.cmp(oth->asUUID);
}
Json_t JsonUUID::deep_copy() const{
    return std::make_shared<JsonUUID>(asUUID);
}
JsonUUID::JsonUUID() {}
JsonUUID::JsonUUID(const elladan::UUID& uid) : asUUID(uid) {}

template< >Json_t toJson<bool> (bool val){
    return std::make_shared<json::JsonBool>(val);
}
template< >Json_t toJson<float> (float val){
    return std::make_shared<json::JsonDouble>(val);
}
template< >Json_t toJson<double> (double val){
    return std::make_shared<json::JsonDouble>(val);
}
template< >Json_t toJson<const std::string&> (const std::string& val){
    return std::make_shared<json::JsonString>(val);
}
template< >Json_t toJson<std::string&> (std::string& val){
    return std::make_shared<json::JsonString>(val);
}
template< >Json_t toJson<const char*> (const char* val){
    return std::make_shared<json::JsonString>(val);
}
template< >Json_t toJson<char*> (char* val){
    return std::make_shared<json::JsonString>(val);
}
template< >Json_t toJson<std::string> (std::string val){
    return std::make_shared<json::JsonString>(val);
}
template< >Json_t toJson<int32_t> (int32_t val){
    return std::make_shared<json::JsonInt>(val);
}
template< >Json_t toJson<uint32_t> (uint32_t val){
    return std::make_shared<json::JsonInt>(val);
}
template< >Json_t toJson<int64_t> (int64_t val){
    return std::make_shared<json::JsonInt>(val);
}
template< > Json_t toJson<uint64_t> (uint64_t val){
    return std::make_shared<json::JsonInt>(val);
}
template< >Json_t toJson<json::Json_t> (json::Json_t val){
    return val;
}
template< >Json_t toJson<Binary_t> (Binary_t val){
    return std::make_shared<JsonBinary>(val);
}
template< >Json_t toJson<const elladan::UUID&> (const elladan::UUID& val){
    return std::make_shared<JsonUUID>(val);
}
template< >Json_t toJson<elladan::UUID> (elladan::UUID val){
    return std::make_shared<JsonUUID>(val);
}

#define FromJson(Type, JSON_TYPE, Value) \
template<> Type fromJson<Type> (Json_t ele){\
    if (ele->getType() != JSON_TYPE) throw Exception("Invalid format, expeced " #JSON_TYPE);\
    return std::dynamic_pointer_cast<Json##Value>(ele)->as##Value;\
}
FromJson(bool, JSON_BOOL, Bool);
FromJson(int64_t, JSON_INTEGER, Int);
FromJson(int32_t, JSON_INTEGER, Int);
FromJson(int16_t, JSON_INTEGER, Int);
FromJson(int8_t, JSON_INTEGER, Int);
FromJson(uint64_t, JSON_INTEGER, Int);
FromJson(uint32_t, JSON_INTEGER, Int);
FromJson(uint16_t, JSON_INTEGER, Int);
FromJson(uint8_t, JSON_INTEGER, Int);
FromJson(double, JSON_DOUBLE, Double);
FromJson(float, JSON_DOUBLE, Double);
FromJson(std::string, JSON_STRING, String);

template<> Json_t fromJson<Json_t> (Json_t value){
    return value;
}
template<> elladan::UUID fromJson<elladan::UUID> (Json_t ele){
   if (ele->getType() == JSON_UUID)
     return std::dynamic_pointer_cast<JsonUUID>(ele)->asUUID;
   else if (ele->getType() == JSON_STRING)
     return elladan::UUID::fromString(std::dynamic_pointer_cast<JsonString>(ele)->asString);
   else
       throw Exception("Invalid format, expeced JSON_UUID");
}
template<> Binary_t fromJson<Binary_t> (Json_t ele){
   if (ele->getType() == JSON_BINARY)
     return std::dynamic_pointer_cast<JsonBinary>(ele)->asBinary;
   else if (ele->getType() == JSON_STRING)
       return std::make_shared<Binary>(std::dynamic_pointer_cast<JsonString>(ele)->asString);
   else
       throw Exception("Invalid format, expeced JSON_UUID");
}


} } // namespace elladan::json

std::string toString (elladan::json::Json_t val)
{
    if (!val) return "none";
    std::stringstream str;
    val->write(&str, elladan::json::EncodingFlags(), elladan::json::StreamFormat::JSON);
    return str.str();
}

std::string toString (elladan::json::JsonType type){
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
