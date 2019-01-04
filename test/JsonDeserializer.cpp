// /*
//  * JsonSerializertest.cpp
//  *
//  *  Created on: Jun 13, 2017
//  *      Author: daniel
//  */

// #include <elladan/Stringify.h>
// #include <elladan/UUID.h>
// #include <bitset>
// #include <exception>
// #include <memory>
// #include <sstream>
// #include <vector>

// #include "../src/json.h"
// #include "../src/serializer/JsonReader.h"
// #include "Test.h"

// using std::to_string;
// using namespace elladan::json::jsonSerializer;

// static const std::string ExpectWS =
//         "{\n"
//             "\t\"arr\" : [\n"
//             "\t],\n"
//             "\t\"obj\" : {\n"
//             "\t},\n"
//             "\t\"string\" : \"testing how this work\",\n"
//             "\t\"int\" : 0,\n"
//             "\t\"fill_arr\" : [\n"
//                 "\t\t0,\n"
//                 "\t\t1,\n"
//                 "\t\t2,\n"
//                 "\t\t3\n"
//             "\t],\n"
//             "\t\"bool\" : true,\n"
//             "\t\"double\" : 3.141500,\n"
//             "\t\"null\" : null\n"
//         "}";

// static std::string test_throw_exep(const std::string& txt){
//     std::string retVal;
//     std::stringstream ss;
//     ss << txt;

//     try{
//         Json obj = jsonSerializer::read(&ss);
//         retVal += "\nDecoding \"" + txt + "\" should have thrown an exception";
//     }  catch (std::exception& e) {    }
//     return retVal;
// }

// #define test_mode_value_noFormat(txt, type, opt)  \
//     try{ \
//         std::stringstream ss;\
//         ss << txt; \
//         Json obj = jsonSerializer::read(&ss, opt); \
//         if (!obj.isOfType<type>()) \
//             retVal += "\nCould not decode \"" txt "\" as " #type; \
//     } \
//     catch (std::exception& e) { \
//         retVal += std::string("\nCould not decode \"" txt "\" as " #type " ") + e.what(); \
//     }

// #define test_mode_value(txt, type, expected) \
//     try{ \
//         std::stringstream ss;\
//         ss << txt; \
//         Json obj = jsonSerializer::read(&ss); \
//         if (!obj.isOfType<type>()) \
//             retVal += "\nCould not decode \"" txt "\" as " #type; \
//         else if (obj.as<type>() != expected) { \
//             retVal += "\nInvalid value for \"" txt "\", expected "; \
//             retVal += to_string(expected) + " got " + to_string(obj.as<type>()); \
//         }\
//     }\
//     catch (std::exception& e) { \
//         retVal += "\nCould not decode \"" txt "\" as " #type; \
//     }

// std::string testTxtToJson(){
//     std::string retVal;

//     // test_throw_exep("null");
//     // test_mode_value_noFormat("null", Null, DecodingFlags::ALLOW_NULL);

//     test_mode_value ("true", bool, true)
//     test_mode_value ("false", bool, false)

//     // test_mode_value ("2", int64_t, 2)
//     // test_mode_value ("-100", int64_t, -100)
//     // test_mode_value ("07", int64_t, 7)
//     // test_throw_exep("100s");

//     // test_mode_value ("7.", double, 7.)
//     // test_mode_value ("0.07", double, 0.07)
//     // test_mode_value ("-200.34", double, -200.34)
//     // test_mode_value ("-200.34e-17", double, -200.34e-17)
//     // test_throw_exep("-20e0.34e-17");

//     // test_mode_value ("\"Some text with \\\"\"", std::string, "Some text with \"")


//     // Json obj;

//     // try{
//     //     std::stringstream ss;
//     //     ss << "[1,2,3]";
//     //     obj = jsonSerializer::read(&ss, DecodingOption());
//     //     if (obj->getType() != JSON_ARRAY)
//     //         retVal += "\nCould not decode \"[1,2,3]\" as JSON_ARRAY";
//     //     else if (std::dynamic_pointer_cast<JsonArray>(obj)->value.size() != 3)
//     //         retVal += "\nInvalid number of value in array for \"[1,2,3]\"";
//     // }
//     // catch (std::exception& e) {
//     //     retVal += "\nCould not decode \"[1,2,3]\" as JSON_ARRAY ";
//     //     retVal += e.what();
//     // }

//     // // Test white space
//     // try{
//     //     std::stringstream ss;
//     //     ss << " [  1,\t2 , 3 ]  \t";
//     //     obj = jsonSerializer::read(&ss, DecodingOption());
//     //     if (obj->getType() != JSON_ARRAY)
//     //         retVal += "\nCould not decode \" [  1,\t2 , 3 ]  \t\" as JSON_ARRAY";
//     //     else if (std::dynamic_pointer_cast<JsonArray>(obj)->value.size() != 3)
//     //         retVal += "\nInvalid number of value in array for \" [  1,\t2 , 3 ]  \t\"";
//     // }
//     // catch (std::exception& e) {
//     //     retVal += "\nCould not decode \" [  1,\t2 , 3 ]  \t\" as JSON_ARRAY ";
//     //     retVal += e.what();
//     // }

//     // // Test comma no error.
//     // try{
//     //     std::stringstream ss;
//     //     ss << "[1,2 3,]";
//     //     obj = jsonSerializer::read(&ss, DF_ALLOW_COMMA_ERR);
//     //     if (obj->getType() != JSON_ARRAY)
//     //         retVal += "\nCould not decode \"[1,2 3,]\" as JSON_ARRAY";
//     //     else if (std::dynamic_pointer_cast<JsonArray>(obj)->value.size() != 3)
//     //         retVal += "\nInvalid number of value in array for \"[1,2 3,]\"";
//     // }
//     // catch (std::exception& e) {
//     //     retVal += "\nCould not decode \"[1,2 3,]\" as JSON_ARRAY ";
//     //     retVal += e.what();
//     // }

//     // try{
//     //     std::stringstream ss;
//     //     ss << "{\"key1\":1,\"key2\":3.1415}";
//     //     obj = jsonSerializer::read(&ss, DecodingOption());
//     //     if (obj->getType() != JSON_OBJECT)
//     //         retVal += "\nCould not decode \"{\"key1\":1}\" as JSON_OBJECT";
//     //     else {
//     //         JsonObject_t tt = std::dynamic_pointer_cast<JsonObject>(obj);
//     //         if (tt->value.size() != 2)
//     //             retVal += "\nInvalid number of value in object for \"{\"key1\":1,\"key2\":3.1415}\"";
//     //         else if (tt->value.find("key1") == tt->value.end())
//     //             retVal += "\nCould not find key in \"{\"key1\":1,\"key2\":3.1415}\"";
//     //         else if (tt->value.find("key1")->second->getType() != JSON_INTEGER)
//     //             retVal += "\nInvalid 1st value type in \"{\"key1\":1,\"key2\":3.1415}\"";
//     //         else if (tt->value.find("key2") == tt->value.end())
//     //             retVal += "\nCould not find key in \"{\"key1\":1,\"key2\":3.1415}\"";
//     //         else if (tt->value.find("key2")->second->getType() != JSON_DOUBLE)
//     //             retVal += "\nInvalid 2nd value type in \"{\"key1\":1,\"key2\":3.1415}\"";
//     //     }
//     // }
//     // catch (std::exception& e) {
//     //     retVal += "\nCould not decode \"{\"key1\":1}\" as JSON_OBJECT ";
//     //     retVal += e.what();
//     // }

//     // try{
//     //     std::stringstream ss;
//     //     ss << "{  \"key1\"1 \"key2\" :\t3.1415,}";
//     //     obj = jsonSerializer::read(&ss, DF_ALLOW_COMMA_ERR);
//     //     if (obj->getType() != JSON_OBJECT)
//     //         retVal += "\nCould not decode \"{\"key1\":1}\" as JSON_OBJECT";
//     //     else {
//     //         JsonObject_t tt = std::dynamic_pointer_cast<JsonObject>(obj);
//     //         if (tt->value.size() != 2)
//     //             retVal += "\nInvalid number of value in object for \"{  \"key1\"1 \"key2\" :\t3.1415,}\"";
//     //         else if (tt->value.find("key1") == tt->value.end())
//     //             retVal += "\nCould not find key in \"{  \"key1\"1 \"key2\" :\t3.1415,}\"";
//     //         else if (tt->value.find("key1")->second->getType() != JSON_INTEGER)
//     //             retVal += "\nInvalid value type in \"{  \"key1\"1 \"key2\" :\t3.1415,}\"";
//     //         else if (tt->value.find("key2") == tt->value.end())
//     //             retVal += "\nCould not find key in \"{  \"key1\"1 \"key2\" :\t3.1415,}\"";
//     //         else if (tt->value.find("key2")->second->getType() != JSON_DOUBLE)
//     //             retVal += "\nInvalid value type in \"{  \"key1\"1 \"key2\" :\t3.1415,}\"";
//     //     }
//     // }
//     // catch (std::exception& e) {
//     //     retVal += "\nCould not decode \"{  \"key1\"1 \"key2\" :\t3.1415,}\" as JSON_OBJECT ";
//     //     retVal += e.what();
//     // }

//     // try{
//     //     std::stringstream ss;
//     //     ss << "{  \"key1\":1, \"key1\":3.1415}";
//     //     obj = jsonSerializer::read(&ss, DF_REJECT_DUPLICATE);
//     //     retVal += "\nShould have refused duplicate key";
//     // }
//     // catch (std::exception& e) {    }

//     // try{
//     //     std::stringstream ss;
//     //     ss << "/* Test de commentai\n"
//     //             "re\n"
//     //             "*/\n"
//     //             "{ \n "
//     //             "  // Besoin de stock\n"
//     //             "  \"key1\":1//Plus\n"
//     //             "  ,\"key2\":2//Plus\n"
//     //             ", \n"
//     //             "  // plus de commentaire\n"
//     //             "}// Et on fini avec desc commentaire";
//     //     obj = jsonSerializer::read(&ss, DF_IGNORE_COMMENT);
//     //     if (obj->getType() != JSON_OBJECT)
//     //         retVal += "\nCould not decode comment as JSON_OBJECT";
//     //     else {
//     //         JsonObject_t tt = std::dynamic_pointer_cast<JsonObject>(obj);
//     //         if (tt->value.size() != 1)
//     //             retVal += "\nInvalid number of value in object for comment";
//     //         else if (tt->value.find("key1") == tt->value.end())
//     //             retVal += "\nCould not find key in comment";
//     //         else if (tt->value.find("key1")->second->getType() != JSON_INTEGER)
//     //             retVal += "\nInvalid value type in comment";
//     //     }
//     // }
//     // catch (std::exception& e) {
//     //     retVal += "\nCould not decode comments as JSON_OBJECT ";
//     //     retVal += e.what();
//     // }

//     // // Touty Frutty.
//     // try{
//     //     std::stringstream ss;
//     //     ss << ExpectWS;
//     //     obj = jsonSerializer::read(&ss, DecodingFlags::DF_ALLOW_NULL);

//     //     if (obj->getType() != JSON_OBJECT)
//     //         retVal += "\nCould not decode \"" + ExpectWS + "\" as JSON_OBJECT";
//     //     else {
//     //         JsonObject* tt = std::dynamic_pointer_cast<JsonObject>(obj).get();
//     //         if (tt->value.size() != 8)
//     //             retVal += "\nInvalid number of element reading " + ExpectWS + "";
//     //     }
//     // }
//     // catch (std::exception& e) {
//     //     retVal += "\nCould not decode \""+ ExpectWS +"\" as JSON_OBJECT ";
//     //     retVal += e.what();
//     // }

//     // // Binary.
//     // try{
//     //     std::stringstream ss;
//     //     ss << "\"FFFFFFFF\"";
//     //     obj = jsonSerializer::read(&ss, DecodingOption());

//     //     if (obj->getType() != JSON_STRING)
//     //         retVal += "\nCould not decode \"FFFFFFFF\" as JSON_STRING";
//     //     else {
//     //         Binary_t bin = fromJson<Binary_t>(obj);
//     //         if (bin->size != 4)
//     //             retVal += "\nInvalid sine reading \"FFFFFFFF\"";
//     //         else if (*((uint32_t*)bin->data) != (uint32_t) -1)
//     //             retVal += "\nInvalid data reading \"FFFFFFFF\"";
//     //     }
//     // }
//     // catch (std::exception& e) {
//     //     retVal += "\nCould not decode \"FFFFFFFF\" as JSON_BINARY ";
//     //     retVal += e.what();
//     // }

//     return retVal;
// }

int main(int argc, char **argv) {
	bool valid = true;
// 	EXE_TEST(testTxtToJson());
	return valid ? 0 : -1;
}
