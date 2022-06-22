#include <stack>

#include "deps.hpp"
#include "html_parser_basic.hpp"
#include "dom_tree.hpp"

// === HtmlParserBasic::parse_html(std::istream& ins) const ===============
//
// Parses an html document read from istream <ins> into a DomTree.
//
// ========================================================================
DomTree     HtmlParserBasic::parse_html(std::istream& ins) const
{
    DomTree     output;

    // 
    if (!ins)
        return output;

    return output;
}// end HtmlParserBasic::parse_html(std::istream& ins) const
