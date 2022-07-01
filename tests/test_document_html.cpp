#include "../deps.hpp"
#include "../dom_tree.hpp"
#include "../document_html.hpp"

class   DocumentHtmlTester : public DocumentHtml
{
    public:
        // === public constructor(s) ======================================
        DocumentHtmlTester(std::istream& ins, const size_t cols)
            : DocumentHtml(ins, cols) {}
        // === public accessor(s) =========================================
        const DomTree&      dom(void) const
            { return m_dom; }
};// end class DocumentHtmlTester

// === main ===============================================================
//
// Reads document text from stdin, wrapping it by the number of columns
// indicated in argument 1 (or 80, if no arguments provided).
//
// ========================================================================
int main(const int argc, const char **argv)
{
    using namespace std;

    const size_t        numCols     = argc > 1 ? atoi(argv[1]) : 80;
    const size_t        numCols2    = numCols * 2;

    try
    {
        DocumentHtmlTester  doc(cin, numCols);

        cout << doc.dom() << endl;

        cout << "Printing document buffer..." << endl << "===" << endl;

        for (auto iter = doc.begin_lines(); iter != doc.end_lines(); ++iter)
        {
            for (auto& node : *iter)
            {
                cout << '{' << node.get_text() << '}';
            }// end for node
            cout << endl;
        }// end for iter

        cout << "Redrawing document with column width " << numCols2
            << "..." << endl;
        doc.redraw(numCols2);

        cout << "Printing document buffer again..." << endl << "==="
            << endl;

        for (auto iter = doc.begin_lines(); iter != doc.end_lines(); ++iter)
        {
            for (auto& node : *iter)
            {
                cout << '{' << node.get_text() << '}';
            }// end for node
            cout << endl;
        }// end for iter
    }
    catch (...)
    {
        cout << "Caught unrecognized exception" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}// end main
