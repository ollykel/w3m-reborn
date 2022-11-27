#include <climits>

#include "../deps.hpp"
#include "../html_parser.hpp"
#include "../dom_tree.hpp"
#include "../document_html.hpp"

class   DocumentHtmlTester : public DocumentHtml
{
    public:
        // === public constructor(s) ======================================
        DocumentHtmlTester(
            const Document::Config& cfg,
            std::istream& ins,
            const size_t cols
        ) : DocumentHtml(cfg, ins, cols) {}
        // === public accessor(s) =========================================
        const DomTree&      dom(void) const
            { return m_dom; }
};// end class DocumentHtmlTester

// === forward declarations ===============================================
void    print_doc(std::ostream& outs, const DocumentHtmlTester& doc);

// === main ===============================================================
//
// Reads document text from stdin, wrapping it by the number of columns
// indicated in argument 1 (or 80, if no arguments provided).
//
// ========================================================================
int main(const int argc, const char **argv)
{
    using namespace std;

    const size_t            numCols     = argc > 1 ? atoi(argv[1]) : 80;
    const size_t            numCols2    = numCols * 2;
    const Document::Config  cfg         = {
        // rows
        {
            40,         // def
            0,          // min
            SIZE_MAX,   // max
        },
    };// TODO: meaningful defaults

    try
    {
        DocumentHtmlTester  doc(cfg, cin, numCols);

        cout << doc.dom() << endl;

        print_doc(cout, doc);

        cout << "Redrawing document with column width " << numCols2
            << "..." << endl;
        doc.redraw(numCols2);

        print_doc(cout, doc);
    }
    catch (HtmlParser::except_invalid_token& e)
    {
        cout << "Caught exception:" << e << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}// end main

// === helper function implementations ====================================

void    print_doc(std::ostream& outs, const DocumentHtmlTester& doc)
{
    using namespace std;

    outs << "Title: " << doc.title() << endl;
    outs << "===" << endl;
    outs << "Printing document buffer..." << endl << "===" << endl;

    for (const auto& line : doc.buffer())
    {
        for (auto& node : line)
        {
            if (node.link_ref())
            {
                outs << "[" << node.text() << "](" 
                    << node.link_ref().index() << ')';
            }
            else
            {
                outs << node.text();
            }
        }// end for node
        outs << endl;
    }// end for iter

    outs << "===" << endl;
    outs << "Links:" << endl;
    outs << "===" << endl;

    size_t      count       = 0;
    for (const auto& link : doc.links())
    {
        outs << count++ << ". " << link.get_url() << endl;
    }// end for iter
}// end print_doc(std::ostream& outs, const DocumentHtmlTester& doc)
