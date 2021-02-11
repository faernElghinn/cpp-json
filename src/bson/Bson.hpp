#pragma once

namespace elladan { namespace json { namespace bson {

constexpr char DOC_END = 0x00;
constexpr char ELE_TYPE_DOUBLE = 0x01;
constexpr char ELE_TYPE_UTF_STRING = 0x02;
constexpr char ELE_TYPE_OBJECT = 0x03;
constexpr char ELE_TYPE_ARRAY = 0x04;
constexpr char ELE_TYPE_BIN = 0x05;
//constexpr char ELE_TYPE_DEPRECATED = 0x06;
//constexpr char ELE_TYPE_OBJECT_ID = 0x07;
constexpr char ELE_TYPE_BOOL = 0x08;
//constexpr char ELE_TYPE_UTC_DATETIME= 0x09;
constexpr char ELE_TYPE_NULL = 0x0A;
//constexpr char ELE_TYPE_REGEX = 0x0B;
//constexpr char ELE_TYPE_DEPRECATED = 0x0C;
//constexpr char ELE_TYPE_JAVASCRIPT = 0x0D;
//constexpr char ELE_TYPE_DEPRECATED = 0x0E;
//constexpr char ELE_TYPE_JAVASCRIPT_SCOPED = 0x0F;
constexpr char ELE_TYPE_INT32 = 0x10;
//constexpr char ELE_TYPE_UINT64 = 0x11;
constexpr char ELE_TYPE_INT64 = 0x12;
//constexpr char ELE_TYPE_DECIMAL128 = 0x13;
//constexpr char ELE_TYPE_MIN = 0xFF;
//constexpr char ELE_TYPE_MAX = 0x7F;
constexpr char BINARY_SUBTYPE_GENERIC = 0x00;
constexpr char BINARY_SUBTYPE_BINARY_OLD = 0x02;
constexpr char BINARY_SUBTYPE_UUID_OLD = 0x03;
constexpr char BINARY_SUBTYPE_UUID = 0x04;
//constexpr char BINARY_SUBTYPE_MD5 = 0x05;

}}}
