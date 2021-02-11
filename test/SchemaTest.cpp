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

#include "../src/Schema/Schema.h"
#include "Test.h"

#undef NULL

using std::to_string;
using namespace elladan::json;

std::string typeTest() {
    std::string retVal;
    
#define TEST_TYPE(Type, TypeJson) do{\
   Json val = Type;\
   Json valid = (Object) {{"type", TypeJson}};\
   auto ret = shemaValidate(val, valid);\
   if (!ret.empty()) retVal += "\n Type " TypeJson " not working " + ret;\
} while(0)

    TEST_TYPE(Null(), "null");
    TEST_TYPE(true, "boolean");
    TEST_TYPE(1, "integer");
    TEST_TYPE(1.0, "number");
    TEST_TYPE("sdaf", "string");
    TEST_TYPE(Array(), "array");
    TEST_TYPE(Object(), "object");
#undef TEST_TYPE

    { // Test type array..
       Json val = true;
       Json valid = (Object) {
          {"type", (Array){"string", "bool"}}
       };

       auto ret = shemaValidate(val, valid);
       if (!ret.empty())
          retVal += "\n Type array not working " + ret;

       val = Null();
       if (shemaValidateQuick(val, valid))
          retVal += "\n Type was not in type array!";
    }

    return retVal;
}

std::string genericTest() {
   std::string retVal;

   { // Test "Default"
      Json val;

      Json valid = (Object) {
         {"type", "object"},
         {"default", (Object){
            {"a", 1}
         }}
      };

      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Could not set default " + ret;

      if (!val.isOfType<Object>() || val.as<Object>().count("a") != 1)
         retVal += "\n Did not apply the default value";
   }

   { // Test "enum"
      Json val;
      Json valid = (Object) {
         {"enum", (Array){
               1, "someString"
         }}
      };

      val = 1;
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass enum validation " + ret;

      val = Null();
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass enum validation " + ret;

      val = "someString";
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass enum validation " + ret;

      val = 2;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed enum validation ";
   }

   { // Test "enum"
      Json val;
      Json valid = (Object) {
         {"const", 1}
      };

      val = 1;
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass const validation " + ret;

      val = 2;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed const validation ";
   }

   return retVal;
}

std::string nullTest() {
   std::string retVal;
   // nothing to test
   return retVal;
}
std::string boolTest() {
   std::string retVal;
   // nothing to test
   return retVal;
}
std::string intTest() {
   std::string retVal;

   { // Test "min/max"
      Json val;
      Json valid = (Object) {
         {"minimum", 1},
         {"maximum", 10}
      };

      val = 1;
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass min validation " + ret;

      val = 0;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed min validation ";

      val = 10;
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass max validation " + ret;

      val = 11;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed max validation ";
   }

   { // Test "exclusive min/max"
      Json val;
      Json valid = (Object) {
         {"exclusiveMinimum", 1},
         {"exclusiveMaximum", 10}
      };

      val = 2;
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass excMin validation " + ret;

      val = 1;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed excMin validation ";

      val = 9;
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass excMax validation " + ret;

      val = 10;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed excMax validation ";

      val = 11;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed excMax validation ";
   }


   { // Test "exclusive min/max"
      Json val;
      Json valid = (Object) {
         {"multipleOf", 2},
      };

      val = 0;
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass multipleOf validation " + ret;

      val = 10;
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass multipleOf validation " + ret;

      val = 1;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed multipleOf validation ";
   }

   return retVal;
}

std::string numberTest() {
   std::string retVal;

   { // Test "min/max"
      Json val;
      Json valid = (Object) {
         {"minimum", 1.},
         {"maximum", 10.4}
      };

      val = 1.;
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass min validation " + ret;

      val = 0.;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed min validation ";

      val = 10.4;
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass max validation " + ret;

      val = 10; // test with an int.
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass max validation " + ret;

      val = 11.;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed max validation ";
   }

   { // Test "exclusive min/max"
      Json val;
      Json valid = (Object) {
         {"exclusiveMinimum", 1.1},
         {"exclusiveMaximum", 10.4}
      };

      val = 2.;
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass excMin validation " + ret;

      val = 1.1;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed excMin validation ";

      val = 10; // int
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass excMax validation " + ret;

      val = 10.4;
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass excMax validation " + ret;

      val = 11;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed excMax validation ";
   }


   { // Test "exclusive min/max"
      Json val;
      Json valid = (Object) {
         {"multipleOf", 1.5},
      };

      val = 0.;
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass multipleOf validation " + ret;

      val = 3.;
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass multipleOf validation " + ret;

      val = 1; // int
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed multipleOf validation ";

      val = 1.;
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed multipleOf validation ";
   }

   return retVal;
}


std::string stringTest() {
   std::string retVal;

   { // Test "min/max"
      Json val;
      Json valid = (Object) {
         {"minLength", 1},
         {"maxLength", 10}
      };

      val = "a";
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass minLength validation " + ret;

      val = "";
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed minLength validation ";

      val = "abcdefghij";
      ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass maxLength validation " + ret;

      val = "abcdefghijklmn";
      if (shemaValidateQuick(val, valid))
         retVal += "\n Should not have passed maxLength validation ";
   }

   { // Test "pattern"
      Json val = "a";
      Json valid = (Object) {
         {"pattern", ".*"},
      };

      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass pattern validation " + ret;

      Json invalid = (Object) {
         {"pattern", "\\d"},
      };

      if (shemaValidateQuick(val, invalid))
         retVal += "\n Should not have passed pattern validation ";
   }

#define TEST_FORMAT(format, valid, invalid) do {\
   Json validator = (Object) {{"format", format}};\
   Json val = validator; \
   auto ret = shemaValidate(val, validator);\
   if (!ret.empty()) retVal += "\n Did not pass " format " validation " + ret;\
   val = invalid;\
   if (shemaValidateQuick(val, val))\
      retVal += "\n Should not have passed " format " validation ";\
} while(0)

   TEST_FORMAT("date-time", "2019-01-12T03:13.23.123Z", "435-324-2342");
   TEST_FORMAT("date", "2019-01-12", "2019-01-12T03:13.23.123Z");
   TEST_FORMAT("time", "12:23:59+005", "2019-01-12T03:13.23.123Z");
   TEST_FORMAT("email", "daniel.gig@gmail.com", "@dfsa_wd3");
   TEST_FORMAT("idn-email", "daniel.gig+Newsletter@gmail.com", "@dfsa_wd3");
   TEST_FORMAT("ipv4", "192.168.0.1", "192.168.0");
   TEST_FORMAT("ipv6", "2001:db8:0:0:0:0:2:1", "192.168.0.1");
   TEST_FORMAT("hostname", "jira.terra-magica.ca", "fds-dsfwe ");
   TEST_FORMAT("idn-hostname", "jira.terra-magica.ca", "fds-dsfwe ");
   TEST_FORMAT("regex", ".*", "(.*");
   TEST_FORMAT("json-pointer", "#/dfsds/0/asdf", "3/213");
   TEST_FORMAT("relative-json-pointer", "3/213", "~/32/32");
   TEST_FORMAT("uri-template", "weather/{state}#", "~/32/32");

   { // Test "format" - uri-template
      Json valid = (Object) {
         {"format", "uri-template"},
      };

      Json val = "weather/{state}#";
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass date validation " + ret;

      val = "+sdf+32-a3w";
      if (shemaValidateQuick(val, val))
         retVal += "\n Should not have passed date validation ";
   }

   { // Test "format" - iri
      Json valid = (Object) {
         {"format", "iri"},
      };

      Json val = "ftp://dfsa/dfsa.html?query=sdfe";
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass iri validation " + ret;

      val = "ftp:/3s/sdf";
      if (shemaValidateQuick(val, val))
         retVal += "\n Should not have passed iri validation ";
   }

   { // Test "format" - iri-reference
      Json valid = (Object) {
         {"format", "iri-reference"},
      };

      Json val = "dfsa.html?query=sdfe";
      auto ret = shemaValidate(val, valid);
      if (!ret.empty())
         retVal += "\n Did not pass iri-reference validation " + ret;

      val = "?query=sdfe";
      if (shemaValidateQuick(val, val))
         retVal += "\n Should not have passed iri-reference validation ";
   }

      {"iri-reference", validateIri<R, true>}

// FIXME:
//   auto param = node.find("format");

   return retVal;
}


int main(int argc, char **argv) {
	bool valid = true;
	EXE_TEST(typeTest());
	EXE_TEST(genericTest());
	EXE_TEST(nullTest());
	EXE_TEST(boolTest());
	EXE_TEST(intTest());
	EXE_TEST(numberTest());
	EXE_TEST(stringTest());
//	EXE_TEST(arrayTest());
//	EXE_TEST(objectTest());
//	EXE_TEST(oneTest());
//	EXE_TEST(anyTest());
//	EXE_TEST(allTest());
//	EXE_TEST(notTest());
//	EXE_TEST(complexTest());
	return valid ? 0 : -1;
}
