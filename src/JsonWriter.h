/*
 * JsonSerializer.h
 *
 *  Created on: Apr 28, 2017
 *      Author: daniel
 */

#pragma once

#include <iostream>
#include <string>
#include <type_traits>
#include <functional>

#include <elladan/FlagSet.h>
#include <elladan/VMap.h>

#include "json.h"

namespace elladan { 
namespace json { 
namespace jsonSerializer {

enum EncodingFlags {
   ENSURE_ASCII   = 1 << 0, /// Throw error if any string are not utf compliant.
   ESCAPE_SLASH   = 1 << 1, /// Escape special character like newline and tabs.
   SORT_KEY       = 1 << 2, /// Sort map's keys before writing them.
   EXTENDED_TYPE  = 1 << 3, /// Extend json with better type info. Result won't be compatible with other json parser
};
struct EncodingOption : public FlagSet
{
   EncodingOption() = default;
   EncodingOption(EncodingFlags flag) : EncodingOption() { set((int)flag); }

   static constexpr int MAX_INDENT_AS_TAB = 15;
   inline void setIndent(int size) {
      _ident = std::clamp(size, 0, MAX_INDENT_AS_TAB);
   }
   inline int getIndent() const { return _ident; }

   static constexpr int MAX_FLOAT = 31;
   inline void setRealPrec(int size) {
      _realPrec = std::clamp(size, 1, MAX_FLOAT);
   }
   inline int getRealPrec() const { return _realPrec; }
   uint8_t _realPrec = 10;
   uint8_t _ident = 0;
};
struct Serializer {
   Serializer(std::ostream& out, EncodingOption& flag);

   Serializer& operator <<(const char*);
   Serializer& operator <<(std::string& str);
   Serializer& operator <<(const std::string& str);
   Serializer& write(void* data, size_t size);

   Serializer& operator <<(const Json& val);

   std::ostream& _out;
   int _depth = 0;
   EncodingOption _flag;

   void writeSpace();
   std::string stringToJson(const std::string& txt);
   static elladan::VMap<JsonType, std::function<void(Serializer& s, const Json& ele)>> writeMap;
};

void write(const Json& data, std::ostream& out, EncodingOption flag = EncodingOption());

} } } // namespace elladan::json:jsonSerializer

