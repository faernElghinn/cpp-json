/*
 * JsonSerializertest.cpp
 *
 *  Created on: Jun 13, 2017
 *      Author: daniel
 */

#include <elladan/Stringify.h>
#include <elladan/UUID.h>
#include <bitset>
#include <exception>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "Test.h"

static const std::string Expect = "{"
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
            "\t\"arr\" : [\n"
            "\t],\n"
            "\t\"bool\" : true,\n"
            "\t\"double\" : 3.141500,\n"
            "\t\"fill_arr\" : [\n"
                "\t\t0,\n"
                "\t\t1,\n"
                "\t\t2,\n"
                "\t\t3\n"
            "\t],\n"
            "\t\"int\" : 0,\n"
            "\t\"null\" : null,\n"
            "\t\"obj\" : {\n"
            "\t},\n"
            "\t\"string\" : \"testing how this work\"\n"
        "}";

std::string testJsonToTxt(){
    std::string retVal;

    std::string str;

    str = toString(std::make_shared<Json>());
    if (str != "none") retVal += "\nCould not stringify Json, expected none, got " + str + "";

    str = toString(std::make_shared<JsonNull>());
    if (str != "null") retVal += "\nCould not stringify JsonNull, expected null, got " + str + "";

    str = toString(std::make_shared<JsonBool>(false));
    if (str != "false") retVal += "\nCould not stringify JsonBool, expected false, got " + str + "";

    str = toString(std::make_shared<JsonBool>(true));
    if (str != "true") retVal += "\nCould not stringify JsonBool, expected true, got " + str + "";

    str = toString(std::make_shared<JsonInt>(10));
    if (str != "10") retVal += "\nCould not stringify JsonInt, expected 10, got " + str + "";

    str = toString(std::make_shared<JsonDouble>(3.1415));
    if (str != "3.141500") retVal += "\nCould not stringify JsonDouble, expected 3.141500, got " + str + "";

    str = toString(std::make_shared<JsonString>("Testing with \""));
    if (str != "\"Testing with \\\"\"") retVal += "\nCould not stringify JsonString, expected \"Testing with \\\"\", got " + str + "";

    Binary_t bin = std::make_shared<Binary>(sizeof(uint64_t));
    *((uint64_t*)bin->data) = (uint64_t) -1;
    str = toString(std::make_shared<JsonBinary>(bin));
    if (str != "\"FFFFFFFFFFFFFFFF\"") retVal += "\nCould not stringify JsonBinary, expected \"FFFFFFFFFFFFFFFF\", got " + str + "";

    JsonObject_t head = std::make_shared<JsonObject>();
    head->operator []("null") = std::make_shared<JsonNull>();
    head->operator []("bool") = toJson(true);
    head->operator []("int") = toJson(0);
    head->operator []("double") = toJson(3.1415);
    head->operator []("string") = toJson("testing how this work");
    head->operator []("arr") = std::make_shared<JsonArray>();
    head->operator []("obj") = std::make_shared<JsonObject>();

    JsonArray_t arr =  std::make_shared<JsonArray>();
    head->operator []("fill_arr") = arr;
    arr->push_back(toJson(0));
    arr->push_back(toJson(1));
    arr->push_back(toJson(2));
    arr->push_back(toJson(3));

    str = toString(head);
    if (str != Expect )
        retVal += "\nInvalid tree toString, expected \n    " + Expect + "\n got \n    " + str;

    std::stringstream ss;
    EncodingOption opt;
    opt.ident = EncodingOption::MAX_INDENT_AS_TAB;
    head->write(&ss, opt, StreamFormat::JSON);
    if (ss.str() != ExpectWS )
        retVal += "\nInvalid tree toString with ws, expected \n    " + ExpectWS + "\n got \n    " + ss.str();

    return retVal;
}

int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(testJsonToTxt());
	return valid ? 0 : -1;
}
