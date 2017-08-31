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

static std::string test_throw_exep(const std::string& txt, DecodingOption opt = DecodingOption() ){
    std::string retVal;
    std::stringstream ss;
    ss << txt;

    try{
        Json_t obj = Json::read(&ss, opt, StreamFormat::JSON);
        retVal += "\nDecoding \"" + txt + "\" should have thrown an exception";
    }  catch (std::exception& e) {    }
    return retVal;
}

#define test_mode_value_noFormat(txt, type, opt) \
    try{ \
        std::stringstream ss;\
        ss << txt; \
        Json_t obj = Json::read(&ss, opt, StreamFormat::JSON); \
        if (obj->getType() != type) \
            retVal += "\nCould not decode \""  txt  "\" as " + toString(type) + ""; \
    } \
    catch (std::exception& e) { \
        retVal += "\nCould not decode \""  txt  "\" as " + toString(type) + " " + e.what(); \
    }

#define test_mode_value(txt, type, opt, Val, expected) \
    try{ \
        std::stringstream ss;\
        ss << txt; \
        Json_t obj = Json::read(&ss, opt, StreamFormat::JSON); \
        if (obj->getType() != type) \
            retVal += "\nCould not decode \"" txt "\" as " + toString(type) + ""; \
        else if (std::dynamic_pointer_cast<Json##Val>(obj)->as##Val != expected) { \
            retVal += "\nInvalid value for \"" txt "\", expected "; \
            retVal += toString(expected) + " got " + toString(std::dynamic_pointer_cast<Json##Val>(obj)->as##Val); \
        }\
    } \
    catch (std::exception& e) { \
        retVal += "\nCould not decode \"" txt "\" as " + toString(type); \
    }

std::string testTxtToJson(){
    std::string retVal;
    Json_t obj;

    test_throw_exep("none");
    test_throw_exep("null");

    test_mode_value_noFormat("null", JSON_NULL, DecodingFlags::DF_ALLOW_NULL);

    test_mode_value ("true", JSON_BOOL, DecodingOption(), Bool, true)
    test_mode_value ("false", JSON_BOOL, DecodingOption(), Bool, false)

    test_mode_value ("2", JSON_INTEGER, DecodingOption(), Int, 2)
    test_mode_value ("-100", JSON_INTEGER, DecodingOption(), Int, -100)
    test_mode_value ("0xff", JSON_INTEGER, DecodingOption(), Int, 255)
    test_mode_value ("07", JSON_INTEGER, DecodingOption(), Int, 7)
    test_throw_exep("100s");

    test_mode_value ("7.", JSON_DOUBLE, DecodingOption(), Double, 7.)
    test_mode_value ("0.07", JSON_DOUBLE, DecodingOption(), Double, 0.07)
    test_mode_value ("-200.34", JSON_DOUBLE, DecodingOption(), Double, -200.34)
    test_mode_value ("-200.34e-17", JSON_DOUBLE, DecodingOption(), Double, -200.34e-17)
    test_throw_exep("-20e0.34e-17");

    test_mode_value ("\"Some text with \\\"\"", JSON_STRING, DecodingOption(), String, "Some text with \"")

    try{
    std::stringstream ss;
        ss << "[1,2,3]";
        obj = Json::read(&ss, DecodingOption(), StreamFormat::JSON);
        if (obj->getType() != JSON_ARRAY)
            retVal += "\nCould not decode \"[1,2,3]\" as JSON_ARRAY";
        else if (std::dynamic_pointer_cast<JsonArray>(obj)->size() != 3)
            retVal += "\nInvalid number of value in array for \"[1,2,3]\"";
    }
    catch (std::exception& e) {
        retVal += "\nCould not decode \"[1,2,3]\" as JSON_ARRAY ";
        retVal += e.what();
    }

    // Test white space
    try{
        std::stringstream ss;
        ss << " [  1,\t2 , 3 ]  \t";
        obj = Json::read(&ss, DecodingOption(), StreamFormat::JSON);
        if (obj->getType() != JSON_ARRAY)
            retVal += "\nCould not decode \" [  1,\t2 , 3 ]  \t\" as JSON_ARRAY";
        else if (std::dynamic_pointer_cast<JsonArray>(obj)->size() != 3)
            retVal += "\nInvalid number of value in array for \" [  1,\t2 , 3 ]  \t\"";
    }
    catch (std::exception& e) {
        retVal += "\nCould not decode \" [  1,\t2 , 3 ]  \t\" as JSON_ARRAY ";
        retVal += e.what();
    }

    // Test comma no error.
    try{
        std::stringstream ss;
        ss << "[1,2 3,]";
        obj = Json::read(&ss, DF_ALLOW_COMMA_ERR, StreamFormat::JSON);
        if (obj->getType() != JSON_ARRAY)
            retVal += "\nCould not decode \"[1,2 3,]\" as JSON_ARRAY";
        else if (std::dynamic_pointer_cast<JsonArray>(obj)->size() != 3)
            retVal += "\nInvalid number of value in array for \"[1,2 3,]\"";
    }
    catch (std::exception& e) {
        retVal += "\nCould not decode \"[1,2 3,]\" as JSON_ARRAY ";
        retVal += e.what();
    }

    try{
        std::stringstream ss;
        ss << "{\"key1\":1,\"key2\":3.1415}";
        obj = Json::read(&ss, DecodingOption(), StreamFormat::JSON);
        if (obj->getType() != JSON_OBJECT)
            retVal += "\nCould not decode \"{\"key1\":1}\" as JSON_OBJECT";
        else {
            JsonObject_t tt = std::dynamic_pointer_cast<JsonObject>(obj);
            if (tt->size() != 2)
                retVal += "\nInvalid number of value in object for \"{\"key1\":1,\"key2\":3.1415}\"";
            else if (tt->find("key1") == tt->end())
                retVal += "\nCould not find key in \"{\"key1\":1,\"key2\":3.1415}\"";
            else if (tt->find("key1")->second->getType() != JSON_INTEGER)
                retVal += "\nInvalid 1st value type in \"{\"key1\":1,\"key2\":3.1415}\"";
            else if (tt->find("key2") == tt->end())
                retVal += "\nCould not find key in \"{\"key1\":1,\"key2\":3.1415}\"";
            else if (tt->find("key2")->second->getType() != JSON_DOUBLE)
                retVal += "\nInvalid 2nd value type in \"{\"key1\":1,\"key2\":3.1415}\"";
        }
    }
    catch (std::exception& e) {
        retVal += "\nCould not decode \"{\"key1\":1}\" as JSON_OBJECT ";
        retVal += e.what();
    }

    try{
        std::stringstream ss;
        ss << "{  \"key1\"1 \"key2\" :\t3.1415,}";
        obj = Json::read(&ss, DF_ALLOW_COMMA_ERR, StreamFormat::JSON);
        if (obj->getType() != JSON_OBJECT)
            retVal += "\nCould not decode \"{\"key1\":1}\" as JSON_OBJECT";
        else {
            JsonObject_t tt = std::dynamic_pointer_cast<JsonObject>(obj);
            if (tt->size() != 2)
                retVal += "\nInvalid number of value in object for \"{  \"key1\"1 \"key2\" :\t3.1415,}\"";
            else if (tt->find("key1") == tt->end())
                retVal += "\nCould not find key in \"{  \"key1\"1 \"key2\" :\t3.1415,}\"";
            else if (tt->find("key1")->second->getType() != JSON_INTEGER)
                retVal += "\nInvalid value type in \"{  \"key1\"1 \"key2\" :\t3.1415,}\"";
            else if (tt->find("key2") == tt->end())
                retVal += "\nCould not find key in \"{  \"key1\"1 \"key2\" :\t3.1415,}\"";
            else if (tt->find("key2")->second->getType() != JSON_DOUBLE)
                retVal += "\nInvalid value type in \"{  \"key1\"1 \"key2\" :\t3.1415,}\"";
        }
    }
    catch (std::exception& e) {
        retVal += "\nCould not decode \"{  \"key1\"1 \"key2\" :\t3.1415,}\" as JSON_OBJECT ";
        retVal += e.what();
    }

    try{
        std::stringstream ss;
        ss << "{  \"key1\":1, \"key1\":3.1415}";
        obj = Json::read(&ss, DF_REJECT_DUPLICATE, StreamFormat::JSON);
        retVal += "\nShould have refused duplicate key";
    }
    catch (std::exception& e) {    }

    try{
        std::stringstream ss;
        ss << "/* Test de commentai\n"
                "re\n"
                "*/\n"
                "{ \n "
                "  // Besoin de stock\n"
                "  \"key1\":1//Plus\n"
                "  ,\"key2\":2//Plus\n"
                ", \n"
                "  // plus de commentaire\n"
                "}// Et on fini avec desc commentaire";
        obj = Json::read(&ss, DF_IGNORE_COMMENT, StreamFormat::JSON);
        if (obj->getType() != JSON_OBJECT)
            retVal += "\nCould not decode comment as JSON_OBJECT";
        else {
            JsonObject_t tt = std::dynamic_pointer_cast<JsonObject>(obj);
            if (tt->size() != 1)
                retVal += "\nInvalid number of value in object for comment";
            else if (tt->find("key1") == tt->end())
                retVal += "\nCould not find key in comment";
            else if (tt->find("key1")->second->getType() != JSON_INTEGER)
                retVal += "\nInvalid value type in comment";
        }
    }
    catch (std::exception& e) {
        retVal += "\nCould not decode comments as JSON_OBJECT ";
        retVal += e.what();
    }

    // Touty Frutty.
    try{
        std::stringstream ss;
        ss << ExpectWS;
        obj = Json::read(&ss, DecodingFlags::DF_ALLOW_NULL, StreamFormat::JSON);

        if (obj->getType() != JSON_OBJECT)
            retVal += "\nCould not decode \"" + ExpectWS + "\" as JSON_OBJECT";
        else {
            JsonObject* tt = std::dynamic_pointer_cast<JsonObject>(obj).get();
            if (tt->size() != 8)
                retVal += "\nInvalid number of element reading " + ExpectWS + "";
        }
    }
    catch (std::exception& e) {
        retVal += "\nCould not decode \""+ ExpectWS +"\" as JSON_OBJECT ";
        retVal += e.what();
    }

    // Binary.
    try{
        std::stringstream ss;
        ss << "\"FFFFFFFF\"";
        obj = Json::read(&ss, DecodingOption(), StreamFormat::JSON);

        if (obj->getType() != JSON_STRING)
            retVal += "\nCould not decode \"FFFFFFFF\" as JSON_STRING";
        else {
            Binary_t bin = fromJson<Binary_t>(obj);
            if (bin->size != 4)
                retVal += "\nInvalid sine reading \"FFFFFFFF\"";
            else if (*((uint32_t*)bin->data) != (uint32_t) -1)
                retVal += "\nInvalid data reading \"FFFFFFFF\"";
        }
    }
    catch (std::exception& e) {
        retVal += "\nCould not decode \"FFFFFFFF\" as JSON_BINARY ";
        retVal += e.what();
    }

    return retVal;
}

int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(testTxtToJson());
	return valid ? 0 : -1;
}
