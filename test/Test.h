/*
 * Test.h
 *
 *  Created on: Jul 10, 2017
 *      Author: daniel
 */

#ifndef TEST_TEST_H_
#define TEST_TEST_H_

#include <cstdio>
#include <string>
#include "../src/json.h"

#define EXE_TEST(func) do { std::string str = func; if (!str.empty()){ valid = false; printf("Test " #func  " failed : %s\n", str.c_str());} } while (0)

using namespace elladan;
using namespace elladan::json;

#endif /* TEST_TEST_H_ */
