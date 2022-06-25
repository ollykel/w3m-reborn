#ifndef __HTML_PARSER_BASIC_HPP__
#define __HTML_PARSER_BASIC_HPP__

#include <stack>
#include <map>
#include <unordered_set>

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
        void    parse_html(DomTree::node& root, std::istream& ins) const;
    private:
        // === private member class(es) ===================================
        struct  tag;

        // === private static function(s) =================================
        static void     push_node(
                            std::istream& ins,
                            std::stack<DomTree::node*>& nodeStack,
                            std::stack<string>& tagStack
                        );
        static string   read_text_token(std::istream& ins);
        static bool     is_empty_tag(const string& tag);
};// end class   HtmlParserBasic : public HtmlParser

// === struct  HtmlParserBasic::tag =======================================
//
// ========================================================================
struct  HtmlParserBasic::tag
{
    // === public member class(es) ========================================
    enum class      Kind
    {
        initial     = 0,
        terminal,
        solo,
        comment,
        version
    };// end enum class Kind

    // === public member variable(s) ======================================
    Kind                        kind            = Kind::initial;
    string                      identifier      = "";
    std::map<string,string>     attributes;

    // === public static functions ========================================
    static auto     from_stream(std::istream& ins) -> tag;
    static void     ignore_comment(std::istream& ins);
    static void     ignore_version(std::istream& ins);
    static std::pair<string,string> read_attribute(std::istream& ins);
};// end struct HtmlParserBasic::tag

#endif
