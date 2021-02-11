// /*
//  * BsonReader.cpp
//  *
//  *  Created on: Apr 17, 2017
//  *      Author: daniel
//  */

#include "BsonReader.h"

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

#include "Bson.hpp"

using std::to_string;


namespace elladan { namespace json { namespace bson {

BsonReader::ErrTracker::ErrTracker(const std::string& err, BsonReader& r)
: reader(r)
, error(err)
{ }

char BsonReader::ErrTracker::peek() {
   return reader.iStr.peek();
}
char BsonReader::ErrTracker::next() {
   return reader.iStr.get();
}
std::vector<char> BsonReader::ErrTracker::read(int count) {
   std::vector<char> retVal(count);
   reader.iStr.read(retVal.data(), count);
   return retVal;
}

BsonReader::BsonReader(std::istream& in, DecodingOption flag)
: iStr(in)
, flags(flag)
{ }

void BsonReader::throwErr(const std::string& err) const {
   throw Exception(err + " at " + std::to_string((size_t)iStr.tellg()));
}
BsonReader::ErrTracker BsonReader::operator()(std::string err) {
   if (err.empty()) err = "Unexpected EOF";
   return ErrTracker(err, *this);
}

elladan::json::Json BsonReader::extractNext(char type) {
   auto ite = readType.find(type);
   if (ite == readType.end())
      throwErr(std::string("Could not decode unknown type " + to_string(type)));
   return ite->second.deserialize(*this);
}

std::vector<elladan::json::Json> BsonReader::extractNext(char type, std::vector<std::string>& path, int deepness) {
   auto ite = readType.find(type);
   if (ite == readType.end())
      throwErr(std::string("Could not extract unknown type " + to_string(type)));
   return ite->second.extract(*this, path, deepness);
}

void BsonReader::skipNext(char type) {
   auto ite = readType.find(type);
   if (ite == readType.end())
      throwErr(std::string("Could not skip unknown type " + to_string(type)));
   ite->second.skip(*this);
}

std::string readKey(BsonReader& s) {
   std::string key;
   auto r = s("Unexpected EOF while reading key");

   while (char c = r.next())
      key.push_back(c);

   return key;
}

void skipWithSize(BsonReader& s){
   int32_t size = *(int32_t*)s("Getting size").read(sizeof(size)).data();
   s("Skipping element").read(size);
}
std::vector<Json> noExtract(BsonReader& s, std::vector<std::string>& path, int deepness){
   return std::vector<Json>();
}

Json deserObject(BsonReader& s) {
   int32_t size = *(int32_t*)s("Getting size").read(sizeof(size)).data();

   Object obj;
   auto r = s("Getting type");
   while (char next = r.next()) {
      std::string key = readKey(s);

      if (s.flags.test(REJECT_DUPLICATE)) {
         if (!obj.emplace(key, s.extractNext(next)))
            s.throwErr(std::string("Key " + key + " already exist"));
      }
      else
         obj.setInplace(key, s.extractNext(next));
   }
   return obj;
}
std::vector<Json> extractObject(BsonReader& s, std::vector<std::string>& path, int deepness) {
   int32_t size = *(int32_t*)s("Getting object size").read(sizeof(size)).data();

   std::vector<Json> retVal;
   auto r =  s("Extracting Object");
   while (char next = r.next()) {
      std::string key = readKey(s);

      if (path[deepness] == "**") {
         if (deepness == path.size()-1) {
            retVal.emplace_back(s.extractNext(next));
         }
         else if (path[deepness+1] == key) {
            if (deepness+1 == path.size()-1) {
               retVal.emplace_back(s.extractNext(next));
            }
            else {
               auto res = s.extractNext(next, path, deepness+2);
               retVal.insert(retVal.end(), res.begin(), res.end());
            }
         }
         else {
            s.skipNext(next);
         }
      }
      else if (path[deepness] == "*" || path[deepness] == key) {
         if (deepness+1 == path.size()-1) {
            retVal.emplace_back(s.extractNext(next));
         }
         else {
            auto res = s.extractNext(next, path, deepness+1);
            retVal.insert(retVal.end(), res.begin(), res.end());
         }
      }
      else {
         s.skipNext(next);
      }
   }
   return retVal;
}

Json deserArray(BsonReader& s) {
   s("Getting array size").read(sizeof(int32_t));

   Array arr;
   auto r = s("Getting type");
      while (char next = r.next()) {
      readKey(s);
      arr.emplace_back(s.extractNext(next));
   }
   return arr;
}
std::vector<Json> extractArray(BsonReader& s, std::vector<std::string>& path, int deepness) {
   int32_t size = *(int32_t*)s("Getting array size").read(sizeof(size)).data();

   std::vector<Json> retVal;
   auto r =  s("Extracting Object");
   while (char next = r.next()) {
      std::string key = readKey(s);

      if (path[deepness] == "**" && deepness + 1 == path.size()) {
            retVal.emplace_back(s.extractNext(next));
      }
      else {
         auto res = s.extractNext(next, path, deepness);
         retVal.insert(retVal.end(), res.begin(), res.end());
      }
   }
   return retVal;
}
Json deserString(BsonReader& s) {
   int32_t size = *(int32_t*)s("Getting string size").read(sizeof(size)).data();

   auto r = s("Getting string").read(size);
   std::string str;
   str.reserve(size);
   str.insert(0, r.data(), size-1);
   return str;
}
Json deserBinary(BsonReader& s) {
   int32_t size = *(int32_t*)s("Getting binary size").read(sizeof(int32_t)).data();

   char subtype = s("Getting binary subtype").next();

   switch (subtype) {
      case BINARY_SUBTYPE_GENERIC:
      case BINARY_SUBTYPE_BINARY_OLD:
         return Binary(std::move(s("Reading raw binary").read(size)));

      case BINARY_SUBTYPE_UUID:
      case BINARY_SUBTYPE_UUID_OLD: 
      {
         auto res = s("Reading UUID").read(size);
         return UUID(res);
      }

      default:
         s.throwErr("Unsupported Binary subtype");
         break;
   }

   // Should never return this, but make code analyzer happy.
   return Json();
}

elladan::VMap<char, BsonReader::DES> BsonReader::readType = {
   {ELE_TYPE_OBJECT,       {&deserObject, &extractObject, &skipWithSize}},
   {ELE_TYPE_ARRAY,        {&deserArray, &extractArray, &skipWithSize}},
   {ELE_TYPE_UTF_STRING,   {&deserString, &noExtract, &skipWithSize}},
   {ELE_TYPE_INT64, {
         [](BsonReader& s)->Json { return *(int64_t*)s("Reading int64").read(sizeof(int64_t)).data();},
         &noExtract,
         [](BsonReader& s)->void { s("Skipping int64").read(sizeof(int64_t));}
   }},
   {ELE_TYPE_DOUBLE, {
         [](BsonReader& s)->Json { return *(double*)s("Reading double").read(sizeof(double)).data();},
         &noExtract,
         [](BsonReader& s)->void { s("Skipping double").read(sizeof(double));}
   }},
   {ELE_TYPE_BOOL,   {
         [](BsonReader& s)->Json { return (bool)s("Reading bool").next(); },
         &noExtract,
         [](BsonReader& s)->void { s("Skipping bool").next();}
   }},
   {ELE_TYPE_NULL,   {
         [](BsonReader& s)->Json { if (!s.flags.test(ALLOW_NULL)) s.throwErr("Forbidden NULL data"); return Null(); },
         &noExtract,
         [](BsonReader& s)->void { /* Do nothing */ }
   }},
   // Supplemental type.
   {ELE_TYPE_BIN,          {&deserBinary, &noExtract, &skipWithSize}},
};


json::Json read(std::istream& in, DecodingOption flag) {
   BsonReader ser(in, flag);
   return ser.extractNext(ELE_TYPE_OBJECT);
}


std::vector<json::Json> extract(std::istream& in, const std::string& path, DecodingOption flag){
   BsonReader ser(in, flag);
   auto pathList = elladan::tokenize(path, "/");
   return ser.extractNext(ELE_TYPE_OBJECT, pathList, 0);
}



} } } // namespace elladan::json
