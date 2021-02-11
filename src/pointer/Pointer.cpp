/*
 * Pointer.cpp
 *
 *  Created on: Aug. 23, 2019
 *      Author: daniel
 */

#include "Pointer.h"

#include <elladan/Stringify.h>
#include <elladan/VMap.h>
#include <vector>
#include <variant>

namespace elladan {
namespace json {

std::optional<std::string> getPointer(const Json& head, const Json& current) {
   if (&current == &head)
     return "";
   else if (auto obj = head.cast<Object>()) {
      for (auto& ite : *obj) {
         auto res = getPointer(ite.second, current);
         if (!!res)
            return std::string("/") + ite.first + res.value();
      }
   }
   else if (auto arr = head.cast<Array>()) {
      for (auto i = 0U; i < arr->size(); i++) {
         auto res = getPointer(arr->at(i), current);
         if (!!res)
            return std::string("/") + std::to_string(i) + res.value();
      }
   }

   return std::nullopt;
}
std::optional<std::string> getRelativePointer(const Json& head, const Json& current, const Json& target) {
   auto from = getPointer(head, current);
   if (!from) return from;
   auto to = getPointer(head, target);
   if (!to) return to;

   std::string fr = from.value();
   std::string t = to.value();

   // Find last '/' on the common path.
   if (fr == t) return "0";

   auto lastSlash = std::string::npos;
   for (auto i = 0U; i < std::min(fr.size(), t.size()); i++) {
      if (fr.at(i) != t.at(i)) break;
      if (fr.at(i) == '/') lastSlash = i;
   }


   // Cut the path in common.
   if (lastSlash != std::string::npos) {
      fr = fr.substr(lastSlash);
      t = t.substr(lastSlash);
   }

   // Count how many '/' there is in the current path.
   auto c = std::count(fr.begin(), fr.end(), '/');
   if (!t.empty())
      t = std::to_string(c) + t;
   else
      t = std::to_string(c);

   return t;
}
std::optional<std::string> relToAbsPointer(const std::string& relativePtr, const Json& head, const Json& current) {
   if (relativePtr.empty() || !std::isdigit(relativePtr.front()))
      return relativePtr;

   auto ptr = getPointer(head, current);
   if (!ptr) return ptr;

   auto& p = ptr.value();

   std::string str;
   std::string remain;

   auto ite = relativePtr.find('/');
   if (ite != std::string::npos) {
      str = relativePtr.substr(0, ite);
      remain = relativePtr.substr(ite);
   }
   else
      str = relativePtr;

   bool returnKey = !str.empty() && relativePtr.back() == '#';

   if (returnKey && !remain.empty())
      throw Exception(std::string("Cannot take index of relative path: ") + relativePtr);

   if (returnKey)
      str.erase(str.end()-1);

   if (!elladan::isInt(str))
      throw Exception(std::string("Invalid relative json pointer : ") + relativePtr);

   int retBy = std::stoi(str);
   while (retBy-- > 0) {
      auto last = p.find_last_of("/");
      if (last != std::string::npos)
         p = p.substr(0, last);
      else if (p.empty())
         throw elladan::Exception("Pointer going back over root!");
      else
         p.clear();
   }

   return p + remain;
}

Json* get(const std::string& ptrPath, Json& head) {
   return const_cast<Json*>(get(ptrPath, (const Json&)head));
}

std::variant<Json*, std::string> get(const std::string& ptrPath, Json& head, Json& current) {
   auto res = get(ptrPath, (const Json&)head, (const Json&)current);

   if (std::holds_alternative<const Json*>(res))
      return const_cast<Json*>(std::get<const Json*>(res));

   return std::get<std::string>(res);
}

const Json* get(const std::string& ptrPath, const Json& head) {
   auto res = get(ptrPath, head, head);

   if (std::holds_alternative<const Json*>(res))
      return std::get<const Json*>(res);

   return nullptr;
}

std::variant<const Json*, std::string> get(const std::string& ptrPath, const Json& head, const Json& current) {
   std::vector<std::string> pathes = elladan::tokenize(ptrPath, "/");

   // Relative path.
   if (!pathes.front().empty()) {
      // FIXME: improve performance by having a parent reference or something!

      // Convert to absolute path.
      auto r = relToAbsPointer(ptrPath, head, current);
      if (!r) return nullptr;

      if (!pathes.empty() && !pathes.back().empty() && pathes.back().back() == '#') {
         auto ite = r.value().find_last_of("/");
         if (ite == std::string::npos)
            return nullptr;
         else
            return r.value().substr(ite+1);
      }

      pathes = elladan::tokenize(r.value(), "/");
   }

   // Absolute path
   const Json* parent = &head;
   int i = 0;
   do {
      if (i >= pathes.size())
         return parent;

      auto name = pathes[i];
      if (name == "")
      {
         // Empty name means ourself.
      }

      else if (i >= pathes.size())
         return parent;

      else if (auto obj = parent->cast<Object>()) {
         auto ite = obj->find(name);
         if (ite == obj->end())
            return nullptr;

         parent = &ite->second;
      }

      else if (auto arr = parent->cast<Array>()) {
         if (!elladan::isInt(name)) {
            std::string errPath;
            for (int j = 0; j <=i; j++) {
               errPath.push_back(ptrPath[j]);
               errPath.push_back('/');
            }
            throw elladan::Exception("Invalid pointer syntax. Array only support number or '-' : " + errPath);
         }

         size_t idx = std::stoi(name);
         if (idx >= arr->size())
            return nullptr;

         parent = &arr->at(idx);
      }

      else
         return nullptr;

      ++i;
   } while(1);
   return nullptr;
}

void set(const std::string& ptrPath, Json& head, Json& value, bool appendOnly) {
   set(ptrPath, head, head, value, appendOnly);
}
void set(const std::string& ptrPath, Json& head, Json& current, Json& value, bool appendOnly) {
   std::vector<std::string> pathes = elladan::tokenize(ptrPath, "/");

   // Relative path.
   if (!pathes.front().empty()) {
      // FIXME: improve performance by having a parent reference or something!

      // Convert to absolute path.
      auto r = relToAbsPointer(ptrPath, head, current);
      if (!r) return;
      pathes = elladan::tokenize(r.value(), "/");
   }

   // Absolute path
   Json* parent = &head;
   int i = 0;
   do {
      if (i >= pathes.size()) {
         if (appendOnly)
            throw elladan::Exception("Setting value would destroy the value : " + ptrPath);

         *parent = value;
         return;
      }

      auto& name = pathes[i];
      if (name == "")
      {
         // Empty name means ourself.
      }

      else if (auto obj = parent->cast<Object>())
         parent = &obj->operator [] (name);

      else if (auto arr = parent->cast<Array>()) {
         if (name == "-") {
            arr->emplace_back(Json());
            parent = &arr->back();
         }
         else if (appendOnly && !elladan::isInt(name)) {
            std::string errPath;
            for (int j = 0; j <=i; j++) {
               errPath.push_back(ptrPath[j]);
               errPath.push_back('/');
            }
            throw elladan::Exception("Setting value would destroy the array : " + errPath);
         }
         else if (!appendOnly && !elladan::isInt(name)) {
            Object&& o = Object();
            auto n = &o[name];
            *parent = o;
            parent = n;
         }
         else {
            size_t idx = std::stoi(name);
            if (idx >= arr->size())
               arr->resize(idx);
            parent = &arr->at(idx);
         }
      }

      else if (appendOnly) {
         std::string errPath;
         for (int j = 0; j <=i; j++) {
            errPath.push_back(ptrPath[j]);
            errPath.push_back('/');
         }
         throw elladan::Exception("Setting value would destroy the value : " + errPath);
      }

      else {
         if (name == "-") {
            Array&& a = Array();
            a.emplace_back(Json());
            auto n = &a.back();
            *parent = a;
            parent = n;
         }
         else {
            Object&& o = Object();
            auto n = &o[name];
            *parent = o;
            parent = n;
         }
      }

      ++i;
   } while(1);
}


} /* namespace json */
} /* namespace elladan */
