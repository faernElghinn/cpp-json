/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#pragma once

#include <stddef.h>
#include <sys/types.h>

namespace elladan { namespace json {

class Utf8 {
public:
    static int encode(int32_t codepoint, char *buffer, size_t *size);
    static size_t check_full(const char *buffer, size_t size, int32_t *codepoint);
    static size_t check_first(char byte);
    static const char * iterate(const char *buffer, size_t bufsize, int32_t *codepoint);
};


} }  // namespace elladan::json

