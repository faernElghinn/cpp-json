
/*
 * JsonReader.h
 *
 *  Created on: Apr 28, 2017
 *      Author: daniel
 */

#pragma once

#include <iostream>
#include <string>
#include <type_traits>
#include <functional>
#include <vector>
#include <optional>
#include <sstream>

#include <elladan/FlagSet.h>
#include <elladan/VMap.h>

#include "../json.h"

namespace elladan { 
namespace json { 
namespace jsonSerializer {

enum DecodingFlags {
   ALLOW_NULL          = 1 << 0, /// If set, null value will NOT throw an error.
   REJECT_DUPLICATE    = 1 << 1, /// If set, an error will be thrown if a map index appear multiple time within the map.
   IGNORE_COMMENT      = 1 << 2, /// If set, c/c+++ like comments will be ignored.
   ALLOW_COMMA_ERR     = 1 << 3, /// If set, I will do my best to ignore pesky comma error (missing comma at the end of a line, trailing comma at the end of a list/array, double commas).
};
typedef FlagSet DecodingOption;

struct Position {
   size_t line = 0;
   size_t col = 0;
   void throwErr(const std::string& err) const;
};
struct JsonReader {
public:
   DecodingOption flags;
   std::istream& iStr;
   Position pos;
   std::stringstream buffer;

   typedef std::function<std::optional<elladan::json::Json> (JsonReader&)> parse;
   static std::vector<parse> readType;

   JsonReader(std::istream& in, DecodingOption flag);
   void throwErr(const std::string& err) const;

   struct ErrTracker {
   public:
      ErrTracker(const std::string& err, JsonReader& reader);
      char peekNext(bool skipWs = true);        // The next char.
      char getNext(bool skipWs = true);         // Consume char.
      std::string peekWord(); // The next word.
      void consumeWord();     // Consume last word

      JsonReader& reader;
      const std::string&  error;
   private:
      inline void fillBuffer();
   };

private:
   std::optional<elladan::json::Json> extractNext();
}












   // struct ErrTracker {
   // public:
   //    ErrTracker(const std::string& err, bool skipWs, bool skipCom, JsonReader& str);
   //    int operator >>(char& cur);
   //    char peek();

   //    std::string _err;
   //    JsonReader& _str;
   //    bool _skip_ws;
   //    bool _skip_comment;
   // };

   // JsonReader(std::istream* in, DecodingOption flag);
   // void throwErr(const std::string& err) const;
   // ErrTracker operator()(std::string err = "", bool skipWs = true, bool skipComment = true);
   // inline void pushBack(char c);
   // int operator >>(char& c);
   // char peek(bool skipWs = true);
   // inline void consume();

   // std::string jsonToString();
   // std::optional<Json> extractNext();


Json read(std::istream* in, DecodingOption flag = DecodingOption());
std::vector<Json> extract(std::istream* in, DecodingOption flag, const std::string& path);

}}} // namespaces