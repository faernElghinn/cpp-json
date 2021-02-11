/*
 * Format.cpp
 *
 *  Created on: Sep. 6, 2019
 *      Author: daniel
 */


#pragma once

#include <bits/stdint-intn.h>
#include <bits/types/struct_tm.h>
#include <elladan/Exception.h>
#include <elladan/Stringify.h>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <vector>

#include "../json.h"


namespace elladan {
namespace json {

struct RetVal {
   RetVal() = default;
   RetVal(const std::string& err)
   : error(err)
   {}

   std::string error;
   inline operator bool() const { return error.empty(); }
   inline operator std::string() const { return error; }
};


#define MK_MATCH_REGEX(NAME, REG, ERR)\
template<typename R>\
static R match##NAME(std::string& val) {\
   if (!std::regex_match(val, REG)) return RetVal(val + ERR);\
   return RetVal();\
}
const std::regex EMAIL_REGEX("([!#-'*+/-9=?A-Z^-~-]+(\\.[!#-'*+/-9=?A-Z^-~-]+)*|\"([]!#-[^-~ \\t]|(\\\\[\\t -~]))+\")@([!#-'*+/-9=?A-Z^-~-]+(\\.[!#-'*+/-9=?A-Z^-~-]+)*|\\[[\\t -Z^-~]*])");
MK_MATCH_REGEX(Email, EMAIL_REGEX, " must be an RFC 5322 compliant email");
const std::regex IDN_EMAIL_REGEX("(?:[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*|\"(?:[\\x01-\\x08\\x0b\\x0c\\x0e-\\x1f\\x21\\x23-\\x5b\\x5d-\\x7f]|\\\\[\\x01-\\x09\\x0b\\x0c\\x0e-\\x7f])*\")@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\\[(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?|[a-z0-9-]*[a-z0-9]:(?:[\\x01-\\x08\\x0b\\x0c\\x0e-\\x1f\\x21-\\x5a\\x53-\\x7f]|\\\\[\\x01-\\x09\\x0b\\x0c\\x0e-\\x7f])+)\\])");
MK_MATCH_REGEX(EmailIdn, IDN_EMAIL_REGEX, " must be an RFC 6531 compliant email");
const std::regex IPV4_REGEX("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$");
MK_MATCH_REGEX(IPv4, IPV4_REGEX, " must be an ipv4");
const std::regex IPV6_REGEX("(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))");
MK_MATCH_REGEX(IPv6, IPV6_REGEX, " must be an ipv6");
const std::regex HOSTNAME_REGEX("^(?=.{1,255}$)[0-9A-Za-z](?:(?:[0-9A-Za-z]|-){0,61}[0-9A-Za-z])?(?:\\.[0-9A-Za-z](?:(?:[0-9A-Za-z]|-){0,61}[0-9A-Za-z])?)*\\.?$");
MK_MATCH_REGEX(Hostname, HOSTNAME_REGEX, " must be an RFC 1034, section 3.1 compliant hostname");
MK_MATCH_REGEX(HostnameIdn, HOSTNAME_REGEX, " must be an RFC 5890, section 2.3.2.3 compliant idn-hostname"); // FIXME: using RFC 1034 regex.
#undef MK_MATCH_REGEX

#define MK_MATCH_TIME(NAME, FORM, ERR)\
template<typename R>\
static R match##NAME(std::string& val) {\
   std::istringstream ss(val);\
   std::tm tm;\
   ss >> std::get_time(&tm, FORM);\
   if (ss.fail()) return RetVal(val + ERR);\
   return RetVal();\
}
MK_MATCH_TIME(DateTime, "%Y-%m-%dT%T", " is not a valid date-time.");
MK_MATCH_TIME(Date, "%Y-%m-%dT", " is not a valid date.");
MK_MATCH_TIME(Time, "%T", " is not a valid time.");
#undef MK_MATCH_TIME


const std::regex AUTHORITY_REGEX("(?:[0-9A-Za-z][0-9A-Za-z\\-]{0,62})(?:.[0-9A-Za-z][0-9A-Za-z\\-]{0,62})*");

template<typename R>
R validateRegex(std::string& val) {
   try {
      std::regex reg(val);
      return RetVal();
   } catch (std::exception& e) {
      return RetVal( std::string("'") + val + "' is not a valid regex");
   }
}

template<typename R, bool canSkipScheme>
R validateIri(std::string& val) {

   // is an uri, supporting utf8 encoded value

   // Scheme
   //  URI = scheme:[//authority]path[?query][#fragment]
   //  authority = [userinfo@]host[:port]
   if (val.empty())
      return RetVal("Empty iri");

   if (val.length() > 255)
      return RetVal(std::string("'") + val + "' is too long. Iri as a max of 255 chars");

   // parse the scheme
   auto colon = val.find_first_of(":/");
   if (colon == val.npos || val.at(colon) == '/') {
      if (!canSkipScheme)
         return RetVal(std::string("'") + val + "' is missing a schem, which is required for iri");
   }
   else {
      std::string schema = val.substr(0, colon);
      auto notValid = toLower(schema).find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789+.-");
      if (notValid != val.npos)
         return RetVal(std::string("'") + val + "' has an invalid scheme");

      val = val.substr(colon + 1);
   }

   auto frag = val.find("#");
   if (frag != val.npos) {
      std::string fragment = val.substr(frag + 1);

      // Make sure there is only one fragment
      if (fragment.find_first_of(":#[]@") == val.npos)
         return RetVal(std::string("'") + val + "' has forbidden chars in his fragment (following the '#')");

      val.erase(frag);
   }

   auto par = val.find("?");
   if (par != val.npos) {
      std::string params = val.substr(par + 1);

      if (params.find_first_of(":/?#[]@") == val.npos)
         return RetVal(std::string("'") + val + "' has forbidden chars in his params (the '?...' parts)");

      auto del = val.find_first_of("!$&'()*+,;=");
      if (del != val.npos) {
         std::string sep;
         sep.push_back(val.at(del));
         std::vector<std::string> pars = tokenize(params, sep);
         for (auto& ite : pars)
            if (std::count(ite.begin(), ite.end(), '=') != 1)
               return RetVal(std::string("'") + val + "' params must match the key=val pattern");
      }

      val.erase(par);
   }

   // Optional authority
   if (val.substr(0, 2) == "//") {

      auto paramStart = val.find(2, '/');
      if (paramStart == val.npos)
         return RetVal(std::string("'") + val + "' need an authority if it start with '//'.");

      std::string authority = val.substr(2, paramStart - 3);

      auto userInfo = authority.find('@');
      if (userInfo != val.npos) {
         std::string userI = val.substr(0, userInfo);

         if (std::count(userI.begin(), userI.end(), ':') > 1)
            return RetVal(std::string("'") + val + "' has more than one password seperator.");

         if (userI.find_first_of("?#[]"))
            return RetVal(std::string("'") + val + "'  has invalid userinfo in iri");

         authority = authority.substr(userInfo + 1);
      }

      // Here we can have the following:
      // - ipv4 : 1.12.244.1
      // - ipv6 : [af12::3213::38b]
      // - host : www.ww.www
      // Followed by an optional :port

      // Is ipv6 address
      if (authority.at(0) == '[') {
         auto end = authority.find(']');
         if (end == std::string::npos)
            return RetVal(std::string("'") + val + "' has an invalid ipv6 address. It must be enclosed within bracket '[]'");

         if (!std::regex_match(authority.substr(1, end - 2), IPV6_REGEX))
            return RetVal(std::string("'") + val + "' has an invalid ipv6 address.");

         authority = authority.substr(end + 1);
      }
      else if (std::regex_search(val, IPV4_REGEX)) {
      }
      else if (std::regex_search(val, AUTHORITY_REGEX)) {
      }
      else
         return RetVal(std::string("'") + val + "' has no valid authority.");

      auto portP = authority.find_last_of(':');
      if (portP == authority.npos) {
         if (!elladan::isInt(authority.substr(portP + 1)))
            return RetVal(" has an invalid port number.");
      }
      val = val.substr(paramStart + 1);
   }

   // Everything that remain is a path.
   // make suer there is no forbiden chars
   if (val.find_first_of("[]#?") != val.npos)
      return RetVal(std::string("'") + val + "' has an invalid path.");

   return RetVal();
}
template<typename R>
R validateUriTemplate(std::string& val) {
   std::string orig = val;
   R err = validateIri<R, false>(val);
   if (!err) return err;

   // Make sure all bracket open and close within a single path.
   auto pos = orig.find_first_of("{}/");
   bool gotTemplate = false;
   while (pos != orig.npos) {
      char on = orig.at(pos);
      if (gotTemplate) {
         switch (pos) {
            case '{':
               return RetVal(std::string("'") + val + "' is opening a new template without closing the previous one");
            case '}':
               gotTemplate = false;
               break;
            case '/':
               return RetVal(std::string("'") + val + "' has new path before the ending '}'");
            default:
               throw elladan::Exception("Internal error, broke on an unexpected char.");
         }
      }
      else {
         switch (pos) {
            case '/':
               break;
            case '{':
               gotTemplate = true;
               break;
            case '}':
               return RetVal(std::string("'") + val + "' is opening a new template without closing the previous one");
            default:
               throw elladan::Exception("Internal error, broke on an unexpected char.");
         }
      }
      pos = orig.find_first_of("{}/", pos + 1);
   }
   if (gotTemplate)
      return RetVal(std::string("'") + val + "' has un-ended template. Missing an '}'");

   return RetVal();
}

// FIXME: this should go in the pointer source.
template<typename R>
R validateJsonPointer(std::string& val) {
   if (val.empty())
      return RetVal(std::string("'") + val + "' is an empty json pointer");

   if (val.at(0) != '/')
      return RetVal(std::string("'") + val + "' json pointer must start with an '/'");

   auto pos = val.find("~/");
   while (pos != val.npos) {
      char on = val.at(pos);
      if (pos + 1 >= val.length()) {
         if (on == '/')
            return RetVal(std::string("'") + val + "' json pointer cannot end with an '/'");
         else
            return RetVal(std::string("'") + val + "' json pointer cannot end with an '~'");
      }

      char next = val.at(pos + 1);
      if (on == '/') {
         if (next == '/')
            return RetVal(std::string("'") + val + "' json pointer cannot have '/' twice in a row");
      }
      else {
         if (next != '0' && next != '1')
            return RetVal(std::string("'") + val + "' json pointer invalid escaped char");
      }
      pos = val.find_first_of("{}/", pos + 1);
   }
   return RetVal();
}

template<typename R>
R validateRelJsonPtr(std::string& val) {
   if (val.empty())
      return RetVal(std::string("'") + val + "' is an empty relative json pointer");

   if (!std::isdigit(val.at(0)))
      return RetVal(std::string("'") + val + "' relative json pointer must start with an number");

   // Fake an absolute path
   val = '/' + val;
   return validateJsonPointer<R>(val);
}


}} // namespace elladan::json

