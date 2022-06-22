#ifndef __HTML_PARSER_HPP__
#define __HTML_PARSER_HPP__

#include "deps.hpp"
#include "dom_tree.hpp"

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
        // === public member class(es) ====================================
        class   except_invalid_token;

        // === public member function(s) ==================================
        DomTree     parse_html(std::istream& ins) const;
};// end class   HtmlParser

class   HtmlParser::except_invalid_token : public StringException
{
    public:
        // === public constructor(s) ======================================
        except_invalid_token(const string& token);// type
};// end class HtmlParser::except_invalid_token

#endif
