#ifndef __URI_HPP__
#define __URI_HPP__

#include "deps.hpp"

struct  Uri
{
    // --- public member type(s) ------------------------------------------
    typedef     Uri     type;

    // --- public member variable(s) --------------------------------------
    string      scheme;
    string      userInfo;
    string      host;
    string      port;
    string      path;
    string      query;
    string      fragment;

    // --- public constructor(s) ------------------------------------------
    Uri(void);// default
    Uri(const string& str);// type

    // --- public accessor(s) ---------------------------------------------
    auto    str(void) const
        -> string;
    auto    empty(void) const
        -> bool;

    // --- public static function(s) --------------------------------------
    static auto from_relative(const type& base, const type& rel)
        -> type;
};// end struct Uri

std::ostream&   operator<<(std::ostream& outs, const Uri& uri);

#endif
