
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
#include <sstream>

#include <elladan/FlagSet.h>
#include <elladan/VMap.h>

#include "../json.h"

namespace elladan { 
namespace json { 
namespace bsonSerializer {

enum DecodingFlags {
   ALLOW_NULL          = 1 << 0, /// If set, null value will NOT throw an error.
   REJECT_DUPLICATE    = 1 << 1, /// If set, an error will be thrown if a map index appear multiple time within the map.
};
typedef FlagSet DecodingOption;

struct BsonReader {
public:
   DecodingOption flags;
   std::istream& iStr;

   typedef std::function<elladan::json::Json (BsonReader&)> Deserialize;
   typedef std::function<std::vector<elladan::json::Json> (BsonReader&, std::vector<std::string>&, int)> Extract;
   typedef std::function<void (BsonReader&)> Skip;
   struct DES {
      Deserialize deserialize;
      Extract extract;
      Skip skip;

   };
   static VMap<char, DES> readType;
   
   struct ErrTracker {
   public:
      ErrTracker(const std::string& err, BsonReader& reader);
      char peek();
      char next();
      std::vector<char> read(int count);

      BsonReader& reader;
      const std::string&  error;
   };

   BsonReader(std::istream& in, DecodingOption flag);
   void throwErr(const std::string& err) const;
   ErrTracker operator()(std::string errMessage = "");
   
   Json extractNext(char type);
   std::vector<Json> extractNext(char type, std::vector<std::string>& path, int deepness);
   void skipNext(char type);

};

json::Json read(std::istream& in, DecodingOption flag = DecodingOption());
std::vector<json::Json> extract(std::istream& in, DecodingOption flag, const std::string& path);

}}} // namespaces
