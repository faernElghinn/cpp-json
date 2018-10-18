/*
 * JsonSerializer.h
 *
 *  Created on: Apr 28, 2017
 *      Author: daniel
 */

#pragma once

#include <iostream>
#include <string>

#include "../json.h"

namespace elladan { namespace json {

class SOStream;
class SIStream;

class JsonSerializer
{
public:
    static void write(std::ostream* out, const Json* data, EncodingOption flag);
    static Json_t read(std::istream* in, DecodingOption flag);
    static std::vector<Json_t> extract(std::istream* in, DecodingOption flag, const std::string& path);

protected:
    static Json_t readJson(SIStream& in, char cur);
    static void writeJson(SOStream& out, const Json* ele, EncodingOption flag, int depth);
    static std::string stringToJson(const std::string& txt, EncodingOption flag);
    static std::string jsonToString(SIStream& in);
};

} } // namespace elladan::json

