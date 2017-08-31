/*
 * BJsonSerializer.h
 *
 *  Created on: Apr 17, 2017
 *      Author: daniel
 */

#pragma once

#include <stddef.h>
#include <bitset>
#include <iostream>
#include <string>
#include <map>
#include <regex>

#include "../json.h"

namespace elladan { namespace json {

class BOStream;
class BIStream;

class BsonSerializer
{
public:
    static void write(std::ostream* out, Json* data, EncodingOption flag);
    static Json_t read(std::istream* in, DecodingOption flag);
    static std::vector<Json_t> extract(std::istream* in, DecodingOption flag, const std::string& path);

protected:
    static char getBsonType(Json* ele);
    static void writeBson(BOStream& out, Json* data);
    static inline void writeDoc(BOStream& out, std::map<std::string, Json_t> eleList);

    static inline void readName(BIStream& in, std::string& name);
    static inline void readRaw(BIStream& in, char* data, size_t size);
    static Json_t readBson(BIStream& in, char type);

    static std::vector<Json_t> searchBson(BIStream& in, char type, int deepness, std::vector<std::string> parts);
    static void skipBson(BIStream& in, char type);

};



} } // namespace elladan::json

