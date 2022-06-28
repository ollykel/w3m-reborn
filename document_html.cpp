#include <cstdio>
#include <sstream>

#include "deps.hpp"
#include "dom_tree.hpp"
#include "document_text.hpp"
#include "document_html.hpp"

// === public constructor(s) ======================================
DocumentHtml::DocumentHtml(void) : Document()
{
    // do nothing
}// end DocumentHtml(void)

DocumentHtml::DocumentHtml(std::istream& ins, const size_t cols) : Document()
{
    from_stream(ins, cols);
}// end DocumentHtml(std::istream& ins, const size_t cols)

DocumentHtml::DocumentHtml(const string& text, const size_t cols) : Document()
{
    from_string(text, cols);
}// end DocumentHtml(const string& text, const size_t cols)

// === public mutator(s) ==========================================
void        DocumentHtml::from_stream(std::istream& ins, const size_t cols)
{
    std::getline(ins, m_data, static_cast<char>(EOF));
    redraw(cols);
}// end DocumentHtml::from_stream(std::istream& ins, const size_t cols)

void        DocumentHtml::from_string(const string& text, const size_t cols)
{
    m_data = text;
    redraw(cols);
}// end DocumentHtml::from_string(const string& text, const size_t cols)

// ------ override(s) ---------------------------------------------
void        DocumentHtml::redraw(size_t cols)
{
    using namespace std;

    istringstream       inBuf(m_data);
    string              currLine        = "";

    m_buffer.clear();

    while (inBuf)
    {
        switch (inBuf.peek())
        {
            // space case: add to current line, if enough room
            case ' ':
                if (currLine.length() < cols)
                {
                    currLine += ' ';
                }
                inBuf.ignore(1);
                break;
            // TODO: custom tab width, from config
            case '\t':
                {
                    size_t  nSpaces = cols - currLine.length();

                    while (nSpaces--)
                    {
                        currLine += ' ';
                    }// end while (nSpaces--)
                    inBuf.ignore(1);
                }
                break;
            // ignore carriage returns
            case '\r':
                inBuf.ignore(1);
                break;
            // newline case
            case '\n':
                m_buffer.emplace_back();
                m_buffer[m_buffer.size() - 1].emplace_back(currLine);
                currLine.clear();
                inBuf.ignore(1);
                break;
            default:
                {
                    string  token   = utils::read_token_until(inBuf, " \t\r\n");

                    do
                    {
                        size_t nRemain  = cols - currLine.length();

                        if (token.length() > nRemain)
                        {
                            if (currLine.empty())
                            {
                                currLine += token.substr(0, nRemain);
                                token.erase(0, nRemain);
                            }
                            m_buffer.emplace_back();
                            m_buffer[m_buffer.size() - 1].emplace_back(currLine);
                            currLine.clear();
                        }
                        else
                        {
                            currLine += token;
                            token.clear();
                        }
                    } while (not token.empty());
                }
        }// end switch (inBuf.peek())
    }// end while (inBuf)
}// end DocumentHtml::redraw(size_t cols)
