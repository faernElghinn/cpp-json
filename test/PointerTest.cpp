/*
 * JsonTest.cpp
 *
 *  Created on: Jun 12, 2017
 *      Author: daniel
 */

#include "../src/pointer/Pointer.h"

#include "Test.h"

#undef NULL

using std::to_string;
using namespace elladan::json;

const Json root =
      (Object){
            {"a", (Object){
               {"a", (Object){ {"0",0}, {"1",1}, {"2",2} }},
               {"b", (Object){ {"0",1}, {"1",2}, {"2",3} }}
            }},
            {"b", (Array){
                (Object){ {"2",4}, {"3",5}, {"4",6} },
                (Object){ {"5",7}, {"6",8}, {"7",9} }
            }}
         };


std::string doSetTest() {
   std::string retVal;

   {
      Json cp = root;
      Json ad = 1;
      set("/c", cp, ad, false);
      if (*get("/c", cp) != ad)
         retVal += "\n Insert failed /c";
   }

   {
      Json cp = root;
      Json ad = 1;
      set("/a", cp, ad, false);
      if (*get("/a", cp) != ad)
         retVal += "\n Insert failed /a";
   }

   try {
      Json cp = root;
      Json ad = 1;
      set("/a", cp, ad, true);
      retVal += "\n /a Should not be inserted";
   } catch (std::exception& e) {
   }

   try {
      Json cp = root;
      Json ad = 1;
      set("/b/a", cp, ad, true);
      retVal += "\n /b/a Should not be inserted";
   } catch (std::exception& e) {
   }

   {
      Json cp = root;
      Json ad = 1;
      set("0/c", cp, cp.as<Object>().at("a"), ad, false);
      if (*get("/a/c", cp) != ad)
         retVal += "\n Insert failed 0/c";
   }

   {
      Json cp = root;
      Json ad = 1;
      set("1/c", cp, cp.as<Object>().at("a"), ad, false);
      if (*get("/c", cp) != ad)
         retVal += "\n Insert failed 1/c";
   }

   return retVal;
}

std::string doGetAbsTest() {
   std::string retVal;

   std::variant<const Json*, std::string> res;
   Json expect;

#define Test(path)\
   do{\
      res = get(path, root);\
      if (!std::holds_alternative<const Json*>(res)) {\
         retVal += "\n Invalid return type, expected a Json ptr : ";\
         retVal += path;\
      }\
      else if (!std::get<const Json*>(res)) {\
         retVal += "\n Could not get a result from query : ";\
         retVal += path;\
      }\
      else if(*std::get<const Json*>(res) != expect) {\
         retVal += "\n Invalid result from query : ";\
         retVal += path;\
         retVal += "\n Expected : \n";\
         retVal += std::to_string(expect);\
         retVal += "\n Got : \n";\
         retVal += std::to_string(*std::get<const Json*>(res));\
      }\
   }\
   while(0)

   // Absolute path.
   expect = 0;
   Test("/a/a/0");

   expect = (Object){{"0",0},{"1",1},{"2",2}};
   Test("/a/a");

   expect = root.as<Object>().at("b");
   Test("/b");

#undef Test

   do {
      res = get("/c", root);
      if (!std::holds_alternative<const Json*>(res)) {
         retVal += "\n Invalid return type, expected a Json ptr : ";
         retVal += "/c";
      }
      else if (std::get<const Json*>(res))
         retVal += "\n Expected no result from query : /c";
   } while(0);

   return retVal;
}

std::string doRelPtrTest() {
   std::string retVal;

   const Json& curr = root
         .as<Object>().at("a")
         .as<Object>().at("a")
         .as<Object>().at("0");

   std::variant<const Json*, std::string> res;
   Json expect;

#define TestIdx(path, want)\
      do{\
         res = get(path, root, curr);\
         if (!std::holds_alternative<std::string>(res)) {\
            retVal += "\n In : ";\
            retVal += path;\
         }\
         else {\
            std::string r = std::get<std::string>(res); \
            if(r != want) {\
               retVal += "\n Invalid result from query : ";\
               retVal += path;\
               retVal += "\n Expected : " want;\
               retVal += "\n Got : ";\
               retVal += r;\
            }\
         }\
      } while(0)

#define TestJson(path)\
   do{\
      res = get(path, root, curr);\
      if (!std::holds_alternative<const Json*>(res)) {\
         retVal += "\n In : ";\
         retVal += path;\
      }\
      else {\
         const Json* r = std::get<const Json*>(res); \
         if (!r) {\
            retVal += "\n Could not get a result from query : ";\
            retVal += path;\
         }\
         else if(*r != expect) {\
            retVal += "\n Invalid result from query : ";\
            retVal += path;\
            retVal += "\n Expected : ";\
            retVal += std::to_string(expect);\
            retVal += "\n Got : ";\
            retVal += std::to_string(*r);\
         }\
      }\
   } while(0)

   // Absolute path.

   TestIdx("0#", "0");
   TestIdx("1#", "a");
   TestIdx("2#", "a");

   expect = (Object){{"0",0},{"1",1},{"2",2}};
   TestJson("3/a/a");

   expect = root.as<Object>().at("a").as<Object>().at("b");
   TestJson("2/b");

   res = get("3/c", root, curr);
   if (!std::holds_alternative<const Json*>(res))
         retVal += "\n Invalid return type, expected a Json ptr : ";
   else if (std::get<const Json*>(res))
      retVal += "\n Expected no result from query : /c";

#undef TestJson
#undef TestIdx

   return retVal;
}


std::string doGenAbsPtrTest() {
   std::string retVal;
   std::optional<std::string> res;

#define TEST(expected, child)\
   do {\
      res = getPointer(root, child);\
      if (!res)\
         retVal += "\n Could not get pointer ";\
      else if (res.value() != expected)\
         retVal += std::string("\n Expected ") + expected + ". Got " + res.value();\
   } while(0)

   TEST("", root);
   TEST("/a", root.as<Object>().at("a"));
   TEST("/b", root.as<Object>().at("b"));
   TEST("/b/1", root.as<Object>().at("b").as<Array>().at(1));
   TEST("/a/a", root.as<Object>().at("a").as<Object>().at("a"));
   TEST("/a/a/2", root.as<Object>().at("a").as<Object>().at("a").as<Object>().at("2"));

#undef TEST
   std::optional<std::string> getRelativePointer(const Json& head, const Json& current, const Json& target);
   std::optional<std::string> relToAbsPointer(const std::string& ptrPath, const Json& head, const Json& current);


   return retVal;
}

std::string doGenRelPtrTest() {
   std::string retVal;
   std::optional<std::string> res;

#define TEST(expected, cur, targ)\
   do {\
      res = getRelativePointer(root, cur, targ);\
      if (!res)\
         retVal += "\n Could not get pointer ";\
      else if (res.value() != expected)\
         retVal += std::string("\n Expected ") + expected + ". Got " + res.value();\
   } while(0)

   TEST("0", root, root);
   TEST("0/a", root, root.as<Object>().at("a"));
   TEST("0/a/a/0", root, root.as<Object>().at("a").as<Object>().at("a").as<Object>().at("0"));
   TEST("0/b", root, root.as<Object>().at("b"));

   TEST("1", root.as<Object>().at("b"), root);
   TEST("2", root.as<Object>().at("a").as<Object>().at("b"), root);
   TEST("3", root.as<Object>().at("a").as<Object>().at("b").as<Object>().at("0"), root);

   TEST("0", root.as<Object>().at("b"), root.as<Object>().at("b"));
   TEST("2/b", root.as<Object>().at("a").as<Object>().at("b"), root.as<Object>().at("b"));
   TEST("2/a/a/2", root.as<Object>().at("b").as<Array>().at(1), root.as<Object>().at("a").as<Object>().at("a").as<Object>().at("2"));
#undef TEST

   return retVal;
}

std::string doRelToAbsTest() {
   std::string retVal;
   std::optional<std::string> res;

#define TEST(expected, path, cur)\
   do {\
      res = relToAbsPointer(path, root, cur);\
      if (!res)\
         retVal += "\n Could not get pointer : " path;\
      else if (res.value() != expected)\
         retVal += std::string("\n Expected ") + expected + ". Got " + res.value();\
   } while(0)

   TEST("", "0", root);
   TEST("/b/2/2", "1/b/2/2", root.as<Object>().at("a"));
   TEST("/a/b/2", "1/b/2", root.as<Object>().at("a").as<Object>().at("a"));
#undef TEST

   return retVal;
}

// FIXME Test error case. What happen if child is not in root? If relative path is instead abs? if not a valid path?. If we expected an object?

int main(int argc, char **argv) {
	bool valid = true;

	EXE_TEST(doGenAbsPtrTest());
	EXE_TEST(doGenRelPtrTest());
	EXE_TEST(doRelToAbsTest());

	EXE_TEST(doGetAbsTest());
	EXE_TEST(doRelPtrTest());

	EXE_TEST(doSetTest());

	return valid ? 0 : -1;
}
