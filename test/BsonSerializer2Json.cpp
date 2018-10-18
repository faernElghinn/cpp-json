/*
 * BsonSerializertest.cpp
 *
 *  Created on: Jun 13, 2017
 *      Author: daniel
 */

#include <elladan/Stringify.h>
#include <bitset>
#include <exception>
#include <memory>
#include <sstream>
#include <vector>
#include <cstdio>

#include "Test.h"

// bson must always start as a object or array.
static const unsigned char BsonNull[] = {
        0x8, 0x0, 0x0, 0x0, // Size of data.
        0x0a, '1', 0x00, // NULL type, name + end
        0x00 // End of document
};
static const unsigned char BsonFalse[] = {
        0x09, 0x0, 0x0, 0x0, // Size of data.
        0x08, '1', 0x00, 0x00,// false
        0x00 // End of document
};
static const unsigned char BsonTrue[] = {
        0x09, 0x0, 0x0, 0x0, // Size of data.
        0x08, '1', 0x00, 0x01,// true
        0x00 // End of document
};
static const unsigned char BsonIntM1[] = {
        0x10, 0x0, 0x0, 0x0, // Size of data.
		0x12, '1', 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //-1
        0x00 // End of document
};
static const unsigned char BsonInt[] = {
        0x10, 0x0, 0x0, 0x0, // Size of data.
		0x12, '1', 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // 0x0807060504030201
        0x00 // End of document
};
static const unsigned char BsonDouble[] = {
        0x10, 0x0, 0x0, 0x0, // Size of data.
        0x01, '1', 0x00,
		0x00,0x00, 0x00,0x00, 0x00,0x00, 0xF0,0x3f,
        0x00 // End of document
};
static const unsigned char BsonString[] = {
        0x11, 0x0, 0x0, 0x0, // Size of data.
        0x02, '1', 0x00,
            0x05, 0x00, 0x00, 0x00,
            'T', 'E', 'S', 'T', 0x00,
        0x00 // End of document
};
static const unsigned char BsonArr[] = {
        0x19, 0x0, 0x0, 0x0, // Size of data.
            0x04, '1', 0x00, // Start of array
                0x11, 0x0, 0x0, 0x0, // size of array
                0x08, '0', 0x00, 0x00,// false
                0x08, '1', 0x00, 0x01,// true
                0x08, '2', 0x00, 0x00,// false
                0x00, // End of Array
        0x00 // End of document
};
static const unsigned char  BsonObject[] = {
        0x25, 0x0, 0x0, 0x0, // Size of data.
            0x03, '1', 0x00, // Start of obj
                0x1d, 0x0, 0x0, 0x0, // size of obj
                0x08, 'T', 'e', 's', 't', '1', 0x00, 0x00,// false
                0x08, 'T', 'e', 's', 't', '2', 0x00, 0x01,// true
                0x08, 'T', 'e', 's', 't', '3', 0x00, 0x00,// false
            0x00, // End of obj
        0x00 // End of document
};
static const unsigned char BsonBinary[] = {
        0x11, 0x0, 0x0, 0x0, // Size of data.
        0x05, '1', 0x00,
            0x04, 0x00, 0x00, 0x00, // Size of bin.
            0x02,
            0x01, 0x02, 0x03, 0x04, // value of data.
        0x00 // End of document
};

std::string printAsHex(const unsigned char* str, size_t size){
    std::string retVal;
    retVal.reserve(size*6);
    for (int i = 0; i < size; i++){
        char hex[8];
        sprintf(hex, "0x%02x, ", str[i]);
        retVal += hex;
    }
    return retVal;
}

std::string printAsHex(const std::string& str){
    return printAsHex((const unsigned char*)str.c_str(), str.size());
}


#define DoTestAndCmp(Value, Type, assign) do{\
\
std::stringstream bson;\
JsonObject_t obj = std::make_shared<JsonObject>();\
obj->value["1"] = std::make_shared<Type>();\
assign;\
obj->write(&bson, EncodingOption(), StreamFormat::BSON);\
if (bson.str().size() != sizeof (Value) || memcmp(bson.str().c_str(), Value, sizeof(Value)) != 0){\
	retVal += "\nInvalid bson size for " #Value ", expected " + std::to_string(sizeof (Value)) +", got " + std::to_string(bson.str().size());\
	retVal += "\nInvalid bson for " #Value ", expected : \n" + printAsHex(Value, sizeof (Value)) +"\ngot :\n" + printAsHex(bson.str());\
}\
} while(0)

std::string testTxtToBson(){
    std::string retVal;

    DoTestAndCmp(BsonNull,   JsonNull,   );
    DoTestAndCmp(BsonFalse,  JsonBool,   obj->value["1"]->toBool()->value = false);
    DoTestAndCmp(BsonTrue,   JsonBool,   obj->value["1"]->toBool()->value = true);
    DoTestAndCmp(BsonIntM1,  JsonInt,    obj->value["1"]->toInt()->value = -1);
    DoTestAndCmp(BsonInt,    JsonInt,    obj->value["1"]->toInt()->value = 0x0807060504030201);
    DoTestAndCmp(BsonDouble, JsonDouble, obj->value["1"]->toDouble()->value = 1.);
    DoTestAndCmp(BsonString, JsonString, obj->value["1"]->toString()->value = "TEST");

    DoTestAndCmp(BsonBinary, JsonBinary, ((JsonBinary*)obj->value["1"].get())->value = std::make_shared<Binary>(sizeof(uint32_t)); *((uint32_t*)((JsonBinary*)obj->value["1"].get())->value->data) = 0x04030201);

    DoTestAndCmp(BsonArr, JsonArray,
            obj->value["1"]->toArray()->value.push_back(std::make_shared<JsonBool>(false));
            obj->value["1"]->toArray()->value.push_back(std::make_shared<JsonBool>(true));
            obj->value["1"]->toArray()->value.push_back(std::make_shared<JsonBool>(false))
    );

    DoTestAndCmp(BsonObject, JsonObject,
            (obj->value["1"]->toObject())->value["Test1"] = std::make_shared<JsonBool>(false);
            (obj->value["1"]->toObject())->value["Test2"] = std::make_shared<JsonBool>(true);
            (obj->value["1"]->toObject())->value["Test3"] = std::make_shared<JsonBool>(false)
    );

    return retVal;
}

int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(testTxtToBson());
	return valid ? 0 : -1;
}
