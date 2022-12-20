#include <cstddef>
#include <climits>
#include <curses.h>

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
    size_t      currBufLine     = m_currLine + m_currCursLine;
    auto&       bufLine         = m_doc->buffer().at(currBufLine);
    size_t      colDiff         = m_currCol;
    size_t      nCols           = 0;
    size_t      nodeSize        = 0;

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

        for (int i = 0; i < LINES; ++i)
        {
            mvwaddstr(stdscr, i, 0, padding.c_str());
        }// end for

        wrefresh(stdscr);
    }

    wmove(m_pad, currBufLine, m_currCol);
    prefresh(m_pad, m_currLine, 0, 0, 0, LINES - 1, COLS - 1);
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

    wmove(m_pad, m_currLine + m_currCursLine, m_currCol);
    wcolor_set(m_pad, 0, NULL);
    wattrset(m_pad, A_NORMAL);
    prefresh(m_pad, m_currLine, 0, 0, 0, LINES - 1, COLS - 1);
    wnoutrefresh(stdscr);
}// end void redraw

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
    m_currLine += nLines;

    if (m_currLine >= m_doc->buffer().size() - LINES)
    {
        m_currLine = m_doc->buffer().size() - LINES;
    }

    refresh();
}// end line_down

void    Viewer::line_up(size_t nLines)
{
    if (m_isSinglePage or not m_currLine)
    {
        return;
    }

    if (m_currLine > nLines)
    {
        m_currLine -= nLines;
    }
    else
    {
        m_currLine = 0;
    }

    refresh();
}// end line_up

void    Viewer::curs_down(size_t nLines)
{
    const size_t    currBufLine     = m_currLine + m_currCursLine;
    size_t          cursLineDiff    = LINES - m_currCursLine - 1;
    
    if (!nLines)
    {
        return;
    }

    if (nLines > cursLineDiff)
    {
        nLines -= cursLineDiff;
    }
    else
    {
        cursLineDiff = nLines;
        nLines = 0;
    }

    m_currCursLine += cursLineDiff;

    // if we are at page bttom, move page down
    if (nLines)
    {
        line_down(nLines);
    }
    else
    {
        refresh();
    }
}// end curs_down

void    Viewer::curs_up(size_t nLines)
{
    const size_t    currBufLine     = m_currLine + m_currCursLine;
    
    if (!nLines)
    {
        return;
    }

    if (nLines > m_currCursLine)
    {
        nLines -= m_currCursLine;
        m_currCursLine = 0;
        if (m_currLine)
        {
            line_up(nLines);
        }
        else
        {
            refresh();
        }
    }
    else
    {
        m_currCursLine -= nLines;
        refresh();
    }
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

    size_t      currBufLine     = m_currLine + m_currCursLine;
    auto&       bufLine         = m_doc->buffer().at(currBufLine);

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

    size_t      currBufLine     = m_currLine + m_currCursLine;
    auto&       bufLine         = m_doc->buffer().at(currBufLine);

    if ((m_bufNodeIter != bufLine.end()) and (m_bufNodeIter->image_ref()))
    {
        return m_doc->images().at(m_bufNodeIter->image_ref()).get_url();
    }

    return NULL_STR;
}// end curr_img

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
    refresh();
    wnoutrefresh(stdscr);

    return out;
}// end prompt_char

auto    Viewer::prompt_string(const string& prompt)
    -> string
{
    WINDOW      *promptWin  = subwin(stdscr, 1, COLS, LINES-1, 0);
    size_t      inputLen    = COLS - prompt.size();
    string      inputPadding(inputLen, ' ');
    string      out         = "";
    int         key;

    refresh();
    mvwaddnstr(promptWin, 0, 0, prompt.c_str(), COLS);
    mvwaddnstr(
        promptWin,
        0,
        prompt.size(),
        string(inputLen, ' ').c_str(),
        inputLen
    );
    wrefresh(promptWin);
    wmove(promptWin, 0, prompt.size());

    while ((key = wgetch(promptWin)))
    {
        int     inputIdx;
        int     cursIdx;

        if (cursIdx > COLS - 1)
        {
            cursIdx = COLS - 1;
        }

        switch (key)
        {
            case KEY_ENTER:
            case '\n':
                goto exit_while;
            case KEY_BACKSPACE:
            case CTRL('h'):
                if (not out.empty())
                {
                    out.pop_back();
                }
                break;
            case CTRL('u'):
                out.clear();
                break;
            default:
                out += key;
                break;
        }// end switch

        inputIdx = out.size() >= inputLen ?
            out.size() - inputLen + 1 :
            0;

        cursIdx = prompt.size() + out.size();

        if (cursIdx > COLS - 1)
        {
            cursIdx = COLS - 1;
        }

        mvwaddstr(promptWin, 0, prompt.size(), inputPadding.c_str());
        mvwaddstr(promptWin, 0, prompt.size(), out.c_str() + inputIdx);
        wrefresh(promptWin);
        wmove(promptWin, 0, cursIdx);
    }// end while
exit_while:
    delwin(promptWin);
    refresh();
    wnoutrefresh(stdscr);

    return out;
}// end prompt_string
