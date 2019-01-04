/*
 * BJsonSerializer.cpp
 *
 *  Created on: Apr 17, 2017
 *      Author: daniel
 */

#include "JsonWriter.h"

#include <elladan/Exception.h>
#include <elladan/Stringify.h>
#include <elladan/UUID.h>
#include <elladan/Binary.h>
#include <stddef.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>
#include <iomanip>

#include "../utf.h"

using std::to_string;

namespace elladan {
namespace json {
namespace jsonSerializer {

Serializer::Serializer(std::ostream& out, EncodingOption& flag) 
: _out(out)
, _flag(flag)
{ }

Serializer& Serializer::operator <<(const char* str) {
   *this << (std::string) str;
   return *this;
}
Serializer& Serializer::operator <<(std::string& str) {
   _out.write(str.c_str(), str.size());
   return *this;
}
Serializer& Serializer::operator <<(const std::string& str) {
   _out.write(str.c_str(), str.size());
   return *this;
}
Serializer& Serializer::write(void* data, size_t size) {
   _out.write((char*) data, size);
   return *this;
}

std::string Serializer::stringToJson(const std::string& txt) {
   std::ostringstream out;

   const char *ite, *lim;
   int32_t codepoint;

   out << "\"";

   ite = txt.c_str();
   lim = ite + txt.length();

   // Look for special characters.
   while (ite < lim) {
      ite = Utf8::iterate(ite, lim - ite, &codepoint);
      if (!ite)
         break;

      // Handle \, /, ", and control codes
      if (
            // mandatory escape or control char
            codepoint == '\\' || codepoint == '"' || codepoint < 0x20 ||
            // escape slash
            (_flag.test(EncodingFlags::ESCAPE_SLASH) && codepoint == '/'))
      {
         switch (codepoint) {
            case '\\':  out << "\\\\";   break;
            case '\"':  out << "\\\"";   break;
            case '\b':  out << "\\b";    break;
            case '\f':  out << "\\f";    break;
            case '\n':  out << "\\n";    break;
            case '\r':  out << "\\r";    break;
            case '\t':  out << "\\t";    break;
            case '/':   out << "\\/";    break;
            default: {
               // codepoint is in BMP
               if (codepoint < 0x10000) {
                  char seq[7];
                  snprintf(seq, sizeof(seq), "\\u%04X", (unsigned int) codepoint);
                  out << seq;
               }

               // not in BMP -> construct a UTF-16 surrogate pair
               else {
                  char seq[13];

                  int32_t first, last;

                  codepoint -= 0x10000;
                  first = 0xD800 | ((codepoint & 0xffc00) >> 10);
                  last = 0xDC00 | (codepoint & 0x003ff);

                  snprintf(seq, sizeof(seq), "\\u%04X\\u%04X", (unsigned int) first, (unsigned int) last);
                  out << seq;
               }
            }
            break;
         }
      }
      // non-ASCII
      else if (_flag.test(EncodingFlags::ENSURE_ASCII) && codepoint > 0x7F)
         throw Exception("Could not encode non UTF code " + to_string(codepoint));
      else
         out.put(codepoint);
   }

   out << "\"";
   return out.str();
}

void Serializer::writeSpace() {
   int indent = _flag.getIndent();
   if (indent == 0)
      // No ident, pack;
      return;

   static const std::string tabSpace = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
   static const std::string whitespace = "                                ";

   const std::string* spaceStr = &whitespace;

   if (indent == EncodingOption::MAX_INDENT_AS_TAB) {
      spaceStr = &tabSpace;  // tab it;
      indent = 1;
   }

   // add the required amount of space.
   *this << "\n";
   indent *= _depth;
   while (indent > 0) {
      int cur_n = std::min((int)spaceStr->size(), indent);
      *this << spaceStr->substr(0, cur_n);
      indent -= cur_n;
   }
}

static void ObjectMap(Serializer& s, const Json& ele) {
   bool first = true;
   s << (std::string)"{";
   s._depth++;

   elladan::VMap<const std::string*, const Json*> sorted;
   for (auto& ite : ele.as<Object>())
      sorted.emplace(&ite.first, &ite.second);

   if (s._flag.test(SORT_KEY))
      sorted.sort([](const std::pair<const std::string*, const Json*>& lhs, const std::pair<const std::string*, const Json*>& rhs) -> bool { return *lhs.first < *rhs.first; } );

   for (auto ite : sorted) {
      if (!first)
         s << (std::string)",";
      first = false;
      s.writeSpace();
      s << s.stringToJson(*ite.first);
      s << (std::string)(s._flag.getIndent() == 0 ? ":" : " : ");
      s << *ite.second;
   }
   s._depth--;
   s.writeSpace();
   s << (std::string)"}";
}

static void ArrayMap(Serializer& s, const Json& ele) {
   bool first = true;
   s << (std::string)"[";
   s._depth++;

   for (auto ite : ele.as<Array>()) {
      if (!first)
         s << (std::string)",";
      first = false;
      s.writeSpace();
      s << ite;
   }
   s._depth--;
   s.writeSpace();
   s << (std::string)"]";
}
static void UUIDMap(Serializer& s, const Json& ele) {
   if (s._flag.test(EncodingFlags::EXTENDED_TYPE))
      s << "u";
   s << s.stringToJson(ele.as<elladan::UUID>().toString());
}
static void
BinaryMap(Serializer& s, const Json& ele)
{
   if (s._flag.test(EncodingFlags::EXTENDED_TYPE))
      s << "b";
   s << s.stringToJson(ele.as<elladan::Binary>().toHex());
}

elladan::VMap<JsonType, std::function<void(Serializer& s, const Json& ele)>> Serializer::writeMap = {
   {elladan::json::Json::typeOf<Object>(), &ObjectMap },
   {elladan::json::Json::typeOf<Array >(), &ArrayMap },
   {elladan::json::Json::typeOf<std::string>(), [](Serializer& s, const Json& ele) -> void { s << s.stringToJson(ele.as<std::string>()); } },
   {elladan::json::Json::typeOf<int64_t >(), [](Serializer& s, const Json& ele) -> void { s._out << ele.as<int64_t>(); } },
   {elladan::json::Json::typeOf<double >(), [](Serializer& s, const Json& ele) -> void { s._out << ele.as<double>(); } },
   {elladan::json::Json::typeOf<bool>(), [](Serializer& s, const Json& ele) -> void { s._out << ele.as<bool>(); } },
   {elladan::json::Json::typeOf<Null>(), [](Serializer& s, const Json& ele) -> void { s << "null"; } },
   // Supplemental type.
   {elladan::json::Json::typeOf<elladan::UUID>(), &UUIDMap },
   {elladan::json::Json::typeOf<elladan::Binary>(), &BinaryMap },
};

Serializer& Serializer::operator<<(const Json& data) {
   auto ite = writeMap.find(data.getType());
   if (ite == writeMap.end())
      throw Exception(std::string("No json serializer for element ") + data.getTypeInfo().name());
   ite->second(*this, data);
   return *this;
}

void write(const Json& data, std::ostream& out, EncodingOption flag) {
   Serializer str(out, flag);
   // Set double format once. 
   str._out << std::setprecision(flag.getRealPrec()) << std::showpoint;
   // Print true false correctly.
   str._out << std::boolalpha;
   str << data;
}

}}} // elladan::json::jsonSerializer