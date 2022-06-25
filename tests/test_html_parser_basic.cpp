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

    int                 ret     = EXIT_SUCCESS;
    HtmlParserBasic     parser;
    DomTree             dom;

    dom.reset_root("window");

    try
    {
        parser.parse_html(*dom.root(), cin);
    }
    catch (HtmlParser::except_invalid_token e)
    {
        cout << "ERROR: " << (string) e << endl;
        ret = EXIT_FAILURE;
    }
    catch (...)
    {
        cout << "ERROR: unrecognized exception" << endl;
        ret = EXIT_FAILURE;
    }

    cout << dom << endl;

    return ret;
}// end main(void)
