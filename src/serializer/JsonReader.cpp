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

#include "../utf.h"

using std::to_string;

namespace elladan {
namespace json {
namespace jsonSerializer {

void Position::throwErr(const std::string& err) const{
   std::ostringstream str;
   str << err << " at line " << line << " column " << col;
   throw Exception(str.str());
}

void JsonReader::throwErr(const std::string& err) const {
   return pos.throwErr(err);
}

JsonReader::ErrTracker::ErrTracker(const std::string& err, JsonReader& r)
: error(err)
, reader(r)
{ }

char JsonReader::ErrTracker::peekNext(bool skipWs) {
   std::istream* istr = &reader.iStr;
   if (!reader.buffer.eof()) 
      istr = &reader.buffer;
   std::istream& s = *istr;

   s >> ( skipWs ? std::skipws : std::noskipws );
   return s.peek();
}
char JsonReader::ErrTracker::getNext(bool skipWs) {
   std::istream* istr = &reader.iStr;
   if (!reader.buffer.eof())
      istr = &reader.buffer;
   std::istream& s = *istr;

   char c;
   s >> ( skipWs ? std::skipws : std::noskipws );
   s.get(c);
   return c;
}
std::string JsonReader::ErrTracker::peekWord() {
   if (reader.buffer.eof()) {
      std::string retVal;
      reader.iStr >> retVal;
      reader.buffer << retVal;
      return retVal;
   }
   return reader.buffer.str();
}
std::string JsonReader::ErrTracker::getWord() {
   if (!reader.buffer.eof()) {
      std::string retVal = ;
      reader.iStr >> retVal;
      reader.buffer << retVal;
      return retVal;
   }
   return reader.buffer.str ();
}
inline void JsonReader::ErrTracker::fillBuffer() {

}



int JsonReader::ErrTracker::operator >>(char& cur) {
   do {
      if (!_str._last.empty()) {
         cur = _str._last.back();
         _str._last.pop_back();
      }
      else if (!((*_str.iStr) >> cur)) {
         if (!_err.empty())
            _str.throwErr(_err);
         return 0;
      }

      if (_skip_comment && _str.flags.test(DecodingFlags::IGNORE_COMMENT) && cur == '/') {
         char prev = cur;
         (*_str.iStr) >> cur;

         // '/*' comment : skip until '*/' is found.
         if (cur == '*') {
            bool prev_is_star = false;
            do {
               (*_str.iStr) >> cur;
               if (prev_is_star && cur == '/')
                  break;
               prev_is_star = cur == '*';
            } while (1);
            (*_str.iStr) >> cur;
         }

         // '//' comment : skip remaining of the line.
         else if (cur == '/') {
            do {
               (*_str.iStr) >> cur;
            } while (cur != '\n');
            (*_str.iStr) >> cur;
         }

         // Not a comment, keep last read character.
         else {
            _str.pushBack(cur);
            cur = prev;
         }
      }
   } while (_skip_ws && std::isspace(cur));
   return 1;
}


JsonReader::JsonReader(std::istream& reader, DecodingOption flag) :
   iStr(reader), flags(flag) {
   pos.line = 0;
   pos.col = -1;
   iStr >> std::noskipws;
}

JsonReader::ErrTracker JsonReader::operator()(std::string err, bool skipWs, bool skipCom) {
   if (!err.empty())
      err = std::string("Unexpected EOF while ") + err;
   else
      err = "Unexpected EOF";
   return ErrTracker(err, skipWs, skipCom, *this);
}

inline void JsonReader::pushBack(char c) {
   _last.push_back(c);
}

int JsonReader::operator >>(char& c) {
   return (*this)("", false, flags.test(DecodingFlags::IGNORE_COMMENT)) >> c;
}

inline void JsonReader::consume() { 
   _last.pop_back(); 
}

char JsonReader::peek(bool skipWs) {
   return operator()("",skipWs,flags.test(DecodingFlags::IGNORE_COMMENT)).peek();
}

std::string JsonReader::jsonToString() {
   std::string retVal;
   char letter;

   do {
      (*this)("Could not decode string : Unexpected end of file.", false) >> letter;
      switch (letter) {
         case '\\': {
            (*this)("decoding escaped char") >> letter;

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
                  if (!flags.test(DecodingFlags::ALLOW_NULL))
                     (*this).throwErr("Found null value reader string");
                  retVal += "\0";
                  break;

               case 'u': {
                  int sum = 0;
                  std::string val;
                  for (int i = 0; i < 4; i++) {
                     (*this)("decoding UTF encoded char") >> letter;
                     val.push_back(letter);
                  }
                  sum = std::stoi(val, 0, 16);

                  if (sum >= 0xDC00)
                     throwErr("Invalid unicode escape");

                  if (sum >= 0xD800) {
                     val.clear();
                     for (int i = 0; i < 4; i++) {
                        (*this)("decoding UTF encoded char") >> letter;
                        val.push_back(letter);
                     }
                     int sum2 = std::stoi(val, 0, 16);

                     if (sum2 < 0xDC00)
                        throwErr("Invalid unicode escape");

                     sum = ((sum - 0xD800) << 10) + (sum2 - 0xDC00) + 0x10000;
                  }

                  char unicode[9];
                  size_t length;
                  if (Utf8::encode(sum, unicode, &length))
                     throwErr("Could not process unicode");

                  unicode[length] = '\0';
                  retVal += unicode;
               }
               break;

               default:
                  throwErr("Invalid escape");
                  break;
            }
         }
         break;

         case '"':
            // End of string.
            return retVal;

         case '\0':
            if (!flags.test(DecodingFlags::ALLOW_NULL))
               throwErr("Found null value reader string");
            /* no break */
         default:
            retVal.push_back(letter);
            break;
      }
   } while (1);
}

static std::optional<Json> parseObject(JsonReader& reader) {
   char cur = reader.peek();

   printf("testing Object with %c\n", cur);

   if (cur != '{') 
      return std::nullopt;

   printf("found Object\n");

   Object obj;

   reader.consume();
   cur = reader("looking for the end of the object").peek();

   while (cur != '}') {
      // Get the key.
      Position startOfLine = reader.pos;
      auto key = reader.extractNext();
      if (!key.has_value() || !key->isOfType<std::string>())
         startOfLine.throwErr("Missing key");

      startOfLine = reader.pos;
      if (reader.flags.test(DecodingFlags::REJECT_DUPLICATE) && obj.count(key->as<std::string>()))
         startOfLine.throwErr("Duplicate value");

      // Get ":"
      startOfLine = reader.pos;
      reader("looking for key value delimiter \':\'") >> cur;
      if (cur == ':') { }
      else if (!reader.flags.test(DecodingFlags::ALLOW_COMMA_ERR))
         startOfLine.throwErr("Expecting array key value delimiter \":\"");

      // Read the associated value.
      startOfLine = reader.pos;
      auto child = reader.extractNext();
      if (!child.has_value())
         startOfLine.throwErr("File ended before getting the value");
      obj.emplace(key->as<std::string>(), std::move(child));

      // Check if there is are remaining values,
      do {
         cur = reader("looking for element delimiter \',\' or closing bracket \'}\'").peek();
         if (cur == ',') reader.consume();
         else if (cur == '}') break;
         else if (reader.flags.test(DecodingFlags::ALLOW_COMMA_ERR)) { /* Do Nothing */ }
         else reader.throwErr("Expected an element delimiter \',\'");
      } while (cur != ',');
   }
   reader.consume();

   return obj;
}


static std::optional<Json> parseArray(JsonReader& reader) {
   char cur = reader.peek();

   printf("testing Array with %c\n", cur);

   if (cur != '[') 
      return std::nullopt;

   printf("found Array \n");

   Array arr;

   reader.consume();
   cur = reader("looking for the end of the array").peek();
   while (cur != ']') {
      // Read the associated value.
      auto child = reader.extractNext();

      Position pos = reader.pos;
      if (!child.has_value())
         pos.throwErr("File ended before getting the value");
      arr.push_back(child);

      // Check if there is are remaining values,
      do{
         cur = reader("looking for element delimiter \',\' or closing bracket \']\'").peek();
         if (cur == ',') reader.consume();
         else if (cur == ']') break;
         else if (reader.flags.test(DecodingFlags::ALLOW_COMMA_ERR)) { /* Do Nothing */ }
         else reader.throwErr("Expected an element delimiter \',\'");
      } while (cur != ',');
   }

   return arr;
}

static std::optional<Json> parseString(JsonReader& reader) {
   char cur = reader.peek();


   printf("testing string with %c\n", cur);
   if (cur != '"') 
      return std::nullopt;

   printf("testing string\n");

   reader.consume();
   return reader.jsonToString();
}

static std::optional<Json> parseNumber(JsonReader& reader) {
   static const std::string validChar = "0123456789.eE";
   char cur = reader.peek();

   printf("testing digit %c\n", cur);
   if (!std::isdigit(cur) && cur != '-' && cur != '+')
      return std::nullopt;


   printf("found digit\n");

   std::string str;
   str.reserve(32);

   Position pos = reader.pos;
   do {
      reader.consume();
      str.push_back(cur);
      cur = reader.peek(false);
   } while(validChar.find(cur) != std::string::npos);

   long int asInt;
   if (elladan::parseString(str, asInt) == str.size())
      return asInt;

   double asDouble;
   if (elladan::parseString(str, asDouble) == str.size())
      return asDouble;

   pos.throwErr("Could not parse something that look like a number");
   return std::nullopt;
}

static std::optional<Json> parseBool(JsonReader& reader) {
   static const std::string validChar = "truefals";
   char cur = tolower(reader.peek());

   printf("testing bool with %c\n", cur);
   if (cur != 't' && cur != 'f')
      return std::nullopt;

   printf("found bool\n");

   bool result = cur == 't' | cur == 'T';
   do {
      reader.consume();
      cur = tolower(reader.peek(false));
   } while(validChar.find(cur) != std::string::npos);

   return Json(result);
}

static std::optional<Json> parseNull(JsonReader& reader) {
   static const std::string expected = "null";

   for (int i = 0; i < expected.size(); i++) {
      char cur = reader.peek(false);
      printf("testing null with %c\n", cur);
      if (tolower(cur) != expected[i]) {
         reader.pushBack(expected[i]);
         return std::nullopt;
      }

      reader.consume();
   }

   if (!reader.flags.test(DecodingFlags::ALLOW_NULL))
      reader.throwErr("Forbidden null found");

   printf("Found null\n");

   return Null();
}

static std::optional<Json> parseUUID(JsonReader& reader) {
   char cur;

   printf("testing uuid with %c\n", reader.peek());

   if (tolower(reader.peek()) != 'u')
      return std::nullopt;
   

   printf("got UUID\n");

   reader.consume();
   if (reader.peek() != '"') {
      reader.pushBack('u');
      return std::nullopt;
   }
   reader.consume();

   std::string str;
   UUID uuid;
   str.reserve(uuid.getSize());

   for (int i = 0; i < uuid.getSize() + 1; i++) {
      reader >> cur;
      if (cur == '"')
         break;
      else if (!std::isalpha(cur) && cur != '-') 
         reader.throwErr("Invalid UUID");
      str.push_back(cur);
   }

   return UUID::fromString(str);
}

static std::optional<Json> parseBinary(JsonReader& reader) {
   char cur;

   printf("testing binary with %c\n", reader.peek());
   if (tolower(reader.peek()) != 'b')
      return std::nullopt;
   
   reader.consume();
   if (reader.peek() != '"') {
      reader.pushBack('b');
      return std::nullopt;
   }
   reader.consume();

   printf("got binary!\n");

   std::stringstream str;
   do {
      reader >> cur;
      if (!std::isalpha(cur)) {
         if (cur == '"')
            break;
         reader.throwErr("Invalid Binary");
      }
      str << (cur);
   } while(true);

   Json bin = std::move(Binary()); //(Binary(str.str()));
   return bin;
}

std::vector<JsonReader::parse> JsonReader::readType = {
   &parseObject, &parseArray, &parseString, &parseNumber, &parseBool, &parseNull,
   &parseUUID, &parseBinary
};

std::optional<Json> JsonReader::extractNext() {
   for (auto ite : readType) {
      auto res = ite(*this);
      if (res.has_value())
         return res;
   }
   return std::nullopt;
}

Json read(std::istream& in_stream, DecodingOption flag) {
   JsonReader reader(in_stream, flag);
   auto res = reader.extractNext();
   return res.value_or(Json());
}

std::vector<Json> extract(std::istream* in_stream, DecodingOption flag, const std::string& path) {
   Json obj = read(in_stream, flag);
   std::vector<Json> retVal;

   for (auto ite : obj.get(path))
      retVal.push_back(*ite);

   return retVal;
}

} } } // namespace elladan::json::jsonSerializer
