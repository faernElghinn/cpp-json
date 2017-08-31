/*
 * BJsonSerializer.cpp
 *
 *  Created on: Apr 17, 2017
 *      Author: daniel
 */

#include "JsonSerializer.h"

#include <elladan/Exception.h>
#include <elladan/Stringify.h>
#include <stddef.h>
#include <bitset>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../utf.h"

namespace elladan {
namespace json {

class SOStream {
public:
    std::ostream* _out;

    SOStream(std::ostream* out) :
            _out(out) {
    }

    SOStream& operator <<(std::string& str);
    SOStream& operator <<(const std::string& str);
    SOStream& write(void* data, size_t size);
};

SOStream& SOStream::operator <<(std::string& str) {
    _out->write(str.c_str(), str.size());
    return *this;
}
SOStream& SOStream::operator <<(const std::string& str) {
    _out->write(str.c_str(), str.size());
    return *this;
}
SOStream& SOStream::write(void* data, size_t size) {
    _out->write((char*) data, size);
    return *this;
}

std::string JsonSerializer::stringToJson(const std::string& txt, EncodingOption flag) {
    std::ostringstream out;

    const char *ite, *lim;
    int32_t codepoint;

    out << "\"";

    ite = txt.c_str();
    lim = ite + txt.length();

    // Look for first special character.
    while (ite < lim) {
        ite = Utf8::iterate(ite, lim - ite, &codepoint);
        if (!ite)
//        	throw Exception(std::string("Invalid utf8 code at position ") + ite-txt.c_str() + " in " + txt);
            break;

        // Handle \, /, ", and control codes
        if (
        // mandatory escape or control char
        codepoint == '\\' || codepoint == '"' || codepoint < 0x20 ||
        // escape slash
                (flag.test(EncodingFlags::EF_JSON_ESCAPE_SLASH) && codepoint == '/')) {
            switch (codepoint) {
                case '\\':  out << "\\\\";   break;
                case '\"':  out << "\\\"";   break;
                case '\b':  out << "\\b";    break;
                case '\f':  out << "\\f";    break;
                case '\n':  out << "\\n";    break;
                case '\r':  out << "\\r";    break;
                case '\t':  out << "\\t";    break;
                case '/':   out << "\\/";    break;
                default: {
                    // codepoint is in BMP
                    if (codepoint < 0x10000) {
                        char seq[7];
                        snprintf(seq, sizeof(seq), "\\u%04X", (unsigned int) codepoint);
                        out << seq;
                    }

                    // not in BMP -> construct a UTF-16 surrogate pair
                    else {
                        char seq[13];

                        int32_t first, last;

                        codepoint -= 0x10000;
                        first = 0xD800 | ((codepoint & 0xffc00) >> 10);
                        last = 0xDC00 | (codepoint & 0x003ff);

                        snprintf(seq, sizeof(seq), "\\u%04X\\u%04X", (unsigned int) first, (unsigned int) last);
                        out << seq;
                    }
                }
                    break;
            }
        }
        // non-ASCII
        else if (flag.test(EncodingFlags::EF_JSON_ENSURE_ASCII) && codepoint > 0x7F)
            throw Exception("Could not encode non UTF code " + toString(codepoint));
        else
            out.put(codepoint);
    }

    out << "\"";
    return out.str();
}

static inline void writeSpace(SOStream& out, EncodingOption flag, int depth) {
    int indent = flag.getIndent();

    switch (indent) {
        case 0: // No ident, pack;
            break;

        case EncodingOption::MAX_INDENT_AS_TAB: // tab it;
        {
            static const std::string tabSpace = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
            out << "\n";
            while (depth > 0) {
                int cur_n = (depth < sizeof(tabSpace) - 1) ? depth : (sizeof(tabSpace) - 1);
                out << tabSpace.substr(0, cur_n);
                depth -= cur_n;
            }
        }
            break;

        default: // add the required amount of space.
        {
            static const std::string whitespace = "                                ";
            out << "\n";
            indent *= depth;
            while (indent > 0) {
                int cur_n = indent < sizeof(whitespace) - 1 ? indent : sizeof(whitespace) - 1;
                out << whitespace.substr(0, cur_n);
                indent -= cur_n;
            }
        }
            break;
    }
}

void JsonSerializer::writeJson(SOStream& out, Json* ele, EncodingOption flag, int depth) {
    switch (ele->getType()) {
        case JsonType::JSON_NULL:
            out << "null";
            break;

        case JsonType::JSON_NONE:
            out << "none";
            break;

        case JsonType::JSON_BOOL:
            out << (((JsonBool*) ele)->asBool ? "true" : "false");
            break;

        case JsonType::JSON_INTEGER:
            out << std::to_string(((JsonInt*) ele)->asInt);
            break;

        case JsonType::JSON_DOUBLE: {
            char d[64];
            snprintf(d, 63, "%#f", ((JsonDouble*) ele)->asDouble);
            out << std::string(d);
        } break;

        case JsonType::JSON_STRING:
            out << stringToJson(((JsonString*) ele)->asString, flag);
            break;

        case JsonType::JSON_UUID:
            out << stringToJson(((JsonUUID*) ele)->asUUID.toString(), flag);
            break;

        case JsonType::JSON_BINARY:
            out << stringToJson(((JsonBinary*) ele)->asBinary->toHex(), flag);
            break;

        case JsonType::JSON_ARRAY: {
            bool first = true;
            out << "[";
            depth++;
            for (auto ite : *((JsonArray*) ele)) {
                if (!first)
                    out << ",";
                first = false;
                writeSpace(out, flag, depth);
                writeJson(out, ite.get(), flag, depth);
            }
            depth--;
            writeSpace(out, flag, depth);
            out << "]";
        } break;

        case JsonType::JSON_OBJECT: {
            bool first = true;
            out << "{";
            depth++;
            for (auto ite : *((JsonObject*) ele)) {
                if (!first)
                    out << ",";
                first = false;
                writeSpace(out, flag, depth);
                out << stringToJson(ite.first, flag);
                out << (flag.getIndent() == 0 ? ":" : " : ");
                writeJson(out, ite.second.get(), flag, depth);
            }
            depth--;
            writeSpace(out, flag, depth);
            out << "}";
        } break;

        default:
            throw Exception("Unsupported Json_t type : " + ele->getType());
    }
}

void JsonSerializer::write(std::ostream* out, Json* data, EncodingOption flag) {
    SOStream str(out);
    writeJson(str, data, flag, 0);
}

///////////////////////////////////

struct Pos {
    size_t line;
    size_t col;
    void throwErr(const std::string& err) {
        std::ostringstream str;
        str << err << " at line " << line << " column " << col;
        throw Exception(str.str());
    }
};

class SIStream {
public:
    class MngError {
    public:
        MngError(const std::string& err, bool skipWs, bool skipCom, SIStream& str) :
                _err(err), _str(str), _skip_ws(skipWs), _skip_comment(skipCom) {
        }

        int operator >>(char& cur) {
            do {
                if (!(_str >> cur)) {
                    if (!_err.empty())
                        _str.throwErr(_err);
                    return 0;
                }

                if (_skip_comment && _str.flags.test(DecodingFlags::DF_IGNORE_COMMENT) && cur == '/') {
                    char prev = cur;
                    _str >> cur;

                    // '/*' comment : skip until '*/' is found.
                    if (cur == '*') {
                        bool prev_is_star = false;
                        do {
                            _str >> cur;
                            if (prev_is_star && cur == '/')
                                break;
                            prev_is_star = cur == '*';
                        } while (1);
                        _str >> cur;
                    }

                    // '//' comment : skip remaining of the line.
                    else if (cur == '/') {
                        do {
                            _str >> cur;
                        } while (cur != '\n');
                        _str >> cur;
                    }

                    // Not a comment, keep last read character.
                    else {
                        _str.pushBack(cur);
                        cur = prev;
                    }
                }
            } while (_skip_ws && std::isspace(cur));
            return 1;
        }
        std::string _err;
        SIStream& _str;
        bool _skip_ws;
        bool _skip_comment;
    };

    DecodingOption flags;
    std::istream* iStr;
    Pos pos;

    SIStream(std::istream* in, DecodingOption flag) :
            iStr(in), flags(flag) {
        pos.line = 0;
        pos.col = -1;
        _last = '\0';
    }

    void throwErr(const std::string& err) {
        pos.throwErr(err);
    }

    MngError operator()(std::string err = "", bool skipWs = true, bool skipCom = true) {
        if (!err.empty())
            err = std::string("Unexpected EOF while ") + err;
        return MngError(err, skipWs, skipCom, *this);
    }

    inline void pushBack(char c) {
        _last = c;
    }

    int operator >>(char& c) {
        if (_last) {
            c = _last;
            _last = 0;
            return 1;
        }

        else {
            iStr->read(&c, 1);
            pos.col++;
            if (c == '\n') {
                pos.line++;
                pos.col = 0;
            }
            return iStr->gcount();
        }
    }

protected:
    friend class MngError;
    char _last;

};

std::string JsonSerializer::jsonToString(SIStream& in) {
    std::string retVal;
    char letter;

    do {
        in("Could not decode string : Unexpected end of file.", false) >> letter;
        switch (letter) {
            case '\\': {
                in("decoding escaped char") >> letter;

                switch (letter) {
                    case '"':    retVal += "\"";  break;
                    case '/':    retVal += "/";   break;
                    case '\\':   retVal += "\\";  break;
                    case 'b':    retVal += "\b";  break;
                    case 'f':    retVal += "\f";  break;
                    case 'n':    retVal += "\n";  break;
                    case 'r':    retVal += "\r";  break;
                    case 't':    retVal += "\t";  break;
                    case '0':
                        if (!in.flags.test(DecodingFlags::DF_ALLOW_NULL))
                            in.throwErr("Found null value in string");
                        retVal += "\0";
                        break;

                    case 'u': {
                        int sum = 0;
                        std::string val;
                        for (int i = 0; i < 4; i++) {
                            in("decoding UTF encoded char") >> letter;
                            val.push_back(letter);
                        }
                        sum = std::stoi(val, 0, 16);

                        if (sum >= 0xDC00)
                            in.throwErr("Invalid unicode escape");

                        if (sum >= 0xD800) {
                            val.clear();
                            for (int i = 0; i < 4; i++) {
                                in("decoding UTF encoded char") >> letter;
                                val.push_back(letter);
                            }
                            int sum2 = std::stoi(val, 0, 16);

                            if (sum2 < 0xDC00)
                                in.throwErr("Invalid unicode escape");

                            sum = ((sum - 0xD800) << 10) + (sum2 - 0xDC00) + 0x10000;
                        }

                        char unicode[9];
                        size_t length;
                        if (Utf8::encode(sum, unicode, &length))
                            in.throwErr("Could not process unicode");

                        unicode[length] = '\0';
                        retVal += unicode;
                    }
                        break;

                    default:
                        in.throwErr("Invalid escape");
                        break;
                }
            }
                break;

            case '"':
                // End of string.
                return retVal;

            case '\0':
                if (!in.flags.test(DecodingFlags::DF_ALLOW_NULL))
                    in.throwErr("Found null value in string");
                // Fall-through
            default:
                retVal.push_back(letter);
                break;
        }
    } while (1);
}

Json_t JsonSerializer::readJson(SIStream& in, char cur) {
    std::string str;
    str.reserve(32);

    // Object
    if (cur == '{') {
        JsonObject_t obj = std::make_shared<JsonObject>();

        in("looking for the end of the object") >> cur;
        while (cur != '}') {

            // Get the key.
            Pos startOFLine = in.pos;
            JsonString_t key = std::dynamic_pointer_cast<JsonString>(readJson(in, cur));
            if (!key)
                startOFLine.throwErr("Missing key");

            if (in.flags.test(DecodingFlags::DF_REJECT_DUPLICATE) && obj->count(key->asString))
                in.throwErr("Duplicate value");

            // Get ":"
            in("looking for key value delimiter \':\'") >> cur;
            if (cur == ':')
                in("looking for object value") >> cur;
            else if (!in.flags.test(DecodingFlags::DF_ALLOW_COMMA_ERR))
                in.throwErr("Expecting array key value delimiter \":\"");

            // Read the associated value.
            Json_t child = readJson(in, cur);
            if (!child)
                in.throwErr("File ended before getting the value");
            obj->operator [](key->asString) = child;

            // Check if there is are remaining values,
            in("looking for element delimiter \',\' or closing bracket \'}\'") >> cur;
            if (cur == ',')
                in("looking for object next object element") >> cur;
            else if (cur != '}' && !in.flags.test(DecodingFlags::DF_ALLOW_COMMA_ERR))
                in.throwErr("Expected an element delimiter \',\'");
        }

        return obj;
    }

    // Array
    if (cur == '[') {
        JsonArray_t obj = std::make_shared<JsonArray>();

        in("while looking for end of array") >> cur;
        while (cur != ']') {
            // Read the associated value.
            Json_t child = readJson(in, cur);
            if (!child)
                in.throwErr("File ended before getting the value");
            obj->push_back(child);

            // Check if there is are remaining values,
            in("looking for element delimiter \',\' or closing bracket \']\'") >> cur;
            if (cur == ',')
                in("looking for object next array element") >> cur;
            else if (cur != ']' && !(in.flags.test(DecodingFlags::DF_ALLOW_COMMA_ERR)))
                in.throwErr("Expected an element delimiter \',\'");
        }

        return obj;
    }

    // String
    if (cur == '"')
        return std::make_shared<JsonString>(jsonToString(in));

    // Something else?

    // Get the whole "word"
    std::string word;

    do {
        // Make sure if fit out permitted list.
        cur = tolower(cur);

        static const std::string validChar = "0123456789abcdefgyijklmnopqrstuvwxyz-.";
        if (validChar.find(cur) == std::string::npos)
            break;

        str.push_back(cur);
    } while ((in >> cur) > 0);

    in.pushBack(cur);

    if (str.empty())
        in.throwErr("Expected value, nothing found");

    if (str == "null" && (in.flags.test(DecodingFlags::DF_ALLOW_NULL)))
        return std::make_shared<JsonNull>();

    long int asInt;
    if (parseInt(str, asInt) == str.size())
        return std::make_shared<JsonInt>(asInt);

    double asDouble;
    if (parseDouble(str, asDouble) == str.size())
        return std::make_shared<JsonDouble>(asDouble);

    bool asBool;
    if (parseBool(str, asBool))
        return std::make_shared<JsonBool>(asBool);

    in.throwErr("Could not identity type of " + str);
}

Json_t JsonSerializer::read(std::istream* in_stream, DecodingOption flag) {
    SIStream in(in_stream, flag);

    char cur;
    if (!(in("") >> cur))
        return std::make_shared<Json>();
    return readJson(in, cur);
}

std::vector<Json_t> JsonSerializer::extract(std::istream* in_stream, DecodingOption flag, const std::string& path) {
    SIStream in(in_stream, flag);

    char cur;
    if (!(in("") >> cur))
        return std::vector<Json_t>();

    return Json::getChild(readJson(in, cur), path);
}

}
} // namespace elladan::json
