/*
 * JsonValidator.h
 *
 *  Created on: May 23, 2019
 *      Author: daniel
 */

#ifndef SCHEMA_SCHEMA_H_
#define SCHEMA_SCHEMA_H_

#include <elladan/json/json.h>

namespace elladan {
namespace json {

std::string shemaValidate(Json& toValidate, const Json& rules);
bool shemaValidateQuick(Json& toValidate, const Json& rules);
std::string validateSchema(Json& toValidate, const Json& rules);

}} /* namespace elladan::json */

#endif /* SCHEMA_SCHEMA_H_ */
