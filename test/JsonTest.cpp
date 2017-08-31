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
        if (toString(val) != "none") retVal += std::string("Invalid JsonNone toString(), expected none got ") + toString(val) + "\n";
    } while (0);

    do {\
        Json_t val = std::make_shared<JsonNull>();\
        if (toString(val) != "null") retVal +=  std::string("Invalid JsonNull toString(), expected null got ") + toString(val) + "\n";
    } while (0);

#define TEST(Val, value, asStr)\
do {\
    Json_t val = toJson(value);\
    Json##Val##_t ele = std::dynamic_pointer_cast<Json##Val>(val);\
    if (!ele) retVal +=  std::string("Allocated wrong type, expected Json" #Val " got ") + toString(val->getType()) + "\n";\
    if (ele->as##Val != value) retVal +=  "Invalid value in Json" #Val "\n";\
    if (toString(val) != asStr) retVal +=  std::string("Invalid Json" #Val " toString(), expected " asStr " got ") + toString(val) + "\n";\
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
        if (toString(val) != "[]") retVal +=  std::string("Invalid JsonArray toString(), expected [] got ") + toString(val) + "\n";
    } while (0);

    do {\
        Json_t val = std::make_shared<JsonObject>();
        JsonObject_t ele = std::dynamic_pointer_cast<JsonObject>(val);
        if (!ele) retVal +=  "Allocated wrong type, expected JsonObject\n" ;
        if (toString(val) != "{}") retVal +=  std::string("Invalid JsonObject toString(), expected {} got ") + toString(val) + "\n";
    } while (0);

#define TEST(val, Type)\
try {\
    auto res = fromJson<Type>(toJson(val)); \
 if (res != val) \
    retVal +=  std::string("Invalid Json" #Type " to bool, got ") + toString(res) + "\n"; \
} catch (std::exception& e) {\
    retVal +=  std::string("Invalid fromJson with Json" #Type ", got exception\n");\
}
    TEST(true, bool);
    TEST(-1,   int64_t);
    TEST(3.14, double);
    TEST("test", std::string);
    UUID uid = UUID::generateUUID();
    TEST(uid, UUID);


    // FIXME: binary and new type.
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
    arr1->push_back(std::make_shared<JsonInt>(0));
    JsonArray_t arr2 = std::make_shared<JsonArray>();
    arr2->push_back(std::make_shared<JsonInt>(0));
    arr2->push_back(std::make_shared<JsonInt>(1));
    JsonArray_t arr3 = std::make_shared<JsonArray>();
    arr3->push_back(std::make_shared<JsonInt>(1));
    JsonObject_t obj0 = std::make_shared<JsonObject>();
    JsonObject_t obj1 = std::make_shared<JsonObject>();
    obj1->operator []("0") = (std::make_shared<JsonInt>(0));
    JsonObject_t obj2 = std::make_shared<JsonObject>();
    obj2->operator []("0") = (std::make_shared<JsonInt>(0));
    obj2->operator []("1") = (std::make_shared<JsonInt>(1));
    JsonObject_t obj3 = std::make_shared<JsonObject>();
    obj3->operator []("0") = (std::make_shared<JsonInt>(1));
    JsonBinary_t bin0 = std::make_shared<JsonBinary>();
    JsonBinary_t bin1 = std::make_shared<JsonBinary>(std::make_shared<Binary>(sizeof(int32_t))); *((int32_t*)bin1->asBinary->data) = 1;
    JsonBinary_t bin2 = std::make_shared<JsonBinary>(std::make_shared<Binary>(sizeof(int32_t))); *((int32_t*)bin2->asBinary->data) = 2;

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
            retVal+= toString(ite->getType());
            retVal+= " already present in stream";
            continue;
        }

        sset.insert(ite);
        if (sset.size() != total){
            retVal+= "\n Failed to insert element ";
            retVal+= toString(ite->getType());
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
            retVal+= toString(ite->getType());
            retVal+= " got element ";
            retVal+= toString(ite->getType());
            retVal+= " instead";
            continue;
        }
    }

    return retVal;
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

    std::vector<Json_t> result;


    result = Json::getChild(root, "child1/sub1/val7");
    if (result.size() != 1) {
        retVal += "\n Could not extract child1/sub1/val7, invalid nb of elements";
    }
    else if (result.front()->getType() != JSON_INTEGER) {
        retVal += "\n Could not extract child1/sub1/val7, invalid type";
    }
    else if (((JsonInt*)result.front().get())->asInt != 8) {
        retVal += "\n Could not extract child1/sub1/val7, invalid value";
    }

    result = Json::getChild(root, "child1/sub0");
    if (result.size() != 1) {
        retVal += "\n Could not extract child1/sub0, invalid nb of elements";
    }
    else if (result.front()->getType() != JSON_OBJECT) {
        retVal += "\n Could not extract child1/sub0, invalid type";
    }
    else if (((JsonInt*)((JsonObject*)result.front().get())->operator []("val2").get())->asInt != 3) {
        retVal += "\n Could not extract child1/sub0, invalid value";
    }

    result = Json::getChild(root, "child0/*/val1");
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

    result = Json::getChild(root, "*");
    if (result.size() != 2) {
        retVal += "\n Could not extract *, invalid nb of elements";
    }
    else if (result.front()->getType() != JSON_OBJECT) {
        retVal += "\n Could not extract *, invalid type";
    }

    result = Json::getChild(root, "**/val1");
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

    result = Json::getChild(root, "child1/**");
    if (result.size() != 2) {
        retVal += "\n Could not extract child1/**, invalid nb of elements";
    }
    else if (result.front()->getType() != JSON_OBJECT) {
        retVal += "\n Could not extract child1/**, invalid type";
    }
    else if (((JsonObject*)result.front().get())->size() != 3) {
        retVal += "\n Could not extract child1/**, invalid size of child";
    }

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
