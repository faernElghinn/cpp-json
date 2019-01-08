// /*
//  * BJsonSerializer.cpp
//  *
//  *  Created on: Apr 17, 2017
//  *      Author: daniel
//  */

#include "BsonWriter.h"

#include <elladan/Exception.h>
#include <elladan/FlagSet.h>
#include <elladan/Stringify.h>
#include <elladan/UUID.h>
#include <elladan/Binary.h>
#include <stdio.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

using std::to_string;


namespace elladan { namespace json { namespace bsonSerializer {

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



class SizeMarker{
public:
   SizeMarker (Serializer& s) : ser(s) {
      _init_pos = ser.oStr.tellp();
      ser << (int32_t) 0;
   }
   ~SizeMarker(){
      int end_pos = ser.oStr.tellp();
      ser.oStr.seekp(_init_pos);
      int32_t size = end_pos - _init_pos;
      ser << size;
      ser.oStr.seekp(0, std::ios_base::end);
   }

protected:
   int32_t _init_pos;
   Serializer& ser;
};

Serializer::Serializer(std::ostream& out) 
: oStr(out)
{}

Serializer& Serializer::operator <<(char c) {
   oStr.write(&c, 1);
   return *this;
}
Serializer& Serializer::operator <<(const char* str) {
   oStr << str << DOC_END;
   return *this;
}
Serializer& Serializer::operator <<(std::string& str) {
   oStr << str << DOC_END;
   return *this;
}
Serializer& Serializer::operator <<(const std::string& str) {
   oStr << str << DOC_END;
   return *this;
}
Serializer& Serializer::operator <<(int32_t v) {
   oStr.write((const char*)&v, sizeof(v));
   return *this;
}
Serializer& Serializer::write(void* data, size_t size){
   oStr.write((const char*)data, size);
   return *this;
}
Serializer& Serializer::operator <<(JsonType v) {
   auto ite = writeMap.find(v);
   if (ite == writeMap.end())
      throw Exception("No serializer for json element" + to_string(v));
   oStr << ite->second.first;
   return *this;
}

void serObject(Serializer& s, const Json& ele) {
   SizeMarker marker (s);
   for (auto& ite : ele.as<Object>()) {
      // print type
      s << ite.second.getType();
      // print name
      s << ite.first;
      // print value
      s << ite.second;
   }
   s << DOC_END;
}
void serArray(Serializer& s, const Json& ele) {
   SizeMarker marker (s);
   int i = 0;
   for (auto& ite : ele.as<Array>()) {
      // print type
      s << ite.getType();
      // print name
      s << to_string(i++);
      // print value
      s << ite;
   }
   s << DOC_END;
}
void serString(Serializer& s, const Json& ele) {
   auto& str = ele.as<std::string>();
   s << (int32_t) str.size() + 1 << str;
}
void serUUID(Serializer& s, const Json& ele) {
   const UUID& uuid = ele.as<UUID>();
   s << (int32_t) uuid.getSize() << (char)0x04;
   s.write((void*)uuid.getRaw(), uuid.getSize());
}
void serBinary(Serializer& s, const Json& ele) {
   const Binary& bin = ele.as<Binary>();
   s << (int32_t) bin.size() << (char)0x00;
   s.write((void*)bin.data.data(), bin.size());
}

elladan::VMap<JsonType, Serializer::writer> Serializer::writeMap = {
   {Json::typeOf<Object>(),{ELE_TYPE_OBJECT, &serObject}},
   {Json::typeOf<Array>(), {ELE_TYPE_ARRAY, &serArray}},
   {Json::typeOf<std::string>(), {ELE_TYPE_UTF_STRING, &serString}},
   {Json::typeOf<int64_t>(), {ELE_TYPE_INT64, [](Serializer& s, const Json& ele)->void { s.write((void*)&ele.as<int64_t>(), sizeof(int64_t));}}},
   {Json::typeOf<double>(), {ELE_TYPE_DOUBLE, [](Serializer& s, const Json& ele)->void { s.write((void*)&ele.as<double>(), sizeof(double));}}},
   {Json::typeOf<bool>(), {ELE_TYPE_BOOL, [](Serializer& s, const Json& ele)->void { s << (char)(ele.as<bool>() ? 1 : 0); }}},
   {Json::typeOf<Null>(), {ELE_TYPE_NULL, [](Serializer& s, const Json& ele)->void { /* Do nothing, only the type matter */}}},
   // Supplemental type.
   {Json::typeOf<UUID>(), {ELE_TYPE_BIN, &serUUID }},
   {Json::typeOf<Binary>(), {ELE_TYPE_BIN, &serBinary }}
};

Serializer& Serializer::operator <<(const Json& val){
   auto ite = writeMap.find(val.getType());
   if (ite == writeMap.end())
      throw Exception("No serializer for json element" + to_string(val));
   ite->second.second(*this, val);
}

void write(const Json& data, std::ostream& out){
   Serializer ser(out);
   ser << data;
}

// class BIStream {
// public :
//    std::istream* _in;

//    BIStream(std::istream* in) : _in(in) {}

//    template <typename T>
//    int operator >> (T& val){
//       _in->read((char*)&val, sizeof(val));
//       return !_in->eof();
//    }
//    int read(void* data, size_t size){
//       _in->read((char*)data, size);
//       return !_in->fail();
//    }
//    int ignore(size_t size){
//       _in->ignore(size);
//       return !_in->eof();
//    }

//    void throwException(const std::string& what){
//       throw Exception(what + " at location " + to_string((int)_in->tellg()));
//    }
// };

// template <>
// int BIStream::operator >> <std::string>(std::string& str){
//    char c;
//    while (*this >> c) {
//       if (!c) return str.size();
//       str.push_back(c);
//    }
//    throw Exception("End of file while reading null terminated string");
// }

// inline void BsonSerializer::readName(BIStream& in, std::string& name){
//    in >> name;
// }

// inline void BsonSerializer::readRaw(BIStream& in, char* data, size_t size){
//    if (!in.read(data, size))
//       in.throwException("End of file before getting end of data");
// }

// Json_t BsonSerializer::readBson(BIStream& in, char type){
//    switch (type) {
//       case ELE_TYPE_OBJECT:
//       {
//          uint32_t size = 0;
//          readRaw(in, (char*)&size, sizeof(size));

//          JsonObject_t obj = std::make_shared<JsonObject>();
//          char subType;
//          in >> subType;
//          while (subType != EOF && subType != DOC_END){
//             std::string name;
//             readName(in, name);
//             obj->value[name] = readBson(in, subType);
//             in >> subType;
//          }

//          return obj;
//       }
//       case ELE_TYPE_ARRAY:
//       {
//          uint32_t size = 0;
//          readRaw(in, (char*)&size, sizeof(size));

//          JsonArray_t obj = std::make_shared<JsonArray>();

//          char subType;
//          in >> subType;
//          while (subType != EOF && subType != DOC_END){
//             std::string name;
//             readName(in, name);
//             obj->value.push_back(readBson(in, subType));
//             in >> subType;
//          }

//          return obj;
//       }
//       case ELE_TYPE_UTF_STRING:
//       {
//          uint32_t size = 0;
//          readRaw(in, (char*)&size, sizeof(size));
//          JsonString_t obj = std::make_shared<JsonString>();
//          obj->value.reserve(size+1);
//          char val[257];
//          int curChar;
//          while (size > 0){
//             curChar = std::min(size, (uint32_t)256);
//             readRaw(in, val, size);
//             val[size] = 0;
//             obj->value.append(val);
//             size -= curChar;
//          }
//          if (val[curChar-1] != 0)
//             in.throwException("String does not end with null char");
//          return obj;
//       }
//       case ELE_TYPE_DOUBLE:
//       {
//          double val = 0;
//          readRaw(in, (char*)&val, sizeof(val));
//          return std::make_shared<JsonDouble>(val);
//       }
//       case ELE_TYPE_INT64:
//       {
//          int64_t val = 0;
//          readRaw(in, (char*)&val, sizeof(val));
//          return std::make_shared<JsonInt>(val);
//       }
//       case ELE_TYPE_BOOL:
//       {
//          char val = 0;
//          readRaw(in, &val, sizeof(val));
//          return std::make_shared<JsonBool>(val);
//       }
//       case ELE_TYPE_NULL:
//          return std::make_shared<JsonNull>();

//       case ELE_TYPE_BIN:
//       {
//          uint32_t size = 0;
//          readRaw(in, (char*)&size, sizeof(size));

//          uint8_t subtype;
//          readRaw(in, (char*)&subtype, sizeof(subtype));

//          switch (subtype) {
//             case 0x02: {
//                Binary_t bin = std::make_shared<Binary>(size);
//                readRaw(in, (char*)bin->data, bin->size);
//                return std::make_shared<JsonBinary>(bin);
//             } break;

//             case 0x04: {
//                JsonUUID_t uuid = std::make_shared<JsonUUID>();
//                if (size != uuid->value.getSize())
//                   in.throwException("Expected UUID, but size is wrong");

//                readRaw(in, (char*)uuid->value.getRaw(), size);
//                return uuid;
//             } break;

//             default:
//                throw Exception("Unknown subtype " + to_string(subtype));
//                break;
//          }
//       } break;

//       default:
//          in.throwException("Unknown/Unsupported type " + std::to_string(type));
//    }
//    return Json_t();
// }


// Json_t BsonSerializer::read(std::istream* in, DecodingOption flag){
//    BIStream str(in);
//    return readBson(str, ELE_TYPE_OBJECT);
// }


// std::vector<Json_t> BsonSerializer::searchBson(BIStream& in, char type, int deepness, std::vector<std::string> parts){
//    std::vector<Json_t> retVal;
//    switch (type) {
//       case ELE_TYPE_OBJECT:
//       case ELE_TYPE_ARRAY:
//       {
//          uint32_t size = 0;
//          readRaw(in, (char*)&size, sizeof(size));

//          JsonObject_t obj = std::make_shared<JsonObject>();
//          char subType;
//          in >> subType;
//          while (subType != EOF && subType != DOC_END){
//             std::string name;
//             readName(in, name);

//             bool match;
//             bool inc = true;

//             if (parts[deepness] == "**") {
//                match = true;
//                inc = false;
//                deepness += (deepness + 1 < parts.size() && parts[deepness+1] == name);
//             }
//             else {
//                match = parts[deepness] == "*" || parts[deepness] == name;
//             }

//             if (match && deepness+1 == parts.size())
//                retVal.push_back(readBson(in, subType));

//             else if (match) {
//                auto subRet = searchBson(in, subType, deepness+inc, parts);
//                retVal.insert(retVal.end(), subRet.begin(), subRet.end());
//             }

//             else
//                skipBson(in, subType);

//             in >> subType;
//          }

//       } break;

//       default:
//          skipBson(in, type);
//          break;
//    }
//    return retVal;
// }

// void BsonSerializer::skipBson(BIStream& in, char type){
//    switch (type) {
//       case ELE_TYPE_OBJECT:
//       case ELE_TYPE_ARRAY:
//       {
//          uint32_t size = 0;
//          readRaw(in, (char*)&size, sizeof(size));
//          in.ignore(size-4);
//       } break;
//       case ELE_TYPE_UTF_STRING:
//       {
//          uint32_t size = 0;
//          readRaw(in, (char*)&size, sizeof(size));
//          in.ignore(size);
//       } break;
//       case ELE_TYPE_BIN:
//       {
//          uint32_t size = 0;
//          readRaw(in, (char*)&size, sizeof(size));
//          in.ignore(size+1);
//       } break;
//       case ELE_TYPE_DOUBLE:
//          in.ignore(sizeof(double));
//          break;
//       case ELE_TYPE_INT64:
//          in.ignore(sizeof(int64_t));
//          break;
//       case ELE_TYPE_BOOL:
//          in.ignore(sizeof(char));
//          break;
//       case ELE_TYPE_NULL:
//          break;
//       default:
//          in.throwException("Unknown/Unsupported type " + std::to_string(type));
//    }
// }

// std::vector<Json_t> BsonSerializer::extract(std::istream* in, DecodingOption flag, const std::string& path){
//    BIStream str(in);
//    return searchBson(str, ELE_TYPE_OBJECT, 1, tokenize(path, "/"));
// }





} } } // namespace elladan::json
