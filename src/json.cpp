/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "json.h"

#include <cassert>

namespace elladan {
namespace json {


Json::Json() : _data(new JsonImp<Null>()) {}
Json::Json(const Json& oth) : _data(oth._data->copy()) { }
Json::Json(Json&& oth) : _data(std::move(oth._data)) {}
Json& Json::operator=(const Json& oth) { _data.reset(oth._data->copy()); return *this; }
JsonType Json::getType() const { return _data->getType(); }
const std::type_info& Json::getTypeInfo() const { return _data->getTypeInfo(); }
bool Json::operator < (const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() <  rhs._data.get(); }
bool Json::operator <=(const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() <= rhs._data.get(); }
bool Json::operator ==(const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() == rhs._data.get(); }
bool Json::operator !=(const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() != rhs._data.get(); }
bool Json::operator >=(const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() >= rhs._data.get(); }
bool Json::operator > (const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() >  rhs._data.get(); }


std::vector<Json*> Json::get(const std::string& uri) {
   // Use const function, then cast away result constness.
   std::vector<Json*> retVal;
   for(auto ite : const_cast<const Json*>(this)->get(uri))
      retVal.push_back(const_cast<Json*>(ite));
   return retVal;
}
std::vector<const Json*> Json::get(const std::string& uri) const {
   std::vector<const Json*> retVal;
   auto path = tokenize(uri, "/");

   std::vector<std::pair<int, const Json&>> toScan = {{0, *this}};

   while ( !toScan.empty() ) {
      auto ite = toScan.back();
      toScan.pop_back();

      // End of the road
      if (ite.first == path.size()) {
         retVal.push_back(&ite.second);
         continue;
      }

      if (path[ite.first] == "**") {
         if (ite.first + 1 == path.size()) {
            retVal.push_back(&ite.second);
            continue;
         }

         if (auto o = ite.second.cast<Object>()) {
            auto child = o->find(path[ite.first + 1]);
            if (child != o->end()) {
               toScan.push_back({ite.first + 2, child->second});
            }
            for (auto& i : *o)
               toScan.push_back({ite.first, i.second});
            continue;
         }

         if (auto a = ite.second.cast<Array>()) {
            for (auto& i : *a)
               toScan.push_back({ite.first, i});
            continue;
         }

         continue;
      }

      if (path[ite.first] == "*") {
         if (auto o = ite.second.cast<Object>()) {
            for (auto& i : *o)
               toScan.push_back({ite.first + 1, i.second});
            continue;
         }

         if (auto a = ite.second.cast<Array>()) {
            for (auto& i : *a)
               toScan.push_back({ite.first + 1, i});
            continue;
         }
         continue;
      }

      if (auto o = ite.second.cast<Object>()) {
         auto child = o->find(path[ite.first]);
         if (child != o->end())
            toScan.push_back({ite.first + 1, child->second});
         continue;
      }

      if (auto a = ite.second.cast<Array>()) {
         for (auto& i : *a)
            toScan.push_back({ite.first + 1, i});
         continue;
      }
   }
   return retVal;
}

template<> Json::JsonIf* Json::JsonImp<Binary>::copy() const { 
   return new JsonImp<Binary>(ele.copy());
}

#define OVERWRITE_TYPE(InitType, NewType) \
      template<> \
      Json::Json(InitType ele, int i)\
      : _data(new JsonImp<NewType>(ele))\
        {}\
        template<> \
        Json& Json::operator=(InitType ele)\
        {\
           _data.reset(new JsonImp<NewType>(ele));\
           return *this;\
        }

OVERWRITE_TYPE(const char*, std::string);
OVERWRITE_TYPE(int16_t, int64_t);
OVERWRITE_TYPE(int32_t, int64_t);
OVERWRITE_TYPE(uint16_t, int64_t);
OVERWRITE_TYPE(uint32_t, int64_t);
OVERWRITE_TYPE(uint64_t, int64_t);
OVERWRITE_TYPE(float, double);

#undef OVERWRITE_TYPE


bool Null::operator < (const Null& rhs) const { return true; }
bool Null::operator <=(const Null& rhs) const { return true; }
bool Null::operator ==(const Null& rhs) const { return true; }
bool Null::operator !=(const Null& rhs) const { return false; }
bool Null::operator >=(const Null& rhs) const { return false; }
bool Null::operator > (const Null& rhs) const { return false; }

}} // namespace elladan::json

#include "serializer/JsonWriter.h"
#include <sstream>
namespace std{
std::string to_string(const elladan::json::Json& ele){
   std::stringstream ret;
   elladan::json::jsonSerializer::write(ele, ret);
   return ret.str();
}
}
