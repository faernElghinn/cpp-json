/*
 * BsonSerializer.h
 *
 *  Created on: Apr 28, 2017
 *      Author: daniel
 */

#pragma once

#include <iostream>
#include <string>
#include <type_traits>
#include <functional>
#include <elladan/VMap.h>

#include "../json.h"

namespace elladan { 
namespace json { 
namespace bsonSerializer {

struct Serializer {
   Serializer(std::ostream& out);

   Serializer& operator <<(char);
   Serializer& operator <<(const char*);
   Serializer& operator <<(std::string& str);
   Serializer& operator <<(const std::string& str);
   Serializer& operator <<(int32_t);
   Serializer& operator <<(JsonType);
   Serializer& write(void* data, size_t size);

   Serializer& operator <<(const Json& val);

   std::ostream& oStr;

   typedef std::function<void(Serializer& s, const Json& ele)> serFunc;
   typedef std::pair<char, serFunc> writer;
   static elladan::VMap<JsonType, writer> writeMap;
};

void write(const Json& data, std::ostream& out);

} } } // namespace elladan::json:jsonSerializer

