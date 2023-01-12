#include <cstddef>
#include <climits>
#include <curses.h>
#include <list>

#include "deps.hpp"
#include "viewer.hpp"

// === TODO: move to header file ==========================================
#define     CTRL(KEY)   ((KEY) & 0x1f)

// --- public constructors ----------------------------------------
Viewer::Viewer(const Config& cfg, Document *doc)
{
    m_cfg = cfg;
    m_doc = doc;
    if (m_doc)
    {
        m_bufLineIter = doc->buffer().begin();
        m_bufNodeIter = m_bufLineIter->begin();
        m_isSinglePage = (doc->buffer().size() < LINES);
    }

    if (m_pad)
    {
        redraw();
    }
}// end type constructor

Viewer::Viewer(const Viewer& other)
{
    copy_from(other);
}// end copy constructor

Viewer::~Viewer(void)
{
    destruct();
}// end destructor

// --- public accessors ---------------------------------------------------
auto    Viewer::curr_curs_line(void) const
    -> size_t
{
    return m_currCursLine;
}// end Viewer::curr_curs_line

auto    Viewer::curr_buf_line(void) const
    -> size_t
{
    return m_currLine;
}// end Viewer::curr_buf_line

auto    Viewer::buffer_size(void) const
    -> size_t
{
    return m_doc->buffer().size();
}// end Viewer::buffer_size

auto    Viewer::curr_form_input(void) const
    -> const Document::FormInput*
{
    auto&       bufLine         = m_doc->buffer().at(m_currCursLine);

    if ((m_bufNodeIter != bufLine.end()) and (m_bufNodeIter->input_ref()))
    {
        return &m_doc->form_inputs()[m_bufNodeIter->input_ref().index()];
    }

    return nullptr;
}// end Viewer::curr_form_input

auto    Viewer::curr_form(void) const
    -> const Document::Form*
{
    const Document::FormInput*  out     = curr_form_input();

    if (out)
    {
        return &out->form();
    }

    return nullptr;
}// end Viewer::curr_form

// === public mutators ============================================
auto    Viewer::operator=(const Viewer& other)
    -> Viewer&
{
    if (this != &other)
    {
        destruct();
        copy_from(other);
    }

    return *this;
}// end operator=

void    Viewer::destruct(void)
{
    if (m_pad)
    {
        delwin(m_pad);
        m_pad = nullptr;
    }
    if (m_statusWin)
    {
        delwin(m_statusWin);
        m_statusWin = nullptr;
    }
}// end destruct

void    Viewer::copy_from(const Viewer& other)
{
    m_cfg = other.m_cfg;
    m_doc = other.m_doc;
    m_currLine = other.m_currLine;
    m_currCursLine = other.m_currCursLine;
    m_currCol = other.m_currCol;

    if (m_doc)
    {
        m_bufLineIter = m_doc->buffer().begin();
        m_bufNodeIter = m_bufLineIter->begin();
        m_isSinglePage = (m_doc->buffer().size() < LINES);

        redraw();
    }
}// end copy_from

void    Viewer::refresh(bool retouch)
{
    size_t                  colDiff         = m_currCol;
    size_t                  nCols           = 0;
    size_t                  nodeSize        = 0;

    if (m_currCursLine < m_currLine)
    {
        m_currCursLine = m_currLine;
    }
    else if (m_currCursLine >= m_currLine + LINES)
    {
        m_currCursLine = m_currLine + LINES - 1;
    }

    auto& bufLine = m_doc->buffer().at(m_currCursLine);
    m_bufNodeIter = bufLine.begin();

    while (
        (m_bufNodeIter != bufLine.end())
        and
        (colDiff >= (nodeSize = m_bufNodeIter->text().size()))
    )
    {
        colDiff -= nodeSize;
        nCols += nodeSize;
        ++m_bufNodeIter;
    }// end while

    if (m_bufNodeIter == bufLine.end())
    {
        m_currCol = std::min(m_currCol, nCols);
    }

    if (retouch)
    {
        string      padding(COLS, ' ');

        for (int i = m_startLine; i < LINES; ++i)
        {
            mvwaddstr(stdscr, i, 0, padding.c_str());
        }// end for

        wrefresh(stdscr);
    }

    wmove(m_pad, m_currCursLine, m_currCol);
    prefresh(
        m_pad,                  // pad
        m_currLine, 0,          // pminrow, pmincol
        m_startLine, m_startCol,    // sminrow, smincol
        LINES - 1, COLS - 1     // smaxrow, smaxcol
    );
}// end refresh

void Viewer::redraw(void)
{
    if (m_pad)
    {
        delwin(m_pad);
    }

    m_pad = newpad(m_doc->buffer().size(), COLS);

    for (int i = 0; i < m_doc->buffer().size(); ++i)
    {
        const auto&     line        = m_doc->buffer().at(i);
        int             j           = 0;
        int             remCols     = COLS;

        for (const auto& node : line)
        {
            // choose attribs
            if (node.input_ref())
            {
                wattrset(m_pad, m_cfg.attribs.input.attr);
                wcolor_set(m_pad, COLOR_PAIR_INPUT, NULL);
            }
            else if (node.image_ref())
            {
                wattrset(m_pad, m_cfg.attribs.image.attr);
                wcolor_set(m_pad, COLOR_PAIR_IMAGE, NULL);
            }
            else if (node.link_ref())
            {
                // TODO: differentiate types of links
                wattrset(m_pad, m_cfg.attribs.link.attr);
                wcolor_set(m_pad, COLOR_PAIR_LINK, NULL);
            }
            else
            {
                // standard
                wattrset(m_pad, A_NORMAL);
                wcolor_set(m_pad, COLOR_PAIR_STANDARD, NULL);
            }
            mvwaddnwstr(m_pad, i, j, (const wchar_t*) node.text().c_str(), remCols);
            j += node.text().size();
            remCols -= node.text().size();
        }// end for node
    }// end for i

    wmove(m_pad, m_currCursLine, m_currCol);
    wcolor_set(m_pad, 0, NULL);
    wattrset(m_pad, A_NORMAL);
    prefresh(m_pad, m_currLine, 0, 0, 0, LINES - 1, COLS - 1);
    wnoutrefresh(stdscr);
}// end void redraw

auto    Viewer::goto_section(const string& id)
    -> bool
{
    Document::buffer_index_type     idx;

    if ((idx = m_doc->get_section_index(id)))
    {
        goto_point(idx.line, 0);
        refresh();
        return true;
    }

    return false;
}// end Viewer::goto_section

void    Viewer::goto_point(size_t line, size_t col)
{
    m_currLine = line;

    if (m_currLine >= m_doc->buffer().size() - LINES)
    {
        m_currLine = m_doc->buffer().size() - LINES;
    }

    m_currCol = col;

    if (m_currCol >= COLS)
    {
        m_currCol = COLS - 1;
    }

    refresh();
}// end goto_point

void    Viewer::line_down(size_t nLines)
{
    if (not nLines)
    {
        return;
    }

    if (LINES >= m_doc->buffer().size())
    {
        return;
    }

    m_currLine += nLines;

    if (m_currLine + LINES >= m_doc->buffer().size())
    {
        m_currLine = m_doc->buffer().size() - LINES;
    }

    refresh();
}// end line_down

void    Viewer::line_up(size_t nLines)
{
    if (not nLines)
    {
        return;
    }

    if (nLines > m_currLine)
    {
        nLines = m_currLine;
    }

    m_currLine -= nLines;

    refresh();
}// end line_up

void    Viewer::curs_down(size_t nLines)
{
    if (not nLines)
    {
        return;
    }

    m_currCursLine += nLines;

    if (m_currCursLine >= m_doc->buffer().size())
    {
        m_currCursLine = m_doc->buffer().size() - 1;
    }

    if (m_currCursLine >= m_currLine + LINES)
    {
        m_currLine = m_currCursLine - LINES + 1;
    }

    refresh();
}// end curs_down

void    Viewer::curs_up(size_t nLines)
{
    if (not nLines)
    {
        return;
    }

    if (nLines > m_currCursLine)
    {
        nLines = m_currCursLine;
    }

    m_currCursLine -= nLines;

    if (m_currCursLine < m_currLine)
    {
        m_currLine = m_currCursLine;
    }

    refresh();
}// end curs_up

void    Viewer::curs_left(size_t nCols)
{
    if (nCols > m_currCol)
    {
        m_currCol = 0;
    }
    else
    {
        m_currCol -= nCols;
    }

    refresh();
}// end curs_left

void    Viewer::curs_right(size_t nCols)
{
    m_currCol += nCols;

    if (m_currCol >= COLS)
    {
        m_currCol = COLS - 1;
    }

    refresh();
}// end curs_right

auto    Viewer::curr_url(void)
    -> const string&
{
    static const string     NULL_STR    = "";

    auto&       bufLine         = m_doc->buffer().at(m_currCursLine);

    if ((m_bufNodeIter != bufLine.end()) and (m_bufNodeIter->link_ref()))
    {
        return m_doc->links().at(m_bufNodeIter->link_ref()).get_url();
    }

    return NULL_STR;
}// end curr_url

auto    Viewer::curr_img(void)
    -> const string&
{
    static const string     NULL_STR    = "";

    auto&       bufLine         = m_doc->buffer().at(m_currCursLine);

    if ((m_bufNodeIter != bufLine.end()) and (m_bufNodeIter->image_ref()))
    {
        return m_doc->images().at(m_bufNodeIter->image_ref()).get_url();
    }

    return NULL_STR;
}// end curr_img

auto    Viewer::curr_form_input(void)
    -> Document::FormInput*
{
    auto&       bufLine         = m_doc->buffer().at(m_currCursLine);

    if ((m_bufNodeIter != bufLine.end()) and (m_bufNodeIter->input_ref()))
    {
        return &m_doc->form_inputs()[m_bufNodeIter->input_ref().index()];
    }

    return nullptr;
}// end Viewer::curr_form_input

auto    Viewer::curr_form(void)
    -> Document::Form*
{
    Document::FormInput*    out     = curr_form_input();

    if (out)
    {
        return &out->form();
    }

    return nullptr;
}// end Viewer::curr_form

void    Viewer::disp_status(const string& str)
{
    string  status  = str;

    if (status.size() < COLS)
    {
        status += string(COLS - str.size(), ' ');
    }

    refresh();
    m_statusWin = subwin(stdscr, 1, COLS, LINES - 1, 0);
    mvwaddnstr(m_statusWin, 0, 0, status.c_str(), COLS);
    wrefresh(m_statusWin);
}// end disp_status

void    Viewer::clear_status(void)
{
    if (m_statusWin)
    {
        return;
    }
    delwin(m_statusWin);
    wnoutrefresh(stdscr);
    m_statusWin = nullptr;
}// end clear_status

auto    Viewer::prompt_char(const string& str)
    -> char
{
    WINDOW      *promptWin  = subwin(stdscr, 1, COLS, LINES-1, 0);
    string      prompt      = str + string(COLS - str.size(), ' ');
    char        out;

    refresh();
    mvwaddnstr(promptWin, 0, 0, prompt.c_str(), COLS);
    wrefresh(promptWin);
    prefresh(m_pad, 0, 0, 0, 0, LINES, COLS);

    out = wgetch(promptWin);
    delwin(promptWin);
    refresh(true);
    wnoutrefresh(stdscr);

    return out;
}// end prompt_char

auto    Viewer::prompt_string(
    string& dest,
    const string& prompt
) -> bool
{
    using namespace std;

    WINDOW          *promptWin  = subwin(stdscr, 1, COLS, LINES-1, 0);
    size_t          inputLen    = COLS - prompt.size();
    string          inputPadding(inputLen, ' ');
    int             cursIdx;
    int             inputIdx;
    int             key;
    size_t          idx;
    string          out     = dest;
    bool            ret     = true;

    idx = out.size();
    cursIdx = prompt.size() + idx;
    inputIdx = out.size() >= inputLen ?
        out.size() - inputLen + 1 :
        0;
    refresh();
    mvwaddnstr(promptWin, 0, 0, prompt.c_str(), COLS);
    mvwaddnstr(
        promptWin,
        0,
        prompt.size(),
        string(inputLen, ' ').c_str(),
        inputLen
    );
    mvwaddstr(promptWin, 0, prompt.size(), out.c_str() + inputIdx);
    wrefresh(promptWin);
    wmove(promptWin, 0, cursIdx);

    while ((key = wgetch(promptWin)))
    {
        if (cursIdx > COLS - 1)
        {
            cursIdx = COLS - 1;
        }

        switch (key)
        {
            case KEY_ENTER:
            case '\n':
                goto end_while;
            case CTRL('a'):
                idx = 0;
                break;
            case CTRL('d'):
                if (idx < out.size())
                {
                    out.erase(idx, 1);
                }
                break;
            case CTRL('e'):
                idx = out.size();
                break;
            case CTRL('f'):
                if (idx < out.size())
                {
                    ++idx;
                }
                break;
            case KEY_BACKSPACE:
            case CTRL('h'):
                if (not out.empty())
                {
                    --idx;
                    out.erase(idx, 1);
                }
                break;
            case CTRL('t'):
                {
                    size_t  pos;
                    
                    if (not idx)
                    {
                        break;
                    }

                    pos = out.find_last_not_of(" \t", idx - 1);
                    if ((0 == pos) or (string::npos == pos))
                    {
                        pos = 0;
                    }
                    else
                    {
                        pos = out.find_last_of(" \t", pos);
                        if (string::npos == pos)
                        {
                            pos = 0;
                        }
                        else
                        {
                            ++pos;
                        }
                    }

                    idx = pos;
                }
                break;
            case CTRL('u'):
                if (idx)
                {
                    out.erase(0, idx);
                    idx = 0;
                }
                break;
            case CTRL('w'):
                {
                    size_t  pos;
                    
                    if (not idx)
                    {
                        break;
                    }

                    pos = out.find_last_not_of(" \t", idx - 1);
                    if ((0 == pos) or (string::npos == pos))
                    {
                        pos = 0;
                    }
                    else
                    {
                        pos = out.find_last_of(" \t", pos);
                        if (string::npos == pos)
                        {
                            pos = 0;
                        }
                        else
                        {
                            ++pos;
                        }
                    }

                    out.erase(pos, idx - pos);
                    idx = pos;
                }
                break;
            case CTRL('c'):
            case CTRL('g'):
                ret = false;
                goto finally;
            default:
                out.insert(idx, 1, key);
                ++idx;
                break;
        }// end switch

        inputIdx = out.size() >= inputLen ?
            out.size() - inputLen + 1 :
            0;

        cursIdx = prompt.size() + idx;

        if (cursIdx > COLS - 1)
        {
            cursIdx = COLS - 1;
        }

        mvwaddstr(promptWin, 0, prompt.size(), inputPadding.c_str());
        mvwaddstr(promptWin, 0, prompt.size(), out.c_str() + inputIdx);
        wrefresh(promptWin);
        wmove(promptWin, 0, cursIdx);
    }// end while

end_while:
    dest = out;
finally:
    delwin(promptWin);
    refresh(true);
    wnoutrefresh(stdscr);
    return ret;
}// end prompt_string
