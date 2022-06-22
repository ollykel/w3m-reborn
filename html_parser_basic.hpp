#ifndef __HTML_PARSER_BASIC_HPP__
#define __HTML_PARSER_BASIC_HPP__

#include "deps.hpp"
#include "html_parser.hpp"
#include "dom_tree.hpp"

// === class HtmlParserBasic ==============================================
//
// Basic implementation of the HtmlParser Abstract Class.
//
// ========================================================================
class   HtmlParserBasic : public HtmlParser
{
    public:
        // === public member function(s) ==================================
        DomTree     parse_html(std::istream& ins) const;
    private:
};// end class   HtmlParserBasic : public HtmlParser

#endif
