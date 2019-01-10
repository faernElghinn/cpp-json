// /*
//  * BsonWriter.cpp
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
constexpr char BINARY_SUBTYPE_GENERIC = 0x00;
constexpr char BINARY_SUBTYPE_BINARY_OLD = 0x02;
constexpr char BINARY_SUBTYPE_UUID_OLD = 0x03;
constexpr char BINARY_SUBTYPE_UUID = 0x04;

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
   s << (int32_t) uuid.getSize() << BINARY_SUBTYPE_UUID;
   s.write((void*)uuid.getRaw(), uuid.getSize());
}
void serBinary(Serializer& s, const Json& ele) {
   const Binary& bin = ele.as<Binary>();
   s << (int32_t) bin.size() << BINARY_SUBTYPE_GENERIC;
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
   return *this;
}

void write(const Json& data, std::ostream& out){
   Serializer ser(out);
   ser << data;
}

} } } // namespace elladan::json
