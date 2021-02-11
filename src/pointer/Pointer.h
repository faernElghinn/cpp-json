/*
 * Pointer.h
 *
 *  Created on: Aug. 23, 2019
 *      Author: daniel
 */

#ifndef SRC_POINTER_POINTER_H_
#define SRC_POINTER_POINTER_H_

#include <optional>
#include <string>
#include <variant>

#include "../json.h"

namespace elladan {
namespace json {


std::optional<std::string> getPointer(const Json& head, const Json& current);
std::optional<std::string> getRelativePointer(const Json& head, const Json& current, const Json& target);
std::optional<std::string> relToAbsPointer(const std::string& ptrPath, const Json& head, const Json& current);

Json* get(const std::string& ptrPath, Json& head);
std::variant<Json*, std::string> get(const std::string& ptrPath, Json& head, Json& current);
const Json* get(const std::string& ptrPath, const Json& head);
std::variant<const Json*, std::string> get(const std::string& ptrPath, const Json& head, const Json& current);

void set(const std::string& ptrPath, Json& head, Json& value, bool appendOnly = true);
void set(const std::string& ptrPath, Json& head, Json& current, Json& value, bool appendOnly = true);

} /* namespace json */
} /* namespace elladan */

#endif /* SRC_POINTER_POINTER_H_ */
