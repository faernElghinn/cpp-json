/*
 * JsonTest.cpp
 *
 *  Created on: Jun 12, 2017
 *      Author: daniel
 */

#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "Test.h"

#undef NULL

using std::to_string;
using namespace elladan::json;

// Test if we can instanciate our object.
std::string doConstructionTest() {
    std::string retVal;

    Json emptyJson;
    Json nullJson = Null();
    Json intJson = 0;
    Json uintJson = 0U;
    Json int64Json = 0L;
    Json uint64Json = 0UL;
    float f = 0.;    Json floatJson = f;
    double d = 0.;   Json doubleJson = d;
    Json charJson = "adsfsa";
    Json StringJson = std::string("adsfsa");
    Json arrayJson = Array();
    Json objJson = Object();
    
    Json uuidJson = UUID();
    Json binJson = Binary();

    objJson.as<Object>().operator[]("some") = 3;
    Json ss = objJson;

    return retVal;
}

std::string doAssignTest() {
   std::string retVal;

   Array ar = { 1, 2, 3, "sfda", 1.0, 2.0, Null(), true, false };
   Json obj = Object();
   obj.as<Object>()["test"] = std::move(ar);

   if (obj.as<Object>()["test"].getType() != Json::typeOf<Array>() )
      retVal += "\n not an array";
   else{
      const Array& arr = obj.as<Object>()["test"].as<Array>();

      if (arr.size() != 9)
         retVal += "\n Expected 9 elements";

      if (arr.at(0).getType() != Json::typeOf<int64_t>() )
         retVal += "\n first element is not of type int64_t";
      else if (arr[0].as<int64_t>() != 1 )
         retVal += "\n Invalid value, expected 1";
         
      if (arr.at(3).getType() != Json::typeOf<std::string>() )
         retVal += "\n forth element is not of type string";
      else if (arr[3].as<std::string>() != "sfda" ) 
         retVal += "\n Invalid value, expected 'sdfa', got";
         
      if (arr.at(5).getType() != Json::typeOf<double>() )
         retVal += "\n fifth element is not of type float";
      else if (arr[5].as<double>() != 2.0 )
         retVal += "\n Invalid value, expected '2.0'";

      if (arr.at(6).cast<Null>() == nullptr)
         retVal += "\n sixth element should be Null";
      else if (arr.at(6).cast<bool>() != nullptr)
         retVal += "\n sixth element should be Null";
         
      if (arr.at(8).getType() != Json::typeOf<bool>() )
         retVal += "\n fifth element is not of type bool";
      else if (arr[8].as<bool>() != false )
         retVal += "\n Invalid value, expected false";

    }

    return retVal;
}

std::string doMemTest() {
   std::string retVal;

   Array orig = { 0,1,2,3};
   
   Json copy1 = orig;
   copy1.as<Array>().pop_back();

   orig.push_back(4);
   orig.push_back(5);

   Json copy2 = std::move(orig);

   copy2.as<Array>().push_back(6);

   if (copy1.as<Array>().size() != 3)
      retVal += "\n Expected array of 3 elements, got " + to_string(copy1.as<Array>().size());

   if (copy2.as<Array>().size() != 7)
      retVal += "\n Expected array of 7 elements, got " + to_string(copy2.as<Array>().size());

   return retVal;
}

std::string doCmpTest() {
   std::string retVal;

   Json int_1 = -1;
   Json int1 = 1;
   Json int4 = 4;
   
   Json d4 = 4.0;
   Json d2 = 2.0;

   Json boolf = false;
   Json boolt = true;
   Json boolt2 = true;

   Array arr1 = {false, 2, 4, 2.1 };
   Array arr2 = {false};
   Array arr3 = {"str", "sdf", 123, 1};

   Object o1 = {
      {"asdf", "dfsafd"},
      {"ASDF", "dfsafd"}
   };
   Object o2 = {
      {"asdf", "gdsa"},
      {"ASDF", "sadf"}
   };
   Object o3 = {
      {"sadfsda", "dfs"},
   };

#define CMP(lhs, op, rhs) if (!(lhs op rhs)) retVal += "\n Expected "#lhs " to be " #op" than " #rhs
   CMP(int4, !=, d4);
   CMP(boolt, !=, int1);
   CMP(boolt, !=, d2);

   CMP(int_1, <, int1);
   CMP(int4, >, int1);
   CMP(int4, >, int_1);
   CMP(int1, ==, int1);

   CMP(d2, <, d4);

   CMP(boolf, !=, boolt);
   CMP(boolt2, ==, boolt);

   CMP(arr1, >, arr2);
   CMP(arr1, <, arr3);

   CMP(o1, <, o2);
   CMP(o1, >, o3);
#undef CMP

   return retVal;
}

int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(doConstructionTest());
	EXE_TEST(doAssignTest());
	EXE_TEST(doMemTest());
	EXE_TEST(doCmpTest());
	return valid ? 0 : -1;
}
