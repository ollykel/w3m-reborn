#include "../deps.hpp"
#include "../html_parser_basic.hpp"
#include "../dom_tree.hpp"

// === test_html_parser_basic =============================================
//
// Reads an html document from stdin, parses it into a DomTree using
// HtmlParserBasic::parse_html, then displays the resulting DOM tree.
//
// This program should be considered a single unit test, to be called from
// a testing script.
//
// ========================================================================
int main(void)
{
    using namespace std;

    HtmlParserBasic     parser;
    DomTree             dom;

    try
    {
        dom = parser.parse_html(cin);
        cout << dom << endl;
    }
    catch (HtmlParser::except_invalid_token e)
    {
        cout << "ERROR: " << (string) e << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cout << "ERROR: unrecognized exception" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}// end main(void)
