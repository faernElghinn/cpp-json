/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
/*
 * This document was converted to C++ and reformatted by Daniel Giguere.
 * There was also some type renaming done, but
 */

#include "utf.h"

#include <cstdint>

namespace elladan {
namespace json {

int Utf8::encode(int32_t codepoint, char *buffer, size_t *size)
{
    if (codepoint < 0)
        return -1;
    else if (codepoint < 0x80) {
        buffer[0] = (char) codepoint;
        *size = 1;
    }
    else if (codepoint < 0x800) {
        buffer[0] = 0xC0 + ((codepoint & 0x7C0) >> 6);
        buffer[1] = 0x80 + ((codepoint & 0x03F));
        *size = 2;
    }
    else if (codepoint < 0x10000) {
        buffer[0] = 0xE0 + ((codepoint & 0xF000) >> 12);
        buffer[1] = 0x80 + ((codepoint & 0x0FC0) >> 6);
        buffer[2] = 0x80 + ((codepoint & 0x003F));
        *size = 3;
    }
    else if (codepoint <= 0x10FFFF) {
        buffer[0] = 0xF0 + ((codepoint & 0x1C0000) >> 18);
        buffer[1] = 0x80 + ((codepoint & 0x03F000) >> 12);
        buffer[2] = 0x80 + ((codepoint & 0x000FC0) >> 6);
        buffer[3] = 0x80 + ((codepoint & 0x00003F));
        *size = 4;
    }
    else
        return -1;

    return 0;
}

const char * Utf8::iterate(const char *buffer, size_t bufsize, int32_t *codepoint)
{
    size_t count;
    int32_t value;

    if (!bufsize)
        return buffer;

    count = check_first(buffer[0]);
    if (count <= 0)
        return nullptr;

    if (count == 1)
        value = (uint8_t) buffer[0];
    else if (count > bufsize || !check_full(buffer, count, &value))
        return nullptr;

    if (codepoint)
        *codepoint = value;

    return buffer + count;
}

size_t Utf8::check_first(char byte)
{
    uint8_t u = (uint8_t) byte;

    if (u < 0x80)
        return 1;

    // second, third or fourth byte of a multi-byte sequence, i.e. a "continuation byte"
    if (0x80 <= u && u <= 0xBF)
        return 0;

    // overlong encoding of an ASCII byte
    else if (u == 0xC0 || u == 0xC1)
        return 0;

    // 2-byte sequence
    else if (0xC2 <= u && u <= 0xDF)
        return 2;

    // 3-byte sequence
    else if (0xE0 <= u && u <= 0xEF)
        return 3;

    // 4-byte sequence
    else if (0xF0 <= u && u <= 0xF4)
        return 4;

    // Restricted (start of 4-, 5- or 6-byte sequence) or invalid UTF-8
    else // u >= 0xF5
        return 0;
}

size_t Utf8::check_full(const char *buffer, size_t size, int32_t *codepoint) {
    uint8_t * buf =  (uint8_t*) buffer;

    int32_t value = 0;
    uint8_t u = *buf;

    if (size == 2)
        value = u & 0x1F;
    else if (size == 3)
        value = u & 0xF;
    else if (size == 4)
        value = u & 0x7;
    else
        return 0;

    for (size_t i = 1; i < size; i++) {
        u = buf[i];

        if (u < 0x80 || u > 0xBF)
            // not a continuation byte
            return 0;

        value = (value << 6) + (u & 0x3F);
    }

    // not in Unicode range
    if (value > 0x10FFFF)
        return 0;

    // invalid code point (UTF-16 surrogate halves)
    else if (0xD800 <= value && value <= 0xDFFF)
        return 0;

    // overlong encoding
    else if (  (size == 2 && value < 0x80)
            || (size == 3 && value < 0x800)
            || (size == 4 && value < 0x10000))
        return 0;

    if (codepoint)
        *codepoint = value;

    return 1;
}

} } // namespace elladan::json
