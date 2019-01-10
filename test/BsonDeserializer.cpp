/*
 *  Created on: Jun 13, 2017
 * BsonSerializertest.cpp
 *
 *      Author: daniel
 */

#include <elladan/FlagSet.h>
#include <elladan/Stringify.h>
#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

#include "../src/serializer/BsonReader.h"
#include "Test.h"

using std::to_string;

using namespace elladan::json::bsonSerializer;

// bson must always start as a object or array.
static const std::vector<char> BsonNull = {
      0x8, 0x0, 0x0, 0x0, // Size of data.
      0x0a, '1', 0x00, // NULL type, name + end
      0x00 // End of document
};

static const std::vector<char> BsonFalse = {
      0x09, 0x0, 0x0, 0x0, // Size of data.
      0x08, '1', 0x00, 0x00,// false
      0x00 // End of document
};

static const std::vector<char> BsonTrue = {
      0x09, 0x0, 0x0, 0x0, // Size of data.
      0x08, '1', 0x00, 0x01,// true
      0x00 // End of document
};

static const std::vector<char> BsonIntM1 = {
      0x10, 0x0, 0x0, 0x0, // Size of data.
		0x12, '1', 0x00, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, //-1
      0x00 // End of document

};

static const std::vector<char> BsonInt = {
      0x10, 0x0, 0x0, 0x0, // Size of data.
		0x12, '1', 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 0x0807060504030201
      0x00 // End of document
};

static const std::vector<char> BsonDouble = {
      0x10, 0x0, 0x0, 0x0, // Size of data.
      0x01, '1', 0x00,
   	0x00,0x00, 0x00,0x00, 0x00,0x00, (char)0xF0,0x3f,
      0x00 // End of document
};

static const std::vector<char> BsonString = {
      0x11, 0x0, 0x0, 0x0, // Size of data.
      0x02, '1', 0x00,
         0x05, 0x00, 0x00, 0x00,
         'T', 'E', 'S', 'T', 0x00,
      0x00 // End of document
};

static const std::vector<char> BsonArr = {
      0x19, 0x0, 0x0, 0x0, // Size of data.
         0x04, '1', 0x00, // Start of array
            0x11, 0x0, 0x0, 0x0, // size of array
            0x08, '0', 0x00, 0x00,// false
            0x08, '1', 0x00, 0x01,// true
            0x08, '2', 0x00, 0x00,// false
         0x00, // End of Array
      0x00 // End of document
};

static const std::vector<char>  BsonObject = {
      0x25, 0x0, 0x0, 0x0, // Size of data.
         0x03, '1', 0x00, // Start of obj
            0x1d, 0x0, 0x0, 0x0, // size of obj
            0x08, 'T', 'e', 's', 't', '1', 0x00, 0x00,// false
            0x08, 'T', 'e', 's', 't', '2', 0x00, 0x01,// true
            0x08, 'T', 'e', 's', 't', '3', 0x00, 0x00,// false
         0x00, // End of obj
      0x00 // End of document
};

static const std::vector<char> BsonBinary = {
      0x11, 0x0, 0x0, 0x0, // Size of data.
      0x05, '1', 0x00,
         0x04, 0x00, 0x00, 0x00, // Size of bin.
         0x00,
         0x04, 0x03, 0x02, 0x01, // value of data.
      0x00 // End of document
};

static const std::vector<char> BsonUUID = {
      0x1d, 0x0, 0x0, 0x0, // Size of data.
      0x05, '1', 0x00,
         0x10, 0x00, 0x00, 0x00, // Size of bin.
         0x04,
         0x5d, 0x79, (char)0xe1, (char)0x9a, 0x31, 0x7c, 0x4d, 0x42, (char)0x82, (char)0x99, 0x08, (char)0xb5, 0x43, 0x2d, (char)0xb8, 0x03,
      0x00 // End of document
};

std::string expectThrow(const std::vector<char>& source, const std::string& name, DecodingOption opt = DecodingOption()){
   std::stringstream ss;
   ss.write(source.data(), source.size());
   
   try
   {
      Json bson = read(ss, opt);
      return name + " should have thrown";
   }
   catch(const std::exception& e) { }
   return "";
}

std::string deserializeTest(Json expected, const std::vector<char>& source, const std::string& name, DecodingOption opt = DecodingOption()){
   std::stringstream ss;
   ss.write(source.data(), source.size());
   Json bson = read(ss, opt);
   if (bson != expected)
      return "\nInvalid object for type" + name +", expected : \n" + to_string(expected) +"\ngot :\n" + to_string(bson);
   return "";
}

std::string testBsonToJson(){
   std::string retVal;

   retVal += expectThrow(BsonNull, "Null");
   retVal += deserializeTest(Object {{"1", Null()}}, BsonNull, "Null", ALLOW_NULL );
   retVal += deserializeTest(Object {{"1", false}}, BsonFalse, "false" );
   retVal += deserializeTest(Object {{"1", true}}, BsonTrue, "true" );
   retVal += deserializeTest(Object {{"1", -1}}, BsonIntM1, "-1" );
   retVal += deserializeTest(Object {{"1", 0x0807060504030201}}, BsonInt, "0x0807060504030201" );
   retVal += deserializeTest(Object {{"1", "TEST"}}, BsonString, "TEST" );
   retVal += deserializeTest(Object {{"1", Array {false,true,false}}}, BsonArr, "Array" );
   retVal += deserializeTest(Object {{"1", Object { {"Test1",false},{"Test2",true},{"Test3",false} }}}, BsonObject, "Object" );
   retVal += deserializeTest(Object {{"1", Binary("04030201") }}, BsonBinary, "Binary" );
   retVal += deserializeTest(Object {{"1", UUID::fromString("5d79e19a-317c-4d42-8299-08b5432db803") }}, BsonUUID, "UUID" );

   return retVal;
}

int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(testBsonToJson());
	return valid ? 0 : -1;
}
