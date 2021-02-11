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

#include "../src/JsonWriter.h"
#include "Test.h"

using std::to_string;
using namespace elladan::json;
using namespace elladan::json::jsonSerializer;

static const std::string ExpectNotSorted = "{"
        "\"null\":null,"
        "\"bool\":true,"
        "\"int\":0,"
        "\"double\":3.141500000,"
        "\"string\":\"testing how this work\","
        "\"arr\":[],"
        "\"obj\":{},"
        "\"fill_arr\":[0,1,2,3]"
      "}";

static const std::string ExpectSorted = "{"
        "\"arr\":[],"
        "\"bool\":true,"
        "\"double\":3.141500000,"
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
            "\t\"double\" : 3.141500000,\n"
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

static const std::string UUID_RAW = "5d79e19a-317c-4d42-8299-08b5432db803";
static const std::string UUIDNoFormat = "\"" + UUID_RAW + "\"";
static const std::string UUIDWithFormat = "u" + UUIDNoFormat;


std::string toJsonString(const Json& json){
   std::stringstream ss;
   json::jsonSerializer::write(json, ss);
   return ss.str();
}

std::string testJsonToTxt(){
    std::string retVal;

    std::string str;

    str = toJsonString(Json());
    if (str != "null") retVal += "\nCould not stringify Json, expected null, got " + str + "";

    str = toJsonString(Json(Null()));
    if (str != "null") retVal += "\nCould not stringify JsonNull, expected null, got " + str + "";

    str = toJsonString(Json(false));
    if (str != "false") retVal += "\nCould not stringify JsonBool, expected false, got " + str + "";

    str = toJsonString(Json(true));
    if (str != "true") retVal += "\nCould not stringify JsonBool, expected true, got " + str + "";

    str = toJsonString(Json(10));
    if (str != "10") retVal += "\nCould not stringify JsonInt, expected 10, got " + str + "";

    str = toJsonString(Json(3.1415));
    if (str != "3.141500000") retVal += "\nCould not stringify JsonDouble, expected 3.141500000, got " + str + "";

    str = toJsonString(Json(9.34323324234e-10));
    if (str != "9.343233242e-10") retVal += "\nCould not stringify JsonDouble, expected 9.343233242e-10, got " + str + "";

    str = toJsonString(Json(3.0));
    if (str != "3.000000000") retVal += "\nCould not stringify JsonDouble, expected 3.000000000, got " + str + "";

    str = toJsonString(Json("Testing with \""));
    if (str != "\"Testing with \\\"\"") retVal += "\nCould not stringify JsonString, expected \"Testing with \\\"\", got " + str + "";

    {
        Json head = Object();
        head.as<Object>()["null"] = Null();
        head.as<Object>()["bool"] = true;
        head.as<Object>()["int"] = 0;
        head.as<Object>()["double"] = 3.1415;
        head.as<Object>()["string"] = "testing how this work";
        head.as<Object>()["arr"] = Array();
        head.as<Object>()["obj"] = Object();

        Json arr = Array();
        arr.as<Array>().push_back(0);
        arr.as<Array>().push_back(1);
        arr.as<Array>().push_back(2);
        arr.as<Array>().push_back(3);
        head.as<Object>()["fill_arr"] = arr;

        std::stringstream os;
        write(head, os, EncodingFlags::SORT_KEY);
        if (os.str() != ExpectSorted)
            retVal += "\nInvalid sorted tree toJsonString, expected \n    " + ExpectSorted + "\ngot:\n    " + os.str();

        str = toJsonString(head);
        if (str != ExpectNotSorted )
            retVal += "\nInvalid unsorted tree toJsonString, expected \n    " + ExpectNotSorted + "\ngot:\n    " + str;
        
        std::stringstream ss;
        EncodingOption opt;
        opt.setIndent(EncodingOption::MAX_INDENT_AS_TAB);
        write(head, ss, opt);
        if (ss.str() != ExpectWS )
            retVal += "\nInvalid tree toJsonString with ws, expected \n    " + ExpectWS + "\n got \n    " + ss.str();
    }

    {
        std::stringstream us;
        write(UUID::fromString(UUID_RAW), us, EXTENDED_TYPE);
        if (us.str() !=  UUIDWithFormat)
            retVal += "\nInvalid UUID extended toJsonString expected \n    " + UUIDWithFormat + "\n got \n    " + us.str();
    }

    {
        std::stringstream ns;
        write(UUID::fromString(UUID_RAW), ns);
        if (ns.str() !=  UUIDNoFormat)
            retVal += "\nInvalid UUID extended toJsonString expected \n    " + UUIDNoFormat + "\n got \n    " + ns.str();
    }
    
    {
        Binary bin = Binary(sizeof(uint64_t));
        *((uint64_t*)bin.data.data()) = (uint64_t) -1;
        str = toJsonString(Json(std::move(bin)));
        if (str != "\"FFFFFFFFFFFFFFFF\"") retVal += "\nCould not stringify JsonBinary, expected \"FFFFFFFFFFFFFFFF\", got " + str + "";
    }

    // FIXME: test extended type!
    


    return retVal;
}

int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(testJsonToTxt());
	return valid ? 0 : -1;
}
