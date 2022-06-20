#ifndef __HTML_PARSER_HPP__
#define __HTML_PARSER_HPP__

#include "deps.hpp"

// === class HtmlParser ===================================================
//
// Abstract class declaration for an html parser.
// An html parses implementation should take input from a std::istream
// object and return a tree of DOM nodes.
//
// ========================================================================
class   HtmlParser
{
    public:
        // === public member function(s) ==================================
        DOMTree     parse_html(std::istream& ins) const;
};// end class   HtmlParser

#endif
