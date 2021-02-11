/*
 * JsonSerializertest.cpp
 *
 *  Created on: Jun 13, 2017
 *      Author: daniel
 */

#include <elladan/Stringify.h>
#include <elladan/UUID.h>
#include <elladan/Binary.h>
#include <bitset>
#include <exception>
#include <memory>
#include <sstream>
#include <vector>

#include "../src/json.h"
#include "../src/JsonReader.h"
#include "Test.h"

using std::to_string;
using namespace elladan::json::jsonSerializer;

static const std::string ExpectWS =
        "{\n"
            "\t\"arr\" : [\n"
            "\t],\n"
            "\t\"obj\" : {\n"
            "\t},\n"
            "\t\"string\" : \"testing how this work\",\n"
            "\t\"int\" : 0,\n"
            "\t\"fill_arr\" : [\n"
                "\t\t0,\n"
                "\t\t1,\n"
                "\t\t2,\n"
                "\t\t3\n"
            "\t],\n"
            "\t\"bool\" : true,\n"
            "\t\"double\" : 3.141500,\n"
            "\t\"null\" : null\n"
        "}";

static std::string test_throw_exep(const std::string& txt, DecodingOption opt = DecodingOption()) {
    std::string retVal;
    std::stringstream ss;
    ss << txt;

    try{
        Json obj = jsonSerializer::read(ss, opt);
        retVal += "\nDecoding \"" + txt + "\" should have thrown an exception.";
    }  catch (std::exception& e) {    }
    return retVal;
}

template<typename T>
std::string test_mode_value_noFormat(std::string txt, DecodingOption opt = DecodingOption()) {
   try{ 
      std::stringstream ss;
      ss << txt; 
      Json obj = jsonSerializer::read(ss, opt); 
      if (!obj.isOfType<T>()) 
         return std::string("\nCould not decode \"") +  txt  + "\" as " + typeid(T).name(); 
   } 
   catch (std::exception& e) { 
      return std::string("\nCould not decode \"") + txt + "\" as " + typeid(T).name() + " " + e.what(); 
   }
	return "";
}

template<typename T>
std::string test_mode_value(std::string txt, T expected, DecodingOption opt = DecodingOption()) {
   try{
      std::stringstream ss;
      ss << txt;
      Json obj = jsonSerializer::read(ss, opt);
      if (!obj.isOfType<T>())
         return std::string("\nCould not decode \"") + txt + "\" as "+ typeid(T).name();
      else if (obj.as<T>() != expected) {
         return std::string("\nInvalid value for \"") + txt + "\", expected " + to_string(expected) + " got " + to_string(obj.as<T>());
      }
   }
   catch (std::exception& e) {
      return std::string("\nCould not decode \"") + txt + "\" as " + typeid(T).name() + " :" + e.what();
   }
	return "";
}

std::string testTxtToJson(){
   std::string retVal;
   retVal += test_throw_exep("null");
   retVal += test_mode_value_noFormat<Null>("null", DecodingFlags::ALLOW_NULL);

   retVal += test_mode_value<bool>("true", true);
   retVal += test_mode_value<bool>("false", false);

   retVal += test_mode_value<int64_t>("2", 2);
   retVal += test_mode_value<int64_t>("-100", -100);
   retVal += test_mode_value<int64_t>("07", 7);

   retVal += test_mode_value<double>("7.", 7.);
   retVal += test_mode_value<double>("0.07", 0.07);
   retVal += test_mode_value<double>("-200.34", -200.34);
   retVal += test_mode_value<double>("-200.34e-17", -200.34e-17);

   retVal += test_mode_value<std::string>("\"Some text with \\\"\"", "Some text with \"");

   retVal += test_mode_value<Array>("[1,2,3]", {1,2,3});
   retVal += test_mode_value<Array>("[ 1,\t2, 3 ]", {1,2,3});
   retVal += test_throw_exep("[,1,,2 3,]");
   retVal += test_mode_value<Array>("[,1,,2 3,]", {1,2,3}, DecodingFlags::ALLOW_COMMA_ERR);

	Object t1;	t1.insert("key1", 1);	t1.insert("key2", 3.1415);
   retVal += test_mode_value<Object>("{\"key1\":1,\"key2\":3.1415}", t1);
   retVal += test_mode_value<Object>("{,,\"key1\"1 \"key2\" :\t3.1415,,}", t1, DecodingFlags::ALLOW_COMMA_ERR);
   retVal += test_throw_exep("{\"key1\":1,\"key1\":2}", DecodingFlags::REJECT_DUPLICATE);

   // retVal += test_mode_value<Object>("/* Test de comm\n" 
	// "taire\n" 
	// "*/\n"
	// "{\n" 
	// "\"key1\":1,//Cmoment\n"
	// "\"key2\":3.1415 //Plus\n"
	// "// Even more comments\n"
	// "}//endWithComments", t1, DecodingFlags::IGNORE_COMMENT);

	Object t2;
	t2.emplace("arr", Array());
	t2.emplace("obj", Object());
	t2.emplace("string", "testing how this work");
	t2.emplace("int", 0);
	t2.emplace("fill_arr", Array {0,1,2,3});
	t2.emplace("bool", true);
	t2.emplace("double", 3.1415);
	t2.emplace("null", Null());
   retVal += test_mode_value<Object>(ExpectWS, t2, DecodingFlags::ALLOW_NULL);

	retVal += test_mode_value<UUID>("u\"5d79e19a-317c-4d42-8299-08b5432db803\"", elladan::UUID("5d79e19a-317c-4d42-8299-08b5432db803"));
	retVal += test_mode_value<Binary>("b\"01234567\"", elladan::Binary("01234567"));

   return retVal;
}

int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(testTxtToJson());
	return valid ? 0 : -1;
}
