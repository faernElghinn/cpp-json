/*Map
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "json.h"

#include <elladan/Stringify.h>

namespace elladan {
namespace json {


Json::Json() : _data(new JsonImp<Null>()) {}
Json::Json(const Json& oth) : _data(oth._data->copy()) { }
Json::Json(Json&& oth) : _data(std::move(oth._data)) {}
Json& Json::operator=(const Json& oth) { _data.reset(oth._data->copy()); return *this; }


Json::Json(std::initializer_list<Json> list)
: Json(Array())
{
   Array& arr = as<Array>();
   for(auto&& ite : list)
       arr.emplace_back(ite);
}

Json::Json(std::initializer_list<std::pair<std::string,Json>> list)
: Json(Object())
{
   Object& obj = as<Object>();
   for (auto&& ite : list)
      obj.insert(ite.first, ite.second);
}

JsonType Json::getType() const { return _data->getType(); }
const std::type_info& Json::getTypeInfo() const { return _data->getTypeInfo(); }
bool Json::operator < (const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() <  rhs._data.get(); }
bool Json::operator <=(const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() <= rhs._data.get(); }
bool Json::operator ==(const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() == rhs._data.get(); }
bool Json::operator !=(const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() != rhs._data.get(); }
bool Json::operator >=(const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() >= rhs._data.get(); }
bool Json::operator > (const Json& rhs) const { assert(_data.get()); assert(rhs._data.get()); return *_data.get() >  rhs._data.get(); }

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


typedef elladan::VMap<std::string, Json> OMap;

Object::Object(std::initializer_list<pair> l) : OMap(l) {}

Object::iterator Object::find(const std::string& key) { return OMap::find(key); }
Object::const_iterator Object::find(const std::string& key) const {  return OMap::find(key); }

bool Object::insert(const std::string& key, const Json& ele) { return OMap::insert(key, ele); }
bool Object::emplace(const std::string& key, Json&& ele) { return OMap::emplace(key, std::move(ele)); }
void Object::set(const std::string& key, const Json& ele) { OMap::set(key, ele); }
void Object::setInplace(const std::string& key, Json&& ele) { OMap::setInplace(key, std::move(ele)); }
bool Object::erase(iterator& ite) { return OMap::erase(ite); }
bool Object::erase(const std::string& key) { return OMap::erase(key); }
Json& Object::operator[] (const std::string& key) { return OMap::operator [](key); }
const Json& Object::at(const std::string& key) const { return OMap::at(key); }
Json& Object::at(const std::string& key) { return OMap::at(key); }
Json&& Object::take(const std::string& key) { return std::move(OMap::take(key)); }
void Object::clear() { return OMap::clear(); }
int Object::count (std::string key) const { return OMap::count(key); }
int Object::size () const { return OMap::size(); }
void Object::reserve (size_t size) { OMap::reserve(size); }
Object::iterator Object::begin() { return OMap::begin(); }
Object::iterator Object::end() { return OMap::end(); }
Object::const_iterator Object::begin() const { return OMap::begin(); }
Object::const_iterator Object::end() const { return OMap::end(); }
Object::reverse_iterator Object::rbegin() { return OMap::rbegin(); }
Object::reverse_iterator Object::rend() { return OMap::rend(); }
Object::const_reverse_iterator Object::rbegin() const { return OMap::rbegin(); }
Object::const_reverse_iterator Object::rend() const { return OMap::rend(); }

void Object::sort() { return OMap::sort(); }
void Object::sort(std::function<bool(const pair& lhs, const pair& rhs)> sortFnc) { return OMap::sort(sortFnc); }

bool Object::operator< (const Object& rhs) const { return OMap::operator< (rhs); }
bool Object::operator<=(const Object& rhs) const { return OMap::operator<=(rhs); }
bool Object::operator>=(const Object& rhs) const { return OMap::operator>=(rhs); }
bool Object::operator> (const Object& rhs) const { return OMap::operator> (rhs); }
bool Object::operator == (const Object& rhs) const { return OMap::operator== (rhs); }
bool Object::operator != (const Object& rhs) const { return OMap::operator!= (rhs); }

}} // namespace elladan::json

namespace std {

std::string to_string(const elladan::json::Json& ele){
   return ele.toString();
}

std::string to_string(const elladan::json::Null&){
   return "Null";
}

}

