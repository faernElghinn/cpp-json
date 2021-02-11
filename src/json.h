#pragma once

#include <elladan/UUID.h>
#include <elladan/FlagSet.h>
#include <elladan/Exception.h>
#include <elladan/Binary.h>
#include <elladan/VMap.h>
#include <stddef.h>
#include <stdint.h>
#include <bitset>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include <type_traits>


namespace elladan {
namespace json {

class Json;
class Null;
class Object;
typedef std::vector<Json> Array;
}}

namespace std{
   std::string to_string(const elladan::json::Json&);
   std::string to_string(const elladan::json::Null&);
   std::string to_string(const elladan::json::Object&);
   std::string to_string(const elladan::json::Array&);
}

namespace elladan {
namespace json {

typedef size_t JsonType;

class Json {
public:
   Json();
   Json(const Json& oth);
   Json(Json&& oth);
   Json& operator=(const Json&);
   Json& operator=(Json&&) = default;
   Json(std::initializer_list<Json>);
   Json(std::initializer_list<std::pair<std::string,Json>>);
   
   template<typename T> 
   Json(T ele, std::enable_if_t<!std::is_same<std::remove_reference_t<T>, Json>::value && !std::is_copy_assignable<T>::value, int> = 0) 
   : _data(new JsonImp<std::remove_reference_t<T>>(std::move(ele)))
   {}
   template<typename T> 
   Json(T ele, std::enable_if_t<!std::is_same<std::remove_reference_t<T>, Json>::value && std::is_copy_assignable<T>::value, int> = 0) 
   : _data(new JsonImp<std::remove_reference_t<T>>(ele)) 
   {}
   template<typename T> std::enable_if_t<!std::is_same<std::remove_reference_t<T>, Json>::value, Json&> operator=(T ele) {
      _data.reset(new JsonImp<std::remove_reference_t<T>>(ele));
      return *this;
   }

   template<typename T>       T* cast()       { auto c = dynamic_cast<JsonImp<T>*>(_data.get()); if (!c) return nullptr; return &c->ele; }
   template<typename T> const T* cast() const { auto c = dynamic_cast<JsonImp<T>*>(_data.get()); if (!c) return nullptr; return &c->ele; }

   template<typename T>       T& as()       { auto c = dynamic_cast<JsonImp<T>*>(_data.get()); if (!c) throw Exception( "Invalid type cast"); return c->ele; }
   template<typename T> const T& as() const { auto c = dynamic_cast<JsonImp<T>*>(_data.get()); if (!c) throw Exception( "Invalid type cast"); return c->ele; }

   template<typename T> static JsonType typeOf() { return typeid(T).hash_code(); }
   JsonType getType() const;
   const std::type_info& getTypeInfo() const;
   template<typename T> inline bool isOfType() const { return getType() == typeOf<T>(); }

   bool operator < (const Json& rhs) const;
   bool operator <=(const Json& rhs) const;
   bool operator ==(const Json& rhs) const;
   bool operator !=(const Json& rhs) const;
   bool operator >=(const Json& rhs) const;
   bool operator > (const Json& rhs) const;

   inline std::string toString() const {
      return _data->toString();
   }

protected:
   struct JsonIf {
      virtual ~JsonIf() = default;
      virtual bool operator < (const JsonIf*) const = 0;
      virtual bool operator <=(const JsonIf*) const = 0;
      virtual bool operator ==(const JsonIf*) const = 0;
      virtual bool operator !=(const JsonIf*) const = 0;
      virtual bool operator >=(const JsonIf*) const = 0;
      virtual bool operator > (const JsonIf*) const = 0;
      virtual JsonType getType() const = 0;
      virtual const std::type_info& getTypeInfo() const = 0;
      virtual JsonIf* copy() const = 0;
      virtual std::string toString() const = 0;
   };
   template<typename T>
   struct JsonImp : public JsonIf {
      T ele;
      JsonImp() = default;
      JsonImp(const T& ite) : JsonIf(), ele(ite) {}
      JsonImp(T&& ite) : JsonIf(), ele(std::move(ite)) {}
      bool operator < (const JsonIf* rhs) const { if (getType() != rhs->getType()) return getType() - rhs->getType(); return ele <  static_cast<const JsonImp<T>*>(rhs)->ele; }
      bool operator <=(const JsonIf* rhs) const { if (getType() != rhs->getType()) return getType() - rhs->getType(); return ele <= static_cast<const JsonImp<T>*>(rhs)->ele; }
      bool operator ==(const JsonIf* rhs) const { if (getType() != rhs->getType()) return getType() - rhs->getType(); return ele == static_cast<const JsonImp<T>*>(rhs)->ele; }
      bool operator !=(const JsonIf* rhs) const { if (getType() != rhs->getType()) return getType() - rhs->getType(); return ele != static_cast<const JsonImp<T>*>(rhs)->ele; }
      bool operator >=(const JsonIf* rhs) const { if (getType() != rhs->getType()) return getType() - rhs->getType(); return ele >= static_cast<const JsonImp<T>*>(rhs)->ele; }
      bool operator > (const JsonIf* rhs) const { if (getType() != rhs->getType()) return getType() - rhs->getType(); return ele >  static_cast<const JsonImp<T>*>(rhs)->ele; }
      JsonType getType() const { return typeOf<T>(); }
      const std::type_info& getTypeInfo() const { return typeid(T); }
      JsonIf* copy() const { return new JsonImp<T>(ele); }
      std::string toString() const { return std::to_string(ele); }
   };

   std::unique_ptr<JsonIf> _data;
};

template<> Json::JsonIf* Json::JsonImp<Binary>::copy() const;


#define OVERWRITE_TYPE(Type) \
template<> Json::Json(Type ele, int i); \
template<> Json& Json::operator=(Type ele)

OVERWRITE_TYPE(const char*);
OVERWRITE_TYPE(int16_t);
OVERWRITE_TYPE(int32_t);
OVERWRITE_TYPE(uint16_t);
OVERWRITE_TYPE(uint32_t);
OVERWRITE_TYPE(uint64_t);
OVERWRITE_TYPE(float);

#undef OVERWRITE_TYPE


struct Null{
   Null() = default;
   Null(const Null&) = default;
   Null(Null&&) = default;
   bool operator < (const Null& rhs) const;
   bool operator <=(const Null& rhs) const;
   bool operator ==(const Null& rhs) const;
   bool operator !=(const Null& rhs) const;
   bool operator >=(const Null& rhs) const;
   bool operator > (const Null& rhs) const;
};

class Object : public elladan::VMap<std::string, Json> {
public:
   Object() = default;
   Object(const Object&) = default;
   Object(Object&&) = default;
   Object& operator=(const Object&) = default;
   Object& operator=(Object&&) = default;
   ~Object() = default;
   Object(std::initializer_list<pair> l);

   iterator find(const std::string& key);
   const_iterator find(const std::string& key) const;
   bool insert(const std::string& key, const Json& ele);
   bool emplace(const std::string& key, Json&& ele);
   void set(const std::string& key, const Json& ele);
   void setInplace(const std::string& key, Json&& ele);
   bool erase(iterator& ite);
   bool erase(const std::string& key);
   Json& operator[] (const std::string& key);
   const Json& at(const std::string& key) const;
   Json& at(const std::string& key);
   Json&& take(const std::string& key);

   void clear();
   int count (std::string key) const;
   int size () const;
   void reserve (size_t size);
   iterator begin();
   iterator end();
   const_iterator begin() const;
   const_iterator end() const;
   reverse_iterator rbegin();
   reverse_iterator rend();
   const_reverse_iterator rbegin() const;
   const_reverse_iterator rend() const;

   void sort();
   void sort(std::function<bool(const pair& lhs, const pair& rhs)> sortFnc);

   bool operator< (const Object& rhs) const;
   bool operator<=(const Object& rhs) const;
   bool operator>=(const Object& rhs) const;
   bool operator> (const Object& rhs) const;
   bool operator == (const Object& rhs) const;
   bool operator != (const Object& rhs) const;
};


}} // namespace elladan::json
