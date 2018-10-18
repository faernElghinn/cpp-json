/*
 * JsonSerializertest.cpp
 *
 *  Created on: Jun 13, 2017
 *      Author: daniel
 */

#include <elladan/Stringify.h>
#include <elladan/UUID.h>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../src/json.h"
#include "../src/serializer/JsonSerializer.h"
#include "Test.h"

using std::to_string;

static const std::string ExpectNotSorted = "{"
        "\"null\":null,"
        "\"bool\":true,"
        "\"int\":0,"
        "\"double\":3.141500,"
        "\"string\":\"testing how this work\","
        "\"arr\":[],"
        "\"obj\":{},"
        "\"fill_arr\":[0,1,2,3]"
      "}";

static const std::string ExpectSorted = "{"
        "\"arr\":[],"
        "\"bool\":true,"
        "\"double\":3.141500,"
        "\"fill_arr\":[0,1,2,3],"
        "\"int\":0,"
        "\"null\":null,"
        "\"obj\":{},"
        "\"string\":\"testing how this work\"}";

static const std::string ExpectWS =
        "{\n"
            "\t\"null\" : null,\n"
            "\t\"bool\" : true,\n"
            "\t\"int\" : 0,\n"
            "\t\"double\" : 3.141500,\n"
            "\t\"string\" : \"testing how this work\",\n"
            "\t\"arr\" : [\n"
            "\t],\n"
            "\t\"obj\" : {\n"
            "\t},\n"
            "\t\"fill_arr\" : [\n"
                "\t\t0,\n"
                "\t\t1,\n"
                "\t\t2,\n"
                "\t\t3\n"
            "\t]\n"
        "}";

std::string testJsonToTxt(){
    std::string retVal;

    std::string str;

    str = to_string(std::make_shared<Json>());
    if (str != "none") retVal += "\nCould not stringify Json, expected none, got " + str + "";

    str = to_string(std::make_shared<JsonNull>());
    if (str != "null") retVal += "\nCould not stringify JsonNull, expected null, got " + str + "";

    str = to_string(std::make_shared<JsonBool>(false));
    if (str != "false") retVal += "\nCould not stringify JsonBool, expected false, got " + str + "";

    str = to_string(std::make_shared<JsonBool>(true));
    if (str != "true") retVal += "\nCould not stringify JsonBool, expected true, got " + str + "";

    str = to_string(std::make_shared<JsonInt>(10));
    if (str != "10") retVal += "\nCould not stringify JsonInt, expected 10, got " + str + "";

    str = to_string(std::make_shared<JsonDouble>(3.1415));
    if (str != "3.141500") retVal += "\nCould not stringify JsonDouble, expected 3.141500, got " + str + "";

    str = to_string(std::make_shared<JsonString>("Testing with \""));
    if (str != "\"Testing with \\\"\"") retVal += "\nCould not stringify JsonString, expected \"Testing with \\\"\", got " + str + "";

    Binary_t bin = std::make_shared<Binary>(sizeof(uint64_t));
    *((uint64_t*)bin->data) = (uint64_t) -1;
    str = to_string(std::make_shared<JsonBinary>(bin));
    if (str != "\"FFFFFFFFFFFFFFFF\"") retVal += "\nCould not stringify JsonBinary, expected \"FFFFFFFFFFFFFFFF\", got " + str + "";

    JsonObject_t head = std::make_shared<JsonObject>();
    head->value["null"] = std::make_shared<JsonNull>();
    head->value["bool"] = toJson(true);
    head->value["int"] = toJson(0);
    head->value["double"] = toJson(3.1415);
    head->value["string"] = toJson("testing how this work");
    head->value["arr"] = std::make_shared<JsonArray>();
    head->value["obj"] = std::make_shared<JsonObject>();

    JsonArray_t arr =  std::make_shared<JsonArray>();
    head->value["fill_arr"] = arr;
    arr->value.push_back(toJson(0));
    arr->value.push_back(toJson(1));
    arr->value.push_back(toJson(2));
    arr->value.push_back(toJson(3));

    std::stringstream os;
    EncodingOption sorted(EncodingFlags::EF_JSON_SORT_KEY);
    head->write(&os, sorted, StreamFormat::JSON);
    if (os.str() != ExpectSorted)
        retVal += "\nInvalid sorted tree to_string, expected \n    " + ExpectSorted + "\ngot:\n    " + os.str();

    str = to_string(head);
    if (str != ExpectNotSorted )
        retVal += "\nInvalid unsorted tree to_string, expected \n    " + ExpectNotSorted + "\ngot:\n    " + str;

    std::stringstream ss;
    EncodingOption opt;
    opt.ident = EncodingOption::MAX_INDENT_AS_TAB;
    head->write(&ss, opt, StreamFormat::JSON);
    if (ss.str() != ExpectWS )
        retVal += "\nInvalid tree to_string with ws, expected \n    " + ExpectWS + "\n got \n    " + ss.str();

    return retVal;
}

int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(testJsonToTxt());
	return valid ? 0 : -1;
}
