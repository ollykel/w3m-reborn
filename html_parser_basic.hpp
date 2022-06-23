#ifndef __HTML_PARSER_BASIC_HPP__
#define __HTML_PARSER_BASIC_HPP__

#include <stack>

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
        DomTree         parse_html(std::istream& ins) const;
    private:
        // === private static function(s) =================================
        static void     push_node(
                            std::istream& ins,
                            std::stack<DomTree::node*>& nodeStack,
                            std::stack<string>& tagStack
                        );
        static std::pair<string,string>
                        read_tag_attribute(std::istream& ins);
        static string   read_text_token(std::istream& ins);
};// end class   HtmlParserBasic : public HtmlParser

#endif
