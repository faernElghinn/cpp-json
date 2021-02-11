/*
 * JsonValidator.cpp
 *
 *  Created on: May 23, 2019
 *      Author: daniel
 */

#include "../Schema/Schema.h"

#include <bits/stdint-intn.h>
#include <bits/types/struct_tm.h>
#include <elladan/Exception.h>
#include <elladan/Stringify.h>
#include <elladan/VMap.h>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iomanip>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "../pointer/Pointer.h"
#include "Format.hpp"

namespace elladan {
namespace json {



template<typename T>
std::optional<T> getNumber(const Json& num) {
   if (num.isOfType<int64_t>())     return num.as<int64_t>();
   else if (num.isOfType<double>()) return num.as<double>();
   else                             return std::nullopt;
}

template<typename T>
std::optional<T> getVal(const Json& num) {
   if (num.isOfType<T>()) return num.as<T>();
   else                   return std::nullopt;
}

bool operator !(const std::string& str) {
   return !!str.empty();
}

template<typename R>
struct ValidatorOption {
   typedef R (*ValidatePattern)(std::string&) ;
//   typedef R (ValidatorOption::*ValidateFnct) (Json& toValidate, const Object& node);
   typedef std::function<R (ValidatorOption<R>* that, Json& toValidate, const Object& node)> ValidateFnct;

   static elladan::VMap<JsonType, std::string> type2Str;
   static VMap<std::string, ValidatePattern> DefaultPattern;
   static elladan::VMap<JsonType, ValidateFnct> type2Val;


   ValidatorOption(Json& toValidate, const Json& rule, const VMap<std::string, ValidatePattern>& patterns = DefaultPattern);
   const Json& toValidateHead;
   const Json& rulesHead;

   const VMap<std::string, ValidatePattern>& pattern;

// Should be protected.
  static R valNull   (ValidatorOption<R>* that, Json& toValidate, const Object& rule);
  static R valBool   (ValidatorOption<R>* that, Json& toValidate, const Object& rule);
  static R valInt    (ValidatorOption<R>* that, Json& toValidate, const Object& rule);
  static R valNum    (ValidatorOption<R>* that, Json& toValidate, const Object& rule);
  static R valString (ValidatorOption<R>* that, Json& toValidate, const Object& rule);
  static R valArray  (ValidatorOption<R>* that, Json& toValidate, const Object& rule);
  static R valObject (ValidatorOption<R>* that, Json& toValidate, const Object& rule);

   R validate(Json& toValidate, const Object& node);
protected:
   R ret (const Json& currentToValidate, const std::string& err = "") const;

   R valGeneric(Json& toValidate, const Object& node);

   // Combination
   R notOf(Json& toValidate, const Object& node);
   R allOf(Json& toValidate, const Object& node);
   R anyOf(Json& toValidate, const Object& node);
   R oneOf(Json& toValidate, const Object& node);

   // Test type validation
   bool hasType(JsonType type, const Json& typeNode);
};


template <>
std::string ValidatorOption<std::string>::ret (const Json& currentToValidate, const std::string& err) const {
   if (!err.empty())
      return json::getPointer(toValidateHead, currentToValidate).value() + err;
   else
      return err;
}
template <>
bool ValidatorOption<bool>::ret (const Json& currentToValidate, const std::string& err) const {
   return err.empty();
}


template<typename R>
VMap<std::string, R (*)(std::string&)>
ValidatorOption<R>::DefaultPattern = {
   {"date-time", matchDateTime<R>},
   {"date", matchDate<R>},
   {"time", matchTime<R>},
   {"email", matchEmail<R>},
   {"idn-email", matchEmailIdn<R>},
   {"ipv4", matchIPv4<R>},
   {"ipv6", matchIPv6<R>},
   {"hostname", matchHostname<R>},
   {"idn-hostname", matchHostnameIdn<R>},
   {"regex", validateRegex<R>},
   {"uri-template", validateUriTemplate<R>},
   {"json-pointer", validateJsonPointer<R>},
   {"relative-json-pointer", validateRelJsonPtr<R>},
   {"iri", validateIri<R, false>},
   {"iri-reference", validateIri<R, true>}
};
template<typename R>
elladan::VMap<JsonType, std::string>
ValidatorOption<R>::type2Str = {
      {Json::typeOf<Object>(), "object"},
      {Json::typeOf<std::string>(), "string"},
      {Json::typeOf<Array>(), "array"},
      {Json::typeOf<double>(), "number"},
      {Json::typeOf<int64_t>(), "integer"},
      {Json::typeOf<bool>(), "bool"},
      {Json::typeOf<Null>(), "null"}
};
template<typename R>
elladan::VMap<JsonType, std::function<R (ValidatorOption<R>* that, Json& toValidate, const Object& node)> >
ValidatorOption<R>::type2Val = {
      {Json::typeOf<Object>(), &ValidatorOption<R>::valObject},
      {Json::typeOf<std::string>(), &ValidatorOption<R>::valString},
      {Json::typeOf<Array>(), &ValidatorOption<R>::valArray},
      {Json::typeOf<double>(), &ValidatorOption<R>::valNum},
      {Json::typeOf<int64_t>(), ValidatorOption<R>::valInt},
      {Json::typeOf<bool>(), ValidatorOption<R>::valBool},
      {Json::typeOf<Null>(), ValidatorOption<R>::valNull}
};


template<typename R>
ValidatorOption<R>::ValidatorOption(Json& toValidate, const Json& rule, const VMap<std::string, ValidatePattern>& patt)
: rulesHead(rule)
, toValidateHead(toValidate)
, pattern(patt)
{ }


template<typename R>
bool ValidatorOption<R>::hasType(JsonType type, const Json& typeNode) {
   auto tstr = type2Str.find(type);
   if (tstr == type2Str.end())
      return true;

   std::string strType = tstr->second;

   if (const std::string* str = typeNode.cast<std::string>())
      return strType == toLower(*str);

   if (const Array* arr = typeNode.cast<Array>()) {
      for(auto& ite : *arr) {
         auto s = ite.cast<std::string>();
         if (!!s &&  *s == strType)
            return true;
      }
      return false;
   }

   return true;
}

template<typename R>
R ValidatorOption<R>::notOf(Json& toValidate, const Object& node) {
   const Object* obj = node.begin()->second.cast<Object>();
   if (obj && !!validate(toValidate, *obj))
      return ret(toValidate, "Must fail the sub Validation");
   return ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::allOf(Json& toValidate, const Object& node) {
   const Array* arr = node.begin()->second.cast<Array>();
   if (!arr) return ret(toValidate);

   for (const Json& ite : *arr) {
      auto obj = ite.cast<Object>();
      if (!obj) continue;

      auto r = validate(toValidate, *obj);
      if (!r) return r;
   }
   return ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::anyOf(Json& toValidate, const Object& node) {
   const Array* arr = node.begin()->second.cast<Array>();
   if (arr)
      ret(toValidate, "Must match one rule, but there is none");

   if (!std::any_of(arr->begin(), arr->end(), [this, &toValidate](const Json& a) {
      const Object* obj = a.cast<Object>();
      if (!obj) return false;
      return !!validate(toValidate, *obj);
   })) return ret(toValidate, "Did not match any subrules");

   return ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::oneOf(Json& toValidate, const Object& node) {
   const Array* arr = node.begin()->second.cast<Array>();
   if (!arr) return ret(toValidate);

   int prev = -1;
   for (auto i = 0U; i < arr->size(); ++i) {
      auto obj = arr->at(i).cast<Object>();

      if (obj && !validate(toValidate, *obj)) {
         if (prev >= 0)
            return ret(toValidate, std::string("Should match only one of the rules, but matched both [") + std::to_string(prev) + "] and [" + std::to_string(i) + "]");
         else
            prev = i;
      }
   }
   return ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::validate(Json& toValidate, const Object& node) {

   if (node.size() == 1) {
      auto& rul = *node.begin();

      if (rul.first == "not")
         return notOf(toValidate, node);

      if (rul.first == "allOf")
         return allOf(toValidate, node);

      if (rul.first == "anyOf")
         return anyOf(toValidate, node);

      if (rul.first == "oneOf")
         return oneOf(toValidate, node);
   }

   R err = valGeneric(toValidate, node);
   if (!err) return err;

   auto type = node.find("type");
   if (type != node.end()) {
      if (!hasType(toValidate.getType(), type->second)) {
         std::string t;
         if (auto str = type->second.cast<std::string>())
            t = *str;
         else if (const Array* arr = type->second.cast<Array>()) {
            for (const Json& ite : *arr) {
               if (!t.empty()) t += ", ";
               if (const std::string* s = ite.cast<std::string>()) t += *s;
            }
         }
         return ret(toValidate, std::string("Is of the wrong type, should be a ") + t);
      }
   }

   auto val = type2Val.find(toValidate.getType());
   if (val == type2Val.end())
      return ret(toValidate);

   return val->second(this, toValidate, node);
}

template<typename R>
R ValidatorOption<R>::valGeneric(Json& toValidate, const Object& node) {
   auto def = node.find("default");
   if (def != node.end() && toValidate.isOfType<Null>()) // FIXME: does this work? how do we add it to parent object?
      toValidate = def->second;

   auto enu = node.find("enum");
   if (enu != node.end()) {
      if (auto arr = enu->second.cast<Array>()) {
         // Would use std::find, but it does not work.
         bool found = false;
         for (auto& ite : *arr) {
            if (ite != toValidate) continue;
            found = true;
            break;
         }
         if (!found) {
            return ret(toValidate, "Element not in the enum list.");
         }
      }
   }

   auto cns = node.find("const");
   if (cns != node.end()) {
      if (cns->second != toValidate) {
         std::string err = "Element does not match the const value :\n\tExpected:\n";
         err += std::to_string(cns->second);
         err += "\n\ttGot:\n";
         err += std::to_string(toValidate);
         return ret(toValidate, err);
      }
   }

   return ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::valNull(ValidatorOption<R>* that, Json& toValidate, const Object& rule) {
   return that->ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::valBool(ValidatorOption<R>* that, Json& toValidate, const Object& rule) {
   return that->ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::valInt(ValidatorOption<R>* that, Json& toValidate, const Object& rule) {
   int64_t val = toValidate.as<int64_t>();

#define TEST_ELEMENT(NAME, CMP, ERR) do {\
      auto param = rule.find(NAME);\
      if (param != rule.end()) {\
         auto res = getNumber<int64_t>(param->second);\
         if (!!res && (CMP)) return that->ret(toValidate, std::to_string(val) + ERR + std::to_string(res.value()));\
      }\
} while(0)

   TEST_ELEMENT("multipleOf", (val % res.value()) != 0, " cannot be divided by ");
   TEST_ELEMENT("minimum", val < res.value(), " should be above or equals to ");
   TEST_ELEMENT("exclusiveMinimum", val <= res.value(), " should be above ");
   TEST_ELEMENT("maximum", val > res.value(), " should be less or equals to ");
   TEST_ELEMENT("exclusiveMaximum", val >= res.value(), " should be under ");

#undef TEST_ELEMENT
   return that->ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::valNum(ValidatorOption<R>* that, Json& toValidate, const Object& rule) {
   double val = toValidate.as<double>();

#define TEST_ELEMENT(NAME, CMP, ERR) do {\
      auto param = rule.find(NAME);\
      if (param != rule.end()) {\
         auto res = getNumber<double>(param->second);\
         if (!!res && (CMP)) return that->ret(toValidate, std::to_string(val) + ERR + std::to_string(res.value()));\
      }\
} while(0)

   TEST_ELEMENT("multipleOf", ((int )((val / res.value()) * 100.) % 100) != 0, " cannot be divided by ");
   TEST_ELEMENT("minimum", val < res.value(), " should be above or equals to ");
   TEST_ELEMENT("exclusiveMinimum", val <= res.value(), " should be above ");
   TEST_ELEMENT("maximum", val > res.value(), " should be less or equals to ");
   TEST_ELEMENT("exclusiveMaximum", val >= res.value(), " should be under ");

#undef TEST_ELEMENT
   return that->ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::valString(ValidatorOption<R>* that, Json& toValidate, const Object& node) {
   auto val = toValidate.as<std::string>();

#define TEST(Name, Type, TypeString, Test, TestError) do {\
      auto param = node.find(Name);\
      if (param != node.end()) {\
         auto res = getVal<Type>(param->second);\
         if (!!res && (val.size() Test res.value())) return that->ret(toValidate, val + " must " TestError + std::to_string(res.value()) + " letters long");\
      }\
   } while (0)
   TEST("minLength", int64_t, "integer", <, "at least");
   TEST("maxLength", int64_t, "integer", >, "at most");
#undef TEST

   do {
      auto param = node.find("pattern");
      if (param != node.end()) {
         auto res = getVal<std::string>(param->second);
         if (!res) break;

         std::regex reg(res.value());
         if (!std::regex_match(val, reg))
            return that->ret(toValidate, val + " must match the following regex : " + std::to_string(res.value()));
      }
   } while (0);

   do {
      auto param = node.find("format");
      if (param != node.end()) {
         auto res = getVal<std::string>(param->second);
         if (!res) break;

         auto valIte = that->pattern.find(res.value());
         if (valIte != that->pattern.end()) {
            auto err = valIte->second(val);
            if (!err) return err;
         }
      }
   } while (0);

   return that->ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::valArray(ValidatorOption<R>* that, Json& toValidate, const Object& node) {
   Array& val = toValidate.as<Array>();

   auto items = node.find("items");
   if (items != node.end()) {
      if (auto* sche = items->second.cast<Object>()) {
         for (auto ite = val.begin(); ite != val.end(); ++ite) {
            auto err = that->validate(*ite, *sche);
            if (!err) return err;
         }
      }
      else if (auto* arr = items->second.cast<Array>()) {
         auto pos = 0U;
         auto end = std::min(val.size(), arr->size());

         for (auto i = 0U; i < end; i++) {
            auto err = that->validate(val.at(i), arr->at(i).as<Object>());
            if (!err) return err;
         }

         if (val.size() < arr->size())
            return that->ret(toValidate, std::string(" Expecting ") + std::to_string(arr->size()) + " elements, got " + std::to_string(val.size()));

         if (val.size() > arr->size()) {
            auto addItem = node.find("additionalItems");
            if (addItem == node.end() || (addItem->second.isOfType<bool>() && addItem->second.as<bool>())) {
               // It's okay.
            }
            else if (addItem->second.isOfType<bool>() && !addItem->second.as<bool>())
               return that->ret(toValidate, std::string("Cannot have more additional item. Expecting ") + std::to_string(arr->size()) + " elements");
            else if (auto obj = addItem->second.cast<Object>()) {
               for (; end < val.size(); end++) {
                  auto err = that->validate(val.at(end), *obj);
                  if (!err) return err;
               }
            }
         }
      }
   }

   auto cont = node.find("contains");
   if (cont != node.end()) {
      auto& obj = cont->second.as<Object>();
      for (auto i = 0U; i < val.size(); i++) {
         auto err = that->validate(val.at(i), obj);
         if (!err) return err;
      }
   }

   auto min = node.find("minItems");
   if (min != node.end()) {
      auto m = min->second.cast<int64_t>();
      if (m && min->second.as<int64_t>() > val.size())
         return that->ret(toValidate, std::string(" Need to have at least ") + std::to_string(min->second.as<int64_t>()) + " elements.");
   }

   auto max = node.find("maxItems");
   if (max != node.end()) {
      if (max->second.as<int64_t>() < val.size())
         return that->ret(toValidate, std::string(" Need to have at most ") + std::to_string(max->second.as<int64_t>()) + " elements.");
   }

   auto unique = node.find("uniqueItems");
   if (unique != node.end()) {
      if (unique->second.isOfType<bool>() && !unique->second.as<bool>()) {
         for (auto ite = 0U; ite < val.size(); ++ite) {
            for (auto next = ite + 1; next < val.size(); ++next) {
               if (val.at(ite) == val.at(next))
                  return that->ret(toValidate, std::string(" Has duplicate values at ") + std::to_string(ite) + " and " + std::to_string(next));
            }
         }
      }
   }

   return that->ret(toValidate);
}

template<typename R>
R ValidatorOption<R>::valObject(ValidatorOption<R>* that, Json& toValidate, const Object& node) {
   Object& obj = toValidate.as<Object>();

   {
      auto min = node.find("minProperties");
      if (min != node.end()) {
         if (min->second.as<int64_t>() < obj.size())
            return that->ret(toValidate, std::string("Object must have at least ") + std::to_string(min->second.as<int64_t>()) + " properties.");
      }
   }

   {
      auto max = node.find("maxProperties");
      if (max != node.end()) {
         if (max->second.as<int64_t>() > obj.size())
            return that->ret(toValidate, std::string("Object must have at most ") + std::to_string(max->second.as<int64_t>()) + " properties.");
      }
   }

   {
      auto req = node.find("required");
      if (req != node.end() && req->second.isOfType<Array>()) {
         for (auto& ite : req->second.as<Array>()) {
            if (!ite.isOfType<std::string>())
               continue;
            auto r = obj.find(ite.as<std::string>());
            if (r == obj.end())
               return that->ret(toValidate, std::string("Child ") + ite.as<std::string>() + " is required");
         }
      }
   }

   {
      auto dep = node.find("dependencies");
      if (dep != node.end() && dep->second.isOfType<Object>()) {
         for (auto& ite : dep->second.as<Object>()) {
            if (!ite.second.isOfType<Array>())
               continue;

            auto c = obj.find(ite.first);
            if (c == obj.end())
               continue;

            for (auto& re : ite.second.as<Array>()) {
               if (!re.isOfType<std::string>())
                  continue;

               auto r = obj.find(re.as<std::string>());
               if (r == obj.end())
                  return that->ret(toValidate, std::string("Child ") + ite.first + " require the presence of child " + re.as<std::string>());
            }
         }
         for (auto& ite : dep->second.as<Object>()) {
            if (!ite.second.isOfType<Object>())
               continue;

            valObject(that, toValidate, ite.second.as<Object>());
         }
      }
   }

   {
      auto propName = node.find("propertyNames");
      if (propName != node.end() && propName->second.isOfType<std::string>()) {
         std::regex reg(propName->second.as<std::string>());
         for (auto& val : obj) {
            if (!std::regex_match(val.first, reg))
               return that->ret(toValidate, std::string("Key ") + val.first + " does not match propertyNames regex " + propName->second.as<std::string>());
         }
      }
   }

   std::variant<bool, const Object*> addProps = true;
   {
      auto addP = node.find("additionalProperties");
      if (addP != node.end()) {
         if (addP->second.isOfType<bool>())
            addProps = addP->second.as<bool>();
         else if (addP->second.isOfType<Object>())
            addProps = &addP->second.as<Object>();
      }
   }

   const Object* patte = nullptr;
   {
      auto pat = node.find("patternProperties");
      if (pat != node.end())
         patte = pat->second.cast<Object>();
   }

   const Object* props = nullptr;
   {
      auto pro = node.find("patternProperties");
      if (pro != node.end())
         props = pro->second.cast<Object>();
   }

   for (auto& ite : obj) {
      bool found = false;
      if (props) {
         auto ir = props->find(ite.first);
         if (ir != props->end()) {
            found = true;
            auto childRules = ir->second.cast<Object>();
            if (!childRules) continue;
            auto res = that->validate(ite.second, *childRules);
            if (!res) return res;
         }
      }

      if (patte) {
         auto ir = patte->end();
         for (auto& p : *patte) {
            std::regex r(p.first);
            if (std::regex_match(ite.first, r)) {
               found = true;
               auto childRules = ir->second.cast<Object>();
               if (!childRules) continue;
               auto res = that->validate(ite.second, *childRules);
               if (!res) return res;
            }
         }
      }

      if (!found) {
         if (std::holds_alternative<bool>(addProps)) {
            if (!std::get<bool>(addProps))
               return that->ret(toValidate, std::string(": Does not support additional properties. '") + ite.first + "' is invalid.");
         }
         else if (std::holds_alternative<const Object*>(addProps)) {
            auto o = std::get<const Object*>(addProps);
            auto res = that->validate(ite.second, *o);
            if (!res) return res;
         }
      }
   }

   return that->ret(toValidate);
}


std::string shemaValidate(Json& toValidate, const Json& rules) {
   ValidatorOption<std::string> val(toValidate, rules.as<Object>());
   return val.validate(toValidate, rules.as<Object>());
}
bool shemaValidateQuick(Json& toValidate, const Json& rules) {
   ValidatorOption<bool> val(toValidate, rules.as<Object>());
   return val.validate(toValidate, rules.as<Object>());
}

}} // namespace elladan:::json
