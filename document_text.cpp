#include <cstdio>
#include <sstream>

#include "deps.hpp"
#include "utils.hpp"
#include "document.hpp"
#include "document_text.hpp"

// === public constructor(s) ======================================
DocumentText::DocumentText(const Document::Config& cfg)
    : Document(cfg)
{
    // do nothing
}// end DocumentText(void)

DocumentText::DocumentText(
    const Document::Config& cfg,
    std::istream& ins,
    const size_t cols
) : DocumentText(cfg)
{
    from_stream(ins, cols);
}// end DocumentText(std::istream& ins, const size_t cols)

DocumentText::DocumentText(
    const Document::Config& cfg,
    const string& text,
    const size_t cols
) : DocumentText(cfg)
{
    from_string(text, cols);
}// end DocumentText(const string& text, const size_t cols)

// === public mutator(s) ==========================================
void        DocumentText::from_stream(std::istream& ins, const size_t cols)
{
    std::getline(ins, m_data, static_cast<char>(EOF));
    redraw(cols);
}// end DocumentText::from_stream(std::istream& ins, const size_t cols)

void        DocumentText::from_string(const string& text, const size_t cols)
{
    m_data = text;
    redraw(cols);
}// end DocumentText::from_string(const string& text, const size_t cols)

// ------ override(s) ---------------------------------------------
void        DocumentText::redraw(size_t cols)
{
    using namespace std;

    istringstream       inBuf(m_data);
    wstring             currLine        = wstring();

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
                                currLine += utils::to_wstr(token.substr(0, nRemain));
                                token.erase(0, nRemain);
                            }
                            m_buffer.emplace_back();
                            m_buffer[m_buffer.size() - 1].emplace_back(currLine);
                            currLine.clear();
                        }
                        else
                        {
                            currLine += utils::to_wstr(token);
                            token.clear();
                        }
                    } while (not token.empty());
                }
        }// end switch (inBuf.peek())
    }// end while (inBuf)
}// end DocumentText::redraw(size_t cols)
