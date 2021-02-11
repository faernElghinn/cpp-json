
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

#include "json.h"

namespace elladan { 
namespace json { 
namespace jsonSerializer {

enum DecodingFlags {
   ALLOW_NULL          = 1 << 0, /// If set, null value will NOT throw an error.
   REJECT_DUPLICATE    = 1 << 1, /// If set, an error will be thrown if a map index appear multiple time within the map.
   // IGNORE_COMMENT      = 1 << 2, /// If set, c/c+++ like comments will be ignored. // TODO: re-enable.
   ALLOW_COMMA_ERR     = 1 << 3, /// If set, I will do my best to ignore pesky comma error (missing comma at the end of a line, trailing comma at the end of a list/array, double commas).
};
typedef FlagSet DecodingOption;

struct Position {
   size_t line = 0;
   size_t col = 0;
   void throwErr(const std::string& err) const;
   void update(char c);
};
struct JsonReader {
public:
   DecodingOption flags;
   std::istream& iStr;
   Position pos;
   std::string buffer;

   typedef std::function<std::optional<elladan::json::Json> (JsonReader&)> parse;
   static std::vector<parse> readType;
   
   struct ErrTracker {
   public:
      ErrTracker(const std::string& err, JsonReader& reader);
      char peekNext(bool skipWs = true); // The next char.
      char getNext(bool skipWs = true); // Consume char.
      std::string peekWord(); // The next word.
      std::string getWord(); // Consume last word
      std::string get(int count); // Get the next 'count' characters.

      JsonReader& reader;
      const std::string&  error;
   };

   JsonReader(std::istream& in, DecodingOption flag);
   void throwErr(const std::string& err) const;
   ErrTracker operator()(std::string errMessage = "");
   
   std::optional<elladan::json::Json> extractNext();
};

json::Json read(std::istream& in, DecodingOption flag = DecodingOption());
std::vector<json::Json> extract(std::istream& in, const std::string& path, DecodingOption flag = DecodingOption());

}}} // namespaces
