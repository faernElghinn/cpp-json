/*
 * BJsonSerializer.cpp
 *
 *  Created on: Apr 17, 2017
 *      Author: daniel
 */

#include "BsonSerializer.h"

#include <elladan/Exception.h>
#include <elladan/FlagSet.h>
#include <elladan/Stringify.h>
#include <elladan/UUID.h>
#include <stdio.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

using std::to_string;


namespace elladan { namespace json {


constexpr char DOC_END = 0x00;
constexpr char ELE_TYPE_DOUBLE = 0x01;
constexpr char ELE_TYPE_UTF_STRING = 0x02;
constexpr char ELE_TYPE_OBJECT = 0x03;
constexpr char ELE_TYPE_ARRAY = 0x04;
constexpr char ELE_TYPE_BIN = 0x05;
//constexpr char ELE_TYPE_DEPRECATED = 0x06;
//constexpr char ELE_TYPE_OBJECT_ID = 0x07;
constexpr char ELE_TYPE_BOOL = 0x08;
//constexpr char ELE_TYPE_UTC_DATETIME= 0x09;
constexpr char ELE_TYPE_NULL = 0x0A;
//constexpr char ELE_TYPE_REGEX = 0x0B;
//constexpr char ELE_TYPE_DEPRECATED = 0x0C;
//constexpr char ELE_TYPE_JAVASCRIPT = 0x0D;
//constexpr char ELE_TYPE_DEPRECATED = 0x0E;
//constexpr char ELE_TYPE_JAVASCRIPT_SCOPED = 0x0F;
//constexpr char ELE_TYPE_INT32 = 0x10;
//constexpr char ELE_TYPE_UINT64 = 0x11;
constexpr char ELE_TYPE_INT64 = 0x12;
//constexpr char ELE_TYPE_DECIMAL128 = 0x13;
//constexpr char ELE_TYPE_MIN = 0xFF;
//constexpr char ELE_TYPE_MAX = 0x7F;

class BOStream {
public :
   std::ostream* _out;
   std::string _str;

   BOStream(std::ostream* out)  : _out(out) {
      _str.reserve(1024);
   }
   ~BOStream() {
      _out->write(_str.c_str(), _str.size());
   }

   BOStream& operator << (char c){
      _str.append(&c, sizeof(c));
      return *this;
   }
   BOStream& operator << (int32_t c){
      _str.append((const char*)&c, sizeof(c));
      return *this;
   }
   BOStream& operator << (int64_t& c){
      _str.append((const char*)&c, sizeof(c));
      return *this;
   }
   BOStream& operator << (uint32_t c){
      _str.append((const char*)&c, sizeof(c));
      return *this;
   }
   BOStream& operator << (uint64_t& c){
      _str.append((const char*)&c, sizeof(c));
      return *this;
   }
   BOStream& operator << (float c){
      _str.append((const char*)&c, sizeof(c));
      return *this;
   }
   BOStream& operator << (double& c){
      _str.append((const char*)&c, sizeof(c));
      return *this;
   }
   BOStream& write(const void* data, size_t size){
      _str.append((const char*) data, size);
      return *this;
   }
   BOStream& operator<< (const std::string& str){
      _str += str;
      _str.push_back(DOC_END);
      return *this;
   }
   size_t pos() const {
      return _str.size();
   }
};

char BsonSerializer::getBsonType(const Json* ele) //throw Error
{
   if (!ele) throw Exception("NONE json not supported ");
   switch (ele->getType()) {
      case JsonType::JSON_NULL:       return ELE_TYPE_NULL;
      case JsonType::JSON_BOOL:       return ELE_TYPE_BOOL;
      case JsonType::JSON_INTEGER:    return ELE_TYPE_INT64;
      case JsonType::JSON_DOUBLE:     return ELE_TYPE_DOUBLE;
      case JsonType::JSON_STRING:     return ELE_TYPE_UTF_STRING;
      case JsonType::JSON_ARRAY:      return ELE_TYPE_ARRAY;
      case JsonType::JSON_OBJECT:     return ELE_TYPE_OBJECT;
      case JsonType::JSON_BINARY:     return ELE_TYPE_BIN;
      case JsonType::JSON_UUID:       return ELE_TYPE_BIN;

      default:
         throw Exception("Unsupported Json_t type : " + ele->getType());
   }
}

class SizeMarker{
public:
   SizeMarker (BOStream& out) : _out(out) {
      _init_pos = out.pos();
      _out << (uint32_t) 0;
   }
   ~SizeMarker(){
      int end_pos = _out.pos();
      uint32_t size = end_pos - _init_pos;
      _out._str.replace(_init_pos, sizeof(size), (const char*)&size, sizeof(size));
      return;
   }

protected:
   size_t _init_pos;
   BOStream& _out;
};

inline void BsonSerializer::writeDoc(BOStream& out, const elladan::VMap<std::string, Json_t>& eleList, EncodingOption flag){
   SizeMarker marker (out);
   for (auto& ite : eleList) {
      // print type
      out << getBsonType(ite.second.get());
      // print name
      out << ite.first;
      // print value
      writeBson(out, ite.second.get(), flag);
   }
   out << DOC_END;
}

void BsonSerializer::writeBson(BOStream& out, const Json* ele, EncodingOption flag) {

   switch (ele->getType()) {
      case JsonType::JSON_NULL:
         // Add nothing.
         break;

      case JsonType::JSON_BOOL:
         out << (char) ((JsonBool*)ele)->value;
         break;

      case JsonType::JSON_INTEGER:
         out << ((JsonInt*)ele)->value;
         break;

      case JsonType::JSON_DOUBLE:
         out << ((JsonDouble*)ele)->value;
         break;

      case JsonType::JSON_STRING:
         out << (uint32_t) ((JsonString*)ele)->value.size() + 1; // for the DOC_END char.
         out << ((JsonString*)ele)->value;
         break;

      case JsonType::JSON_ARRAY:
      {
         elladan::VMap<std::string, Json_t> eleList;
         int i = 0;
         for (Json_t ite : static_cast<const JsonArray*>(ele)->value)
            eleList[to_string(i++)] = ite;

         writeDoc(out, eleList, flag);
      } break;

      case JsonType::JSON_OBJECT:
         if (flag.test(EncodingFlags::EF_JSON_SORT_KEY)) {
            elladan::VMap<std::string, Json_t> eleList;
            int i = 0;
            for (Json_t ite : static_cast<const JsonArray*>(ele)->value)
               eleList[to_string(i++)] = ite;
            std::stable_sort(eleList.begin(), eleList.end());
            writeDoc(out, eleList, flag);
         }
         else
            writeDoc(out, static_cast<const JsonObject*>(ele)->value, flag);
         break;

      case JsonType::JSON_BINARY:
      {
         JsonBinary* bin = ((JsonBinary*)ele);
         if (!bin->value) {
            out << (uint32_t)0;
            out << (char) 0x02;
         }
         else {
            out << (uint32_t)bin->value->size;
            out << (char) 0x02;
            out.write(bin->value->data, bin->value->size);
         }
      } break;

      case JsonType::JSON_UUID:
      {
         JsonUUID* uuid = ((JsonUUID*)ele);
         out << (uint32_t)uuid->value.getSize();
         out << (char) 0x04;
         out.write(uuid->value.getRaw(), uuid->value.getSize());
      } break;

      default:
         throw Exception("Unsupported Json_t type : " + to_string(ele->getType()));
   }
}

void BsonSerializer::write(std::ostream* out, const Json* data, EncodingOption flag){
   BOStream str(out);
   switch (data->getType()) {
      case JSON_ARRAY:
      case JSON_OBJECT:
         writeBson(str, data, flag);
         break;

      default:
         throw Exception("Bson require that root object is either an object or an array. Is an " + to_string(data->getType()));
         break;
   }
}


///////////////////////////////////

class BIStream {
public :
   std::istream* _in;

   BIStream(std::istream* in) : _in(in) {}

   template <typename T>
   int operator >> (T& val){
      _in->read((char*)&val, sizeof(val));
      return !_in->eof();
   }
   int read(void* data, size_t size){
      _in->read((char*)data, size);
      return !_in->fail();
   }
   int ignore(size_t size){
      _in->ignore(size);
      return !_in->eof();
   }

   void throwException(const std::string& what){
      throw Exception(what + " at location " + to_string((int)_in->tellg()));
   }
};

template <>
int BIStream::operator >> <std::string>(std::string& str){
   char c;
   while (*this >> c) {
      if (!c) return str.size();
      str.push_back(c);
   }
   throw Exception("End of file while reading null terminated string");
}

inline void BsonSerializer::readName(BIStream& in, std::string& name){
   in >> name;
}

inline void BsonSerializer::readRaw(BIStream& in, char* data, size_t size){
   if (!in.read(data, size))
      in.throwException("End of file before getting end of data");
}

Json_t BsonSerializer::readBson(BIStream& in, char type){
   switch (type) {
      case ELE_TYPE_OBJECT:
      {
         uint32_t size = 0;
         readRaw(in, (char*)&size, sizeof(size));

         JsonObject_t obj = std::make_shared<JsonObject>();
         char subType;
         in >> subType;
         while (subType != EOF && subType != DOC_END){
            std::string name;
            readName(in, name);
            obj->value[name] = readBson(in, subType);
            in >> subType;
         }

         return obj;
      }
      case ELE_TYPE_ARRAY:
      {
         uint32_t size = 0;
         readRaw(in, (char*)&size, sizeof(size));

         JsonArray_t obj = std::make_shared<JsonArray>();

         char subType;
         in >> subType;
         while (subType != EOF && subType != DOC_END){
            std::string name;
            readName(in, name);
            obj->value.push_back(readBson(in, subType));
            in >> subType;
         }

         return obj;
      }
      case ELE_TYPE_UTF_STRING:
      {
         uint32_t size = 0;
         readRaw(in, (char*)&size, sizeof(size));
         JsonString_t obj = std::make_shared<JsonString>();
         obj->value.reserve(size+1);
         char val[257];
         int curChar;
         while (size > 0){
            curChar = std::min(size, (uint32_t)256);
            readRaw(in, val, size);
            val[size] = 0;
            obj->value.append(val);
            size -= curChar;
         }
         if (val[curChar-1] != 0)
            in.throwException("String does not end with null char");
         return obj;
      }
      case ELE_TYPE_DOUBLE:
      {
         double val = 0;
         readRaw(in, (char*)&val, sizeof(val));
         return std::make_shared<JsonDouble>(val);
      }
      case ELE_TYPE_INT64:
      {
         int64_t val = 0;
         readRaw(in, (char*)&val, sizeof(val));
         return std::make_shared<JsonInt>(val);
      }
      case ELE_TYPE_BOOL:
      {
         char val = 0;
         readRaw(in, &val, sizeof(val));
         return std::make_shared<JsonBool>(val);
      }
      case ELE_TYPE_NULL:
         return std::make_shared<JsonNull>();

      case ELE_TYPE_BIN:
      {
         uint32_t size = 0;
         readRaw(in, (char*)&size, sizeof(size));

         uint8_t subtype;
         readRaw(in, (char*)&subtype, sizeof(subtype));

         switch (subtype) {
            case 0x02: {
               Binary_t bin = std::make_shared<Binary>(size);
               readRaw(in, (char*)bin->data, bin->size);
               return std::make_shared<JsonBinary>(bin);
            } break;

            case 0x04: {
               JsonUUID_t uuid = std::make_shared<JsonUUID>();
               if (size != uuid->value.getSize())
                  in.throwException("Expected UUID, but size is wrong");

               readRaw(in, (char*)uuid->value.getRaw(), size);
               return uuid;
            } break;

            default:
               throw Exception("Unknown subtype " + to_string(subtype));
               break;
         }
      } break;

      default:
         in.throwException("Unknown/Unsupported type " + std::to_string(type));
   }
   return Json_t();
}


Json_t BsonSerializer::read(std::istream* in, DecodingOption flag){
   BIStream str(in);
   return readBson(str, ELE_TYPE_OBJECT);
}


std::vector<Json_t> BsonSerializer::searchBson(BIStream& in, char type, int deepness, std::vector<std::string> parts){
   std::vector<Json_t> retVal;
   switch (type) {
      case ELE_TYPE_OBJECT:
      case ELE_TYPE_ARRAY:
      {
         uint32_t size = 0;
         readRaw(in, (char*)&size, sizeof(size));

         JsonObject_t obj = std::make_shared<JsonObject>();
         char subType;
         in >> subType;
         while (subType != EOF && subType != DOC_END){
            std::string name;
            readName(in, name);

            bool match;
            bool inc = true;

            if (parts[deepness] == "**") {
               match = true;
               inc = false;
               deepness += (deepness + 1 < parts.size() && parts[deepness+1] == name);
            }
            else {
               match = parts[deepness] == "*" || parts[deepness] == name;
            }

            if (match && deepness+1 == parts.size())
               retVal.push_back(readBson(in, subType));

            else if (match) {
               auto subRet = searchBson(in, subType, deepness+inc, parts);
               retVal.insert(retVal.end(), subRet.begin(), subRet.end());
            }

            else
               skipBson(in, subType);

            in >> subType;
         }

      } break;

      default:
         skipBson(in, type);
         break;
   }
   return retVal;
}

void BsonSerializer::skipBson(BIStream& in, char type){
   switch (type) {
      case ELE_TYPE_OBJECT:
      case ELE_TYPE_ARRAY:
      {
         uint32_t size = 0;
         readRaw(in, (char*)&size, sizeof(size));
         in.ignore(size-4);
      } break;
      case ELE_TYPE_UTF_STRING:
      {
         uint32_t size = 0;
         readRaw(in, (char*)&size, sizeof(size));
         in.ignore(size);
      } break;
      case ELE_TYPE_BIN:
      {
         uint32_t size = 0;
         readRaw(in, (char*)&size, sizeof(size));
         in.ignore(size+1);
      } break;
      case ELE_TYPE_DOUBLE:
         in.ignore(sizeof(double));
         break;
      case ELE_TYPE_INT64:
         in.ignore(sizeof(int64_t));
         break;
      case ELE_TYPE_BOOL:
         in.ignore(sizeof(char));
         break;
      case ELE_TYPE_NULL:
         break;
      default:
         in.throwException("Unknown/Unsupported type " + std::to_string(type));
   }
}

std::vector<Json_t> BsonSerializer::extract(std::istream* in, DecodingOption flag, const std::string& path){
   BIStream str(in);
   return searchBson(str, ELE_TYPE_OBJECT, 1, tokenize(path, "/"));
}





} } // namespace elladan::json
