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

// Test if we can instanciate our object.
std::string doConstructionTest(){
    std::string retVal;

    // Test creation
#define CREATE(name, Type) do{\
    Json_t name = std::make_shared<Type>();\
    if (name->getType() != JSON_ ##name) \
    retVal += "Fail to allocate " # Type " \n";\
    std::shared_ptr<Type> castAs ##name = std::dynamic_pointer_cast<Type>(name);\
    if (!castAs ##name) retVal += "->Fail to cast " # Type " \n";\
} while(0)
    CREATE(NONE,   Json);
    CREATE(NULL,   JsonNull);
    CREATE(BOOL,   JsonBool);
    CREATE(INTEGER,JsonInt);
    CREATE(DOUBLE, JsonDouble);
    CREATE(STRING, JsonString);
    CREATE(ARRAY,  JsonArray);
    CREATE(OBJECT, JsonObject);
//    CREATE(BINARY, JsonBinary);
#undef CREATE

    return retVal;
}

// Test if conversion to / from json works
std::string doAutoJsonTest(){
    std::string retVal;

    do {\
        Json_t val = std::make_shared<Json>();
        if (to_string(val) != "none") retVal += std::string("Invalid JsonNone to_string(), expected none got ") + to_string(val) + "\n";
    } while (0);

    do {\
        Json_t val = std::make_shared<JsonNull>();\
        if (to_string(val) != "null") retVal +=  std::string("Invalid JsonNull to_string(), expected null got ") + to_string(val) + "\n";
    } while (0);

#define TEST(Val, Value, asStr)\
do {\
    Json_t val = toJson(Value);\
    Json##Val##_t ele = std::dynamic_pointer_cast<Json##Val>(val);\
    if (!ele) retVal +=  std::string("Allocated wrong type, expected Json" #Val " got ") + to_string(val->getType()) + "\n";\
    if (ele->value != Value) retVal +=  "Invalid value in Json" #Val "\n";\
    if (to_string(val) != asStr) retVal +=  std::string("Invalid Json" #Val " to_string(), expected " asStr " got ") + to_string(val) + "\n";\
} while (0);
    TEST(Bool, true, "true");
    TEST(Int, 2, "2");
    TEST(Double, 2.7, "2.700000");
    TEST(String, "test", "\"test\"");
#undef TEST

    do {\
        Json_t val = std::make_shared<JsonArray>();
        JsonArray_t ele = std::dynamic_pointer_cast<JsonArray>(val);
        if (!ele) retVal +=  "Allocated wrong type, expected JsonArray\n";
        if (to_string(val) != "[]") retVal +=  std::string("Invalid JsonArray to_string(), expected [] got ") + to_string(val) + "\n";
    } while (0);

    do {\
        Json_t val = std::make_shared<JsonObject>();
        JsonObject_t ele = std::dynamic_pointer_cast<JsonObject>(val);
        if (!ele) retVal +=  "Allocated wrong type, expected JsonObject\n" ;
        if (to_string(val) != "{}") retVal +=  std::string("Invalid JsonObject to_string(), expected {} got ") + to_string(val) + "\n";
    } while (0);

#define TEST(val, Type)\
try {\
    auto res = fromJson<Type>(toJson(val)); \
 if (res != val) \
    retVal +=  std::string("Invalid Json" #Type " to bool, got ") + to_string(res) + "\n"; \
} catch (std::exception& e) {\
    retVal +=  std::string("Invalid fromJson with Json" #Type ", got exception\n");\
}
    TEST(true, bool);
    TEST(-1,   int64_t);
    TEST(3.14, double);
    TEST("test", std::string);
    UUID uid = UUID::generateUUID();
    TEST(uid, UUID);
    Binary_t bin = std::make_shared<Binary>("ABCDEFG");
    TEST(bin, Binary_t);
#undef TEST

    return retVal;
}



std::string doSortTest(){

    JsonNull_t nul = std::make_shared<JsonNull>();
    JsonBool_t bt = std::make_shared<JsonBool>(false);
    JsonBool_t bf = std::make_shared<JsonBool>(true);
    JsonInt_t i0 = std::make_shared<JsonInt>(0);
    JsonInt_t i1 = std::make_shared<JsonInt>(1);
    JsonDouble_t d0 = std::make_shared<JsonDouble>(0);
    JsonDouble_t d1 = std::make_shared<JsonDouble>(1);
    JsonString_t str0 = std::make_shared<JsonString>("0");
    JsonString_t str1 = std::make_shared<JsonString>("1");
    JsonArray_t arr0 = std::make_shared<JsonArray>();
    JsonArray_t arr1 = std::make_shared<JsonArray>();
    arr1->value.push_back(std::make_shared<JsonInt>(0));
    JsonArray_t arr2 = std::make_shared<JsonArray>();
    arr2->value.push_back(std::make_shared<JsonInt>(0));
    arr2->value.push_back(std::make_shared<JsonInt>(1));
    JsonArray_t arr3 = std::make_shared<JsonArray>();
    arr3->value.push_back(std::make_shared<JsonInt>(1));
    JsonObject_t obj0 = std::make_shared<JsonObject>();
    JsonObject_t obj1 = std::make_shared<JsonObject>();
    obj1->value["0"] = (std::make_shared<JsonInt>(0));
    JsonObject_t obj2 = std::make_shared<JsonObject>();
    obj2->value["0"] = (std::make_shared<JsonInt>(0));
    obj2->value["1"] = (std::make_shared<JsonInt>(1));
    JsonObject_t obj3 = std::make_shared<JsonObject>();
    obj3->value["0"] = (std::make_shared<JsonInt>(1));
    JsonBinary_t bin0 = std::make_shared<JsonBinary>();
    JsonBinary_t bin1 = std::make_shared<JsonBinary>(std::make_shared<Binary>(sizeof(int32_t))); *((int32_t*)bin1->value->data) = 1;
    JsonBinary_t bin2 = std::make_shared<JsonBinary>(std::make_shared<Binary>(sizeof(int32_t))); *((int32_t*)bin2->value->data) = 2;

    std::vector<Json_t> all = {
            nul, bf, bt, i0, i1, d0, d1, str0, str1, arr0, arr1, arr2, arr3, obj0, obj1, obj2, obj3, bin0, bin1, bin2
    };

    std::set<Json_t, cmpJson> sset;

    std::string retVal;
    auto total = 1;
    for (auto ite : all)
    {
        if (sset.count(ite) != 0) {
            retVal+= "\n Element mismatch, ";
            retVal+= to_string(ite->getType());
            retVal+= " already present in stream";
            continue;
        }

        sset.insert(ite);
        if (sset.size() != total){
            retVal+= "\n Failed to insert element ";
            retVal+= to_string(ite->getType());
            continue;
        }
        total++;
    }

    std::map<Json_t, JsonType, cmpJson> mmap;
    for (auto ite : all)
        mmap[ite] = ite->getType();

    for (auto ite : all) {
        if ( mmap[ite] != ite->getType()) {
            retVal+= "\n Failed to index element ";
            retVal+= to_string(ite->getType());
            retVal+= " got element ";
            retVal+= to_string(ite->getType());
            retVal+= " instead";
            continue;
        }
    }

    return retVal;
}

static std::string textExtract(const std::string& query, std::vector<Json_t> expected, Json_t root) {
    std::vector<Json_t> result = Json::getChild(root, query);
    if (result.size() != expected.size())
        return std::string("\n Could not extract ") + query + ", wrong number of item, expected " + to_string(expected.size()) + ", got " + to_string(result.size());

    for (int i = 0; i < result.size(); i++)
        if (result[i]->cmp(expected[i].get()) != 0)
            return std::string("\n Could not extract ") + query + ", element  " + to_string(i) + " differ.";

    return "";
}

std::string doSearchTest(){
    std::string retVal;

    JsonObject_t root = std::make_shared<JsonObject>();
    JsonObject_t child0 = std::make_shared<JsonObject>();
    JsonObject_t child0_1 = std::make_shared<JsonObject>();
    JsonObject_t child0_2 = std::make_shared<JsonObject>();
    JsonObject_t child1 = std::make_shared<JsonObject>();
    JsonObject_t child1_1 = std::make_shared<JsonObject>();
    JsonObject_t child1_2 = std::make_shared<JsonObject>();

    JsonObject *obj;

    obj = root.get();
    obj->value["child0"] = child0;
    obj->value["child1"] = child1;

    obj = child0.get();
    obj->value["sub0"] = child0_1;
    obj->value["sub1"] = child0_2;

    obj = child1.get();
    obj->value["sub0"] = child1_1;
    obj->value["sub1"] = child1_2;

    obj = child0_1.get();
    obj->value["val0"] = std::make_shared<JsonInt>(0);
    obj->value["val1"] = std::make_shared<JsonInt>(1);
    obj->value["val2"] = std::make_shared<JsonInt>(2);

    obj = child0_2.get();
    obj->value["val1"] = std::make_shared<JsonInt>(0);
    obj->value["val2"] = std::make_shared<JsonInt>(1);
    obj->value["val3"] = std::make_shared<JsonInt>(2);

    obj = child1_1.get();
    obj->value["val2"] = std::make_shared<JsonInt>(3);
    obj->value["val3"] = std::make_shared<JsonInt>(4);
    obj->value["val4"] = std::make_shared<JsonInt>(5);

    obj = child1_2.get();
    obj->value["val5"] = std::make_shared<JsonInt>(6);
    obj->value["val6"] = std::make_shared<JsonInt>(7);
    obj->value["val7"] = std::make_shared<JsonInt>(8);

    std::vector<Json_t> expectedResult;



    expectedResult.clear();
    expectedResult.push_back(child1_2->value["val7"]);
    textExtract("/child1/sub1/val7", expectedResult, root);

    expectedResult.clear();
    expectedResult.push_back(child1_1);
    textExtract("/child1/sub0",      expectedResult, root);

    expectedResult.clear();
    expectedResult.push_back(child0_1->value["val1"]);
    expectedResult.push_back(child0_2->value["val1"]);
    textExtract("/child0/*/val1",    expectedResult, root);

    expectedResult.clear();
    expectedResult.push_back(child0_1->value["val1"]);
    expectedResult.push_back(child0_2->value["val1"]);
    textExtract("/**/val1",    expectedResult, root);

    expectedResult.clear();
    expectedResult.push_back(child0);
    expectedResult.push_back(child1);
    textExtract("/*",    expectedResult, root);

    expectedResult.clear();
    expectedResult.push_back(child0);
    expectedResult.push_back(child1);
    textExtract("/child1/**",    expectedResult, root);


//    result = Json::getChild(root, "/child1/sub1/val7");
//    if (result.size() != 1) {
//        retVal += "\n Could not extract /child1/sub1/val7, invalid nb of elements";
//    }
//    else if (result.front()->getType() != JSON_INTEGER) {
//        retVal += "\n Could not extract /child1/sub1/val7, invalid type";
//    }
//    else if (((JsonInt*)result.front().get())->value != 8) {
//        retVal += "\n Could not extract /child1/sub1/val7, invalid value";
//    }
//
//    result = Json::getChild(root, "/child1/sub0");
//    if (result.size() != 1) {
//        retVal += "\n Could not extract /child1/sub0, invalid nb of elements";
//    }
//    else if (result.front()->getType() != JSON_OBJECT) {
//        retVal += "\n Could not extract /child1/sub0, invalid type";
//    }
//    else if (((JsonInt*)((JsonObject*)result.front().get())->value["val2"].get())->value != 3) {
//        retVal += "\n Could not extract /child1/sub0, invalid value";
//    }
//
//    result = Json::getChild(root, "/child0/*/val1");
//    if (result.size() != 2) {
//        retVal += "\n Could not extract child0/*/val1, invalid nb of elements";
//    }
//    else if (result.front()->getType() != JSON_INTEGER) {
//        retVal += "\n Could not extract child0/*/val1, invalid type";
//    }
//    else if (((JsonInt*)result.front().get())->value != 1) {
//        retVal += "\n Could not extract child0/*/val1, invalid value";
//    }
//    else if (((JsonInt*)result.back().get())->value != 0) {
//        retVal += "\n Could not extract child0/*/val1, invalid value";
//    }
//
//    result = Json::getChild(root, "*");
//    if (result.size() != 2) {
//        retVal += "\n Could not extract *, invalid nb of elements";
//    }
//    else if (result.front()->getType() != JSON_OBJECT) {
//        retVal += "\n Could not extract *, invalid type";
//    }
//
//    result = Json::getChild(root, "**/val1");
//    if (result.size() != 2) {
//        retVal += "\n Could not extract **/val1, invalid nb of elements";
//    }
//    else if (result.front()->getType() != JSON_INTEGER) {
//        retVal += "\n Could not extract **/val1, invalid type";
//    }
//    else if (((JsonInt*)result.front().get())->value != 1) {
//        retVal += "\n Could not extract **/val1, invalid value";
//    }
//    else if (((JsonInt*)result.back().get())->value != 0) {
//        retVal += "\n Could not extract **/val1, invalid value";
//    }

//    result = Json::getChild(root, "child1/**");
//    if (result.size() != 2) {
//        retVal += "\n Could not extract child1/**, invalid nb of elements";
//    }
//    else if (result.front()->getType() != JSON_OBJECT) {
//        retVal += "\n Could not extract child1/**, invalid type";
//    }
//    else if (((JsonObject*)result.front().get())->value.size() != 3) {
//        retVal += "\n Could not extract child1/**, invalid size of child";
//    }

    return retVal;
}

int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(doConstructionTest());
	EXE_TEST(doAutoJsonTest());
	EXE_TEST(doSortTest());
	EXE_TEST(doSearchTest());
	return valid ? 0 : -1;
}
