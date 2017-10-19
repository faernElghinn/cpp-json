/*
 * BsonSerializertest.cpp
 *
 *  Created on: Jun 13, 2017
 *      Author: daniel
 */

#include <elladan/FlagSet.h>
#include <elladan/Stringify.h>
#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>

#include "Test.h"

using namespace elladan;
using namespace elladan::json;

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
        0x18, 0x0, 0x0, 0x0, // Size of data.
            0x04, '1', 0x00, // Start of array
                0x10, 0x0, 0x0, 0x0, // size of array
                0x08, '1', 0x00, 0x00,// false
                0x08, '2', 0x00, 0x01,// true
                0x08, '3', 0x00, 0x00,// false
            0x00, // End of Array
        0x00 // End of document
};
static const unsigned char  BsonObject[] = {
        0x20, 0x0, 0x0, 0x0, // Size of data.
            0x03, '1', 0x00, // Start of obj
                0x18, 0x0, 0x0, 0x0, // size of obj
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

#define TEST_SIMPLE_BT(src, Type, type, value)\
try{\
    std::stringstream ss;\
    ss.write((const char*)src, sizeof(src));\
    obj = std::dynamic_pointer_cast<JsonObject> (Json::read(&ss, DecodingOption(), StreamFormat::BSON));\
    if (!obj)\
        retVal += "\nCould not decode root object in " #src;\
    else if (obj->count("1") != 1)\
        retVal += "\nCould not decode index of first item in" #src;\
    else if (obj->operator []("1")->getType() != JSON_##Type )\
        retVal += "\nInvalid decoded type in " #src;\
    else if (((Json##type*)obj->operator []("1").get())->as##type != value)\
        retVal += "\nInvalid decoded value in " #src ", expected " + toString(value) + " got " + toString(((Json##type*)obj->operator []("1").get())->as##type);\
} catch (std::exception& e) {\
    retVal += std::string("\nError decoding ") + #src + " : " + e.what();\
}

std::string testBsonToTxt(){
    std::string retVal;

    JsonObject_t obj;

    try{
        std::stringstream ss;
        ss.write((const char*)BsonNull, sizeof(BsonNull));
        obj = std::dynamic_pointer_cast<JsonObject> (Json::read(&ss, DecodingOption(), StreamFormat::BSON));
        if (!obj)
            retVal += "\nCould not decode root object in " "BsonNull";
        else if (obj->count("1") != 1)
            retVal += "\nCould not decode index of first item in" "BsonNull";
    } catch (std::exception& e) {
        retVal += std::string("\nError decoding ") + "BsonNull" + " : " + e.what();
    }

    TEST_SIMPLE_BT(BsonFalse, BOOL, Bool, false)
    TEST_SIMPLE_BT(BsonTrue, BOOL, Bool, true)
    TEST_SIMPLE_BT(BsonInt, INTEGER, Int, 0x0807060504030201)
    TEST_SIMPLE_BT(BsonIntM1, INTEGER, Int, -1)
    TEST_SIMPLE_BT(BsonDouble, DOUBLE, Double, 1.)
    TEST_SIMPLE_BT(BsonString, STRING, String, "TEST")

    try{
        std::stringstream ss;
        ss.write((const char*)BsonArr, sizeof(BsonArr));
        obj = std::dynamic_pointer_cast<JsonObject> (Json::read(&ss, DecodingOption(), StreamFormat::BSON));
        if (!obj)
            retVal += "\nCould not decode root object in " "BsonArr";
        else if (obj->count("1") != 1)
            retVal += "\nCould not decode index of 1st item in " "BsonArr";
        else if (obj->operator []("1")->getType() != JSON_ARRAY)
            retVal += "\nCould not decode type of " "BsonArr";
        else {
            JsonArray_t arr = std::dynamic_pointer_cast<JsonArray>(obj->operator []("1"));
            if (arr->size() != 3)
                retVal += "\nWrong number of element in " "BsonArr";
            else if (((JsonBool*)arr->operator [](0).get())->asBool != false)
                retVal += "\nCould not decode value of 1st item in " "BsonArr";
            else if (((JsonBool*)arr->operator [](1).get())->asBool != true)
                retVal += "\nCould not decode value of 2nd item in " "BsonArr";
            else if (((JsonBool*)arr->operator [](2).get())->asBool != false)
                retVal += "\nCould not decode value of 3rd item in " "BsonArr";
        }
    } catch (std::exception& e) {
        retVal += std::string("\nError decoding ") + "BsonArr" + " : " + e.what();
    }

    try{
        std::stringstream ss;
        ss.write((const char*)BsonObject, sizeof(BsonObject));
        obj = std::dynamic_pointer_cast<JsonObject> (Json::read(&ss, DecodingOption(), StreamFormat::BSON));
        if (!obj)
            retVal += "\nCould not decode root object in " "BsonObject";
        else if (obj->count("1") != 1)
            retVal += "\nCould not decode index of 1st item in " "BsonObject";
        else if (obj->operator []("1")->getType() != JSON_OBJECT)
            retVal += "\nCould not decode type of " "BsonObject";
        else {
            obj = std::dynamic_pointer_cast<JsonObject>(obj->operator []("1"));
            if (obj->count("Test1") != 1)
                retVal += "\nCould not decode index of 2nd item in " "BsonObject";
            else if (obj->count("Test2") != 1)
                retVal += "\nCould not decode index of 2nd item in " "BsonObject";
            else if (obj->count("Test3") != 1)
                retVal += "\nCould not decode index of 3rd item in " "BsonObject";
            else if (((JsonBool*)obj->operator []("Test1").get())->asBool != false)
                retVal += "\nCould not decode value of 1st item in " "BsonObject";
            else if (((JsonBool*)obj->operator []("Test2").get())->asBool != true)
                retVal += "\nCould not decode value of 2nd item in " "BsonObject";
            else if (((JsonBool*)obj->operator []("Test3").get())->asBool != false)
                retVal += "\nCould not decode value of 3rd item in " "BsonObject";
        }
    } catch (std::exception& e) {
        retVal += std::string("\nError decoding ") + "BsonObject" + " : " + e.what();
    }

    try{
        std::stringstream ss;
        ss.write((const char*)BsonBinary, sizeof(BsonBinary));
        obj = std::dynamic_pointer_cast<JsonObject> (Json::read(&ss, DecodingOption(), StreamFormat::BSON));
        if (!obj)
            retVal += "\nCould not decode root object in " "BsonBinary";
        else if (obj->count("1") != 1)
            retVal += "\nCould not decode index of 1st item in " "BsonBinary";
        else if (obj->operator []("1")->getType() != JSON_BINARY)
            retVal += "\nCould not decode type of " "BsonBinary";
        else {
            JsonBinary_t bin = std::dynamic_pointer_cast<JsonBinary>(obj->operator []("1"));
            if (!bin->asBinary)
                retVal += "\nEmpty data in " "BsonBinary";
            else if (bin->asBinary->size != 4)
                retVal += "\nInvalid data size in " "BsonBinary";
            else if (*((uint32_t*)bin->asBinary->data) != (uint32_t) 0x04030201)
                retVal += "\nInvalid data in " "BsonBinary";
        }
    } catch (std::exception& e) {
        retVal += std::string("\nError decoding ") + "BsonBinary" + " : " + e.what();
    }

    return retVal;
}

std::string testBsonExtract(){
    std::string retVal;
    std::vector<Json_t> result;

    JsonObject_t root = std::make_shared<JsonObject>();
    JsonObject_t child0 = std::make_shared<JsonObject>();
    JsonObject_t child0_1 = std::make_shared<JsonObject>();
    JsonObject_t child0_2 = std::make_shared<JsonObject>();
    JsonObject_t child1 = std::make_shared<JsonObject>();
    JsonObject_t child1_1 = std::make_shared<JsonObject>();
    JsonObject_t child1_2 = std::make_shared<JsonObject>();

    JsonObject *obj;

    obj = root.get();
    obj->operator[]("child0") = child0;
    obj->operator[]("child1") = child1;

    obj = child0.get();
    obj->operator[]("sub0") = child0_1;
    obj->operator[]("sub1") = child0_2;

    obj = child1.get();
    obj->operator[]("sub0") = child1_1;
    obj->operator[]("sub1") = child1_2;

    obj = child0_1.get();
    obj->operator[]("val0") = std::make_shared<JsonInt>(0);
    obj->operator[]("val1") = std::make_shared<JsonInt>(1);
    obj->operator[]("val2") = std::make_shared<JsonInt>(2);

    obj = child0_2.get();
    obj->operator[]("val1") = std::make_shared<JsonInt>(0);
    obj->operator[]("val2") = std::make_shared<JsonInt>(1);
    obj->operator[]("val3") = std::make_shared<JsonInt>(2);

    obj = child1_1.get();
    obj->operator[]("val2") = std::make_shared<JsonInt>(3);
    obj->operator[]("val3") = std::make_shared<JsonInt>(4);
    obj->operator[]("val4") = std::make_shared<JsonInt>(5);

    obj = child1_2.get();
    obj->operator[]("val5") = std::make_shared<JsonInt>(6);
    obj->operator[]("val6") = std::make_shared<JsonInt>(7);
    obj->operator[]("val7") = std::make_shared<JsonInt>(8);


    std::stringstream str;
    root->write(&str, EncodingOption(), StreamFormat::BSON);

    str.seekg(std::istream::beg);
    Json_t cp = Json::read(&str, DecodingOption(), StreamFormat::BSON);
    if (cp->cmp(root.get()) != 0) {
        retVal += "\n Could not decode complex object";
        std::ofstream file ("object");
        file << str.str();
        file.close();
        return retVal;
    }

    str.seekg(std::istream::beg);
    result = Json::extract(&str, DecodingOption(), StreamFormat::BSON, "child1/sub1/val7");
    if (result.size() != 1) {
        retVal += "\n Could not extract child1/sub1/val7, invalid nb of elements";
    }
    else if (result.front()->getType() != JSON_INTEGER) {
        retVal += "\n Could not extract child1/sub1/val7, invalid type";
    }
    else if (((JsonInt*)result.front().get())->asInt != 8) {
        retVal += "\n Could not extract child1/sub1/val7, invalid value";
    }

    str.seekg(std::istream::beg);
    result = Json::extract(&str, DecodingOption(), StreamFormat::BSON, "child1/sub0");
    if (result.size() != 1) {
        retVal += "\n Could not extract child1/sub0, invalid nb of elements";
    }
    else if (result.front()->getType() != JSON_OBJECT) {
        retVal += "\n Could not extract child1/sub0, invalid type";
    }
    else if (((JsonInt*)((JsonObject*)result.front().get())->operator []("val2").get())->asInt != 3) {
        retVal += "\n Could not extract child1/sub0, invalid value";
    }

    str.seekg(std::istream::beg);
    result = Json::extract(&str, DecodingOption(), StreamFormat::BSON, "child0/*/val1");
    if (result.size() != 2) {
        retVal += "\n Could not extract child0/*/val1, invalid nb of elements";
    }
    else if (result.front()->getType() != JSON_INTEGER) {
        retVal += "\n Could not extract child0/*/val1, invalid type";
    }
    else if (((JsonInt*)result.front().get())->asInt != 1) {
        retVal += "\n Could not extract child0/*/val1, invalid value";
    }
    else if (((JsonInt*)result.back().get())->asInt != 0) {
        retVal += "\n Could not extract child0/*/val1, invalid value";
    }

    str.seekg(std::istream::beg);
    result = Json::extract(&str, DecodingOption(), StreamFormat::BSON, "*");
    if (result.size() != 2) {
        retVal += "\n Could not extract *, invalid nb of elements";
    }
    else if (result.front()->getType() != JSON_OBJECT) {
        retVal += "\n Could not extract *, invalid type";
    }

    str.seekg(std::istream::beg);
    result = Json::extract(&str, DecodingOption(), StreamFormat::BSON, "**/val1");
    if (result.size() != 2) {
        retVal += "\n Could not extract **/val1, invalid nb of elements";
    }
    else if (result.front()->getType() != JSON_INTEGER) {
        retVal += "\n Could not extract **/val1, invalid type";
    }
    else if (((JsonInt*)result.front().get())->asInt != 1) {
        retVal += "\n Could not extract **/val1, invalid value";
    }
    else if (((JsonInt*)result.back().get())->asInt != 0) {
        retVal += "\n Could not extract **/val1, invalid value";
    }

    str.seekg(std::istream::beg);
    result = Json::extract(&str, DecodingOption(), StreamFormat::BSON, "child1/**");
    if (result.size() != 2) {
        retVal += "\n Could not extract child1/**, invalid nb of elements";
    }
    else if (result.front()->getType() != JSON_OBJECT) {
        retVal += "\n Could not extract child1/**, invalid type";
    }
    else if (((JsonObject*)result.front().get())->size() != 3) {
        retVal += "\n Could not extract child1/**, invalid child size";
    }
}


int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(testBsonToTxt());
	EXE_TEST(testBsonExtract());
	return valid ? 0 : -1;
}
