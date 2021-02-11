/*
 * JsonReader.cpp
 *
 *  Created on: Apr 17, 2017
 *      Author: daniel
 */

#include "JsonReader.h"

#include <elladan/Exception.h>
#include <elladan/Stringify.h>
#include <elladan/UUID.h>
#include <elladan/Binary.h>
#include <elladan/utf.h>
#include <stddef.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>
#include <locale>
#include <locale>


using std::to_string;
using elladan::json::Json;

namespace elladan {
namespace json {
namespace jsonSerializer {

void Position::throwErr(const std::string& err) const{
   std::ostringstream str;
   str << err << " at line " << line+1 << " column " << col+1;
   throw Exception(str.str());
}

void Position::update(char c){
   if (c == '\n') {
      ++line;
      col = 0;
   }
   else{
      ++col;
   }
}

void JsonReader::throwErr(const std::string& err) const {
   return pos.throwErr(err);
}

JsonReader::ErrTracker::ErrTracker(const std::string& err, JsonReader& r)
: error(err)
, reader(r)
{ }

char JsonReader::ErrTracker::peekNext(bool skipWs) {
   for (int i = 0; i < reader.buffer.size(); i++) {
      if (!isspace(reader.buffer[i]) || !skipWs)
         return reader.buffer[i];
   }
   do{
      int c = reader.iStr.get();
      if (c != EOF)
         reader.buffer.push_back(c);
      if (!isspace(c) || !skipWs)
        return c;
   } while (reader.iStr);
   reader.throwErr(error);
   return '0';
}

char JsonReader::ErrTracker::getNext(bool skipWs) {
   while (!reader.buffer.empty()) {
      auto c = reader.buffer.front();
      reader.buffer.erase(0,1);
      reader.pos.update(c);
      if (!isspace(c) || !skipWs)
         return c;
   }
   do{
      int c = reader.iStr.get();
      reader.pos.update(c);
      if (!isspace(c) || !skipWs)
        return c;
   } while (reader.iStr);
   reader.throwErr(error);
   return EOF;
}
std::string JsonReader::ErrTracker::peekWord() {
   std::string ss;
   for (int i = 0; i < reader.buffer.size(); i++) {
      char c = reader.buffer[i];
      if (!isalnum(c) && c != '.' && c != '-' && c != '+') {
         if (!ss.empty())
            return ss;
      }
      else
         ss.push_back(c);
   }
   do {
      int c = reader.iStr.get();
      if (c != EOF)
         reader.buffer.push_back(c);
      if (!isalnum(c) && c != '.' && c != '-' && c != '+') {
         if (!ss.empty())
            return ss;
      }
      if (c != EOF)
         ss.push_back(c);
   } while (reader.iStr);

   if (ss.empty())
      reader.throwErr(error);
   return ss;
}
std::string JsonReader::ErrTracker::getWord() {
   std::string ss;
   while (!reader.buffer.empty()) {
      auto c = reader.buffer.front();
      if (!isalnum(c) && c != '.' && c != '-' && c != '+') {
         if (!ss.empty())
            return ss;
      }
      else
         ss.push_back(c);
      reader.pos.update(c);
      reader.buffer.erase(0,1);
   }
   do{
      int c = reader.iStr.get();
      if (!isalnum(c) && c != '.' && c != '-' && c != '+') {
         if (!ss.empty()) {
            reader.buffer.push_back(c);
            return ss;
         }
      }
      reader.pos.update(c);
      if (c != EOF)
         ss.push_back(c);
   } while (reader.iStr);

   if (ss.empty())
      reader.throwErr(error);
   return ss;
}
std::string JsonReader::ErrTracker::get(int count) {
   std::string ss = reader.buffer.substr(0, count);
   ss.reserve(count);

   for (auto ite : ss)
      reader.pos.update(ite);

   while (ss.size() < count) {
      char c = reader.iStr.get();
      reader.pos.update(c);
      if (!reader.iStr) 
         reader.throwErr(error);
      ss.push_back(c);
   }

   return ss;
}

JsonReader::JsonReader(std::istream& reader, DecodingOption flag) :
   iStr(reader), flags(flag) {
   pos.line = 0;
   pos.col = -1;
   iStr >> std::noskipws;
}

JsonReader::ErrTracker JsonReader::operator()(std::string errMessage) {
   if (!errMessage.empty())
      errMessage = std::string("Unexpected EOF while ") + errMessage;
   else
      errMessage = "Unexpected EOF";
   return ErrTracker(errMessage, *this);
}

static std::string jsonToString(JsonReader& reader) {
   std::string retVal;
   char letter;

   do {
      letter = reader("Could not decode string : Unexpected end of file.").getNext(false);
      switch (letter) {
         case '\\': {
            letter = reader("decoding escaped char").getNext(false);

            switch (letter) {
               case '"':    retVal += "\"";  break;
               case '/':    retVal += "/";   break;
               case '\\':   retVal += "\\";  break;
               case 'b':    retVal += "\b";  break;
               case 'f':    retVal += "\f";  break;
               case 'n':    retVal += "\n";  break;
               case 'r':    retVal += "\r";  break;
               case 't':    retVal += "\t";  break;
               case '0':
                  if (!reader.flags.test(DecodingFlags::ALLOW_NULL))
                     reader.throwErr("Found null value reader string");
                  retVal += "\0";
                  break;

               case 'u': {
                  int sum = 0;
                  std::string val;
                  for (int i = 0; i < 4; i++) 
                     val.push_back(reader("decoding UTF encoded char").getNext(false));
                  sum = std::stoi(val, 0, 16);

                  if (sum >= 0xDC00)
                     reader.throwErr("Invalid unicode escape");

                  if (sum >= 0xD800) {
                     val.clear();
                     for (int i = 0; i < 4; i++) {
                        val.push_back(reader("decoding UTF encoded char").getNext(false));
                     }
                     int sum2 = std::stoi(val, 0, 16);

                     if (sum2 < 0xDC00)
                        reader.throwErr("Invalid unicode escape");

                     sum = ((sum - 0xD800) << 10) + (sum2 - 0xDC00) + 0x10000;
                  }

                  char unicode[9];
                  size_t length;
                  if (Utf8::encode(sum, unicode, &length))
                     reader.throwErr("Could not process unicode");

                  unicode[length] = '\0';
                  retVal += unicode;
               }
               break;

               default:
                  reader.throwErr("Invalid escape");
                  break;
            }
         }
         break;

         case '"':
            // End of string.
            return retVal;

         case '\0':
            if (!reader.flags.test(DecodingFlags::ALLOW_NULL))
               reader.throwErr("Found null value reader string");
            /* no break */
         default:
            retVal.push_back(letter);
            break;
      }
   } while (1);
}

static std::optional<json::Json> parseObject(JsonReader& reader) {
   if (reader().peekNext(true) != '{') 
      return std::nullopt;
   reader().getNext(true);

   json::Object obj;
   char cur = reader("looking for the end of the object").peekNext(true);

   bool firstElement = true;
   while (cur != '}') {
      if (cur != ',' && !firstElement && !reader.flags.test(DecodingFlags::ALLOW_COMMA_ERR))
         reader.throwErr("Expected an element delimiter \',\'");

      while (cur == ',') {
         reader().getNext(true);
         cur = reader("looking for next element").peekNext(true);
         if (!reader.flags.test(DecodingFlags::ALLOW_COMMA_ERR)) 
            break;
      }
      if (cur == '}')
         break;

      firstElement = false;

      // Get the key.
      Position startOfLine = reader.pos;
      auto key = reader.extractNext();
      if (!key.has_value() || !key->isOfType<std::string>())
         startOfLine.throwErr("Missing key");
      if (reader.flags.test(DecodingFlags::REJECT_DUPLICATE) && obj.count(key->as<std::string>()))
         startOfLine.throwErr("Duplicate value");

      // Get ":"
      cur = reader("looking for key value delimiter \':\'").peekNext(true);
      if (cur == ':') reader().getNext();
      else if (!reader.flags.test(DecodingFlags::ALLOW_COMMA_ERR))
         reader.throwErr("Expecting array key value delimiter \":\"");

      // Read the associated value.
      startOfLine = reader.pos;
      auto child = reader.extractNext();
      if (!child.has_value())
         startOfLine.throwErr("File ended before getting the value");
      obj.setInplace(key->as<std::string>(), std::move(child.value()));

      cur = reader("looking for the end of the object").peekNext(true);
   }
   reader().getNext();

   return obj;
}

static std::optional<json::Json> parseArray(JsonReader& reader) {
   if (reader().peekNext(true) != '[') 
      return std::nullopt;
   reader().getNext(true);

   Array arr;
   char cur = reader("looking for the end of the array").peekNext(true);
   bool firstElement = true;
   while (cur != ']') {

      if (cur != ',' && !firstElement && !reader.flags.test(DecodingFlags::ALLOW_COMMA_ERR))
         reader.throwErr("Expected an element delimiter \',\'");

      while (cur == ',') {
         reader().getNext(true);
         cur = reader("looking for next element").peekNext(true);
         if (!reader.flags.test(DecodingFlags::ALLOW_COMMA_ERR)) 
            break;
      }
      if (cur == ']')
         break;

      firstElement = false;

      // Read the associated value.
      Position pos = reader.pos;
      auto child = reader.extractNext();
      if (!child.has_value())
         pos.throwErr("File ended before getting the value");
      arr.push_back(child);

      cur = reader("looking for next element").peekNext(true);
   }
   reader().getNext();

   return arr;
}

static std::optional<json::Json> parseString(JsonReader& reader) {
   if (reader().peekNext(true) != '"') 
      return std::nullopt;

   reader().getNext(true);
   return jsonToString(reader);
}

static std::optional<json::Json> parseNumber(JsonReader& reader) {
   std::string str = reader().peekWord();

   long int asInt;
   if (elladan::parseString(str, asInt) == str.size()){
      reader().getWord();
      return asInt;
   }

   double asDouble;
   if (elladan::parseString(str, asDouble) == str.size()) {
      reader().getWord();
      return asDouble;
   }

   return std::nullopt;
}

static std::optional<json::Json> parseBool(JsonReader& reader) {
   std::string str = elladan::toLower(reader().peekWord());
   if (str == "true") {
      reader().getWord();
      return json::Json(true);
   }
   if (str == "false") {
      reader().getWord();
      return json::Json(false);
   }

   return std::nullopt;
}

static std::optional<json::Json> parseNull(JsonReader& reader) {
   static const std::string expected = "null";

   auto v = reader().peekWord();
   std::string str = elladan::toLower(v);
   if (str == "null") {
      if (!reader.flags.test(DecodingFlags::ALLOW_NULL))
         reader.throwErr("Forbidden null found");

      reader().getWord();
      return json::Json(Null());
   }

   return std::nullopt;
}

static std::optional<json::Json> parseUUID(JsonReader& reader) {

   if (tolower(reader().peekNext(true)) != 'u')
      return std::nullopt;

   reader().getNext(true);

   if (reader("Fetching start of UUID").getNext(false) != '"') 
      reader.throwErr("Invalid UUID format");

   UUID uuid;
   std::string str = reader("Reading UUID").get(uuid.getStringSize() + 1);
   if (str.size() != uuid.getStringSize() + 1 || str.back() != '"')
      reader.throwErr("Invalid UUID");

   str.pop_back();

   return UUID::fromString(str);
}

static std::optional<json::Json> parseBinary(JsonReader& reader) {
   if (tolower(reader().peekNext(true)) != 'b')
      return std::nullopt;
   
   reader().getNext();
   
   if (reader("Fetching start of Binary").getNext(false) != '"') 
      reader.throwErr("Invalid Binary format");

   std::stringstream str;
   auto src = reader("Reading binary");
   do {
      char cur = src.getNext(false);
      if (cur == '"')
         break;
      str << cur;
   } while(true);

   json::Json bin = std::move(Binary(str.str())); // FIXME: should include length!
   return bin;
}

std::vector<JsonReader::parse> JsonReader::readType = {
   &parseObject, &parseArray, &parseString, &parseNumber, &parseBool, &parseNull,
   &parseUUID, &parseBinary
};

std::optional<json::Json> JsonReader::extractNext() {
   for (auto ite : readType) {
      auto res = ite(*this);
      if (res.has_value())
         return res;
   }
   return std::nullopt;
}

json::Json read(std::istream& in_stream, DecodingOption flag) {
   JsonReader reader(in_stream, flag);
   auto res = reader.extractNext();
   return res.value_or(Json());
}

// FIXME: extract!
//std::vector<json::Json> extract(std::istream& in_stream, const std::string& path, DecodingOption flag) {
//   json::Json obj = read(in_stream, flag);
//   std::vector<json::Json> retVal;
//
//   for (auto ite : obj.get(path))
//      retVal.push_back(*ite);
//
//   return retVal;
//}

} } } // namespace elladan::json::jsonSerializer
