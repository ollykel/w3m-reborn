#ifndef __URI_HPP__
#define __URI_HPP__

#include "deps.hpp"

struct  Uri
{
    // === public member variable(s) ======================================
    string      scheme;
    string      userInfo;
    string      host;
    string      port;
    string      path;
    string      query;
    string      fragment;

    // === public constructor(s) ==========================================
    Uri(void);// default
    Uri(const string& str);// type

    // === public accessor(s) =============================================
    auto    str(void) const
        -> string;
};// end struct Uri

std::ostream&   operator<<(std::ostream& outs, const Uri& uri);

#endif
