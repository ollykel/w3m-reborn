#include <cstdio>
#include <climits>
#include <map>
#include <list>

#include <curses.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "deps.hpp"
#include "command.hpp"
#include "html_parser.hpp"
#include "dom_tree.hpp"
#include "document_html.hpp"
#include "uri.hpp"

#define     COLOR_DEFAULT               -1

#define     COLOR_PAIR_STANDARD         0x01
#define     COLOR_PAIR_INPUT            0x02
#define     COLOR_PAIR_IMAGE            0x03
#define     COLOR_PAIR_LINK             0x04
#define     COLOR_PAIR_LINK_CURRENT     0x05
#define     COLOR_PAIR_LINK_VISITED     0x06

// === TODO: move to header file ==========================================
#define     CTRL(KEY)   ((KEY) & 0x1f)

struct  Attrib
{
    short   fg;
    short   bg;
    int     attr;
};// end struct Attrib

struct  Config
{
    string                  fetchCommand;
    string                  initUrl;
    struct
    {
        Attrib      standard;
        Attrib      input;
        Attrib      image;
        Attrib      link;
        Attrib      linkCurrent;
        Attrib      linkVisited;
    }                       attribs;
    Document::Config        document;
};// end struct Config

class   Viewer
{
    public:
        // === public constructors ========================================
        Viewer(const Config& cfg = {}, Document *doc = nullptr)
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

        Viewer(const Viewer& other)
        {
            copy_from(other);
        }// end copy constructor

        ~Viewer(void)
        {
            destruct();
        }// end destructor

        // === public mutators ============================================
        auto    operator=(const Viewer& other)
            -> Viewer&
        {
            if (this != &other)
            {
                destruct();
                copy_from(other);
            }

            return *this;
        }// end operator=

        void    destruct(void)
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

        void    copy_from(const Viewer& other)
        {
            m_cfg = other.m_cfg;
            m_doc = other.m_doc;
            m_currLine = other.m_currLine;
            m_currCursLine = other.m_currCursLine;
            m_currCol = other.m_currCol;
            m_isSinglePage = other.m_isSinglePage;

            if (m_doc)
            {
                redraw();
            }
        }// end copy_from

        void    refresh(void)
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

            wmove(m_pad, currBufLine, m_currCol);
            prefresh(m_pad, m_currLine, 0, 0, 0, LINES - 1, COLS - 1);
        }// end refresh

        void redraw(void)
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
                    mvwaddnstr(m_pad, i, j, node.text().c_str(), remCols);
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

        void    line_down(size_t nLines = 1)
        {
            m_currLine += nLines;

            if (m_currLine >= m_doc->buffer().size() - LINES)
            {
                m_currLine = m_doc->buffer().size() - LINES;
            }

            refresh();
        }// end line_down

        void    line_up(size_t nLines = 1)
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

        void    curs_down(size_t nLines = 1)
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

        void    curs_up(size_t nLines = 1)
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

        void    curs_left(size_t nCols = 1)
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

        void    curs_right(size_t nCols = 1)
        {
            m_currCol += nCols;

            if (m_currCol >= COLS)
            {
                m_currCol = COLS - 1;
            }

            refresh();
        }// end curs_right

        auto    curr_url(void)
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

        void    disp_status(const string& str)
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

        void    clear_status(void)
        {
            if (m_statusWin)
            {
                return;
            }
            delwin(m_statusWin);
            wnoutrefresh(stdscr);
            m_statusWin = nullptr;
        }// end clear_status

        auto    prompt_char(const string& str)
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

        auto    prompt_string(const string& prompt)
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
    private:
        // === private member variables ===================================
        Config                                  m_cfg;
        WINDOW                                  *m_pad              = nullptr;
        WINDOW                                  *m_statusWin        = nullptr;
        Document                                *m_doc              = nullptr;
        Document::buffer_type::const_iterator   m_bufLineIter;
        Document::BufferLine::const_iterator    m_bufNodeIter;
        size_t                                  m_currLine          = 0;
        size_t                                  m_currCursLine      = 0;
        size_t                                  m_currCol           = 0;
        bool                                    m_isSinglePage      = false;
};// end class Viewer

class DocumentFetcher
{
    public:
        // --- public constructors ----------------------------------------
        DocumentFetcher(const string& shellCmd, const Document::Config& documentConfig)
        {
            m_cmd = Command(shellCmd);
            m_cmd.set_stdout_piped(true);
            m_documentConfig = documentConfig;
        }// end constructor

        // --- public accessors -------------------------------------------
        auto    fetch_url(const string& url, std::map<string, string>& headers) const
            -> s_ptr<Document>
        {
            Command             cmd         = m_cmd;
            s_ptr<Document>     docPtr;

            cmd.set_env("W3M_URL", url);

            auto        sproc   = cmd.spawn();

            while (sproc.stdout())
            {
                string      line;
                string      key;
                string      val;
                size_t      colonIdx;

                getline(sproc.stdout(), line);

                if (line.empty() or (line.size() == 1 and line[0] == '\r'))
                {
                    break;
                }

                colonIdx = line.find(':');

                if (string::npos == colonIdx)
                {
                    continue;
                }

                key = line.substr(0, line.find(':'));

                for (auto& ch : key)
                {
                    key = tolower(ch);
                }// end for

                for (auto iter = line.begin() + colonIdx; iter != line.end(); ++iter)
                {
                    if (' ' != *iter and '\t' != *iter)
                    {
                        for (auto iterB = iter; iter != line.end(); ++iter)
                        {
                            if (';' == *iterB)
                            {
                                val = string(iter, iterB);
                                break;
                            }
                        }// end for
                        if (val.empty())
                        {
                            val = string(iter, line.end());
                        }
                        break;
                    }
                }// end for

                headers[key] = val;
            }// end while

            // TODO: differentiate document types
            // if (headers.at("content-type") == "document/html")
            {
                DocumentHtml    *docHtml    = new DocumentHtml(m_documentConfig);

                docPtr.reset(docHtml);
                docHtml->from_stream(sproc.stdout(), COLS);

                sproc.stdout().close();
                sproc.wait();
            }

            return docPtr;
        }// end fetch_url

        // --- public mutators --------------------------------------------
        void    set_property(const string& key, const string& val)
        {
            m_cmd.set_env(key, val);
        }// end set_property

        auto    document_config(void)
            -> Document::Config&
        {
            return m_documentConfig;
        }
    private:
        // --- private member variables -----------------------------------
        Command             m_cmd;
        Document::Config    m_documentConfig;
};// end class DocumentFetcher

struct Page
{
    s_ptr<Document>     documentPtr;
    Viewer              viewer;
    Uri                 uri;
};// end struct Page

// === Function Prototypes ================================================
int runtime(const Config& cfg);

// === main ===============================================================
//
// ========================================================================
int main(const int argc, const char **argv, const char **envp)
{
    using namespace std;

    int         ret         = EXIT_FAILURE;
    int         wstatus;
    pid_t       child;
    Config      config      = {
        // fetchCommand
        "curl --include ${W3M_URL}",
        // initUrl
        "",
        // attribs
        {
            { COLOR_WHITE, COLOR_DEFAULT, A_NORMAL },// standard
            { COLOR_RED, COLOR_DEFAULT, A_UNDERLINE },// input
            { COLOR_GREEN, COLOR_DEFAULT, A_BOLD },// image
            { COLOR_BLUE, COLOR_DEFAULT, A_NORMAL },// link
            { COLOR_CYAN, COLOR_DEFAULT, A_NORMAL },// linkCurrent
            { COLOR_MAGENTA, COLOR_DEFAULT, A_NORMAL },// linkVisited
        },
        {
            // inputWidth
            {
                40,         // def
                0,          // min
                SIZE_MAX,   // max
            },
        },
    };

    // get url
    if (argc > 1)
    {
        config.initUrl = argv[1];
    }
    else
    {
        const char  *url    = getenv("WWW_HOME");

        if (url)
        {
            config.initUrl = url;
        }
    }

    // initialize screen
    initscr();
    cbreak();
    noecho();

    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    wtimeout(stdscr, 100);
    start_color();
    use_default_colors();

    switch ((child = fork()))
    {
        case 0:
            // we are child process
            return runtime(config);
        case -1:
            // could not fork
            cerr << "ERROR: could not spawn child process" << endl;
            ret = EXIT_FAILURE;
        default:
            // we are parent
            waitpid(child, &wstatus, 0x00);

            ret = WEXITSTATUS(wstatus);
    }// end switch

finally:
    // reset screen
    use_default_colors();
    curs_set(1);
    endwin();

    return ret;
}// end main

// === Function Definitions ===============================================

int runtime(const Config& cfg)
{
    using namespace std;

    std::map<string,string>     headers         = {};
    DocumentFetcher             fetcher         = {
                                                    cfg.fetchCommand,
                                                    cfg.document
                                                };
    std::list<Page>         pages;
    Page                    *currPage;
    Viewer                  *currViewer;
    s_ptr<Document>         documentPtr;

    DocumentHtml    doc(cfg.document);
    char            key;

    // print error message and exit if no initial url
    if (cfg.initUrl.empty())
    {
        curs_set(0);
        waddnstr(stdscr, "ERROR: no url given", COLS);
        wrefresh(stdscr);

        sleep(3);
        curs_set(1);

        return EXIT_FAILURE;
    }

    // init colors
    init_pair(
        COLOR_PAIR_STANDARD,
        cfg.attribs.standard.fg,
        cfg.attribs.standard.bg
    );
    init_pair(
        COLOR_PAIR_INPUT,
        cfg.attribs.input.fg,
        cfg.attribs.input.bg
    );
    init_pair(
        COLOR_PAIR_IMAGE,
        cfg.attribs.image.fg,
        cfg.attribs.image.bg
    );
    init_pair(
        COLOR_PAIR_LINK,
        cfg.attribs.link.fg,
        cfg.attribs.link.bg
    );
    init_pair(
        COLOR_PAIR_LINK_CURRENT,
        cfg.attribs.linkCurrent.fg,
        cfg.attribs.linkCurrent.bg
    );
    init_pair(
        COLOR_PAIR_LINK_VISITED,
        cfg.attribs.linkVisited.fg,
        cfg.attribs.linkVisited.bg
    );

    // emplace new page
    pages.emplace_back();
    currPage = &pages.back();
    currViewer = &currPage->viewer;

    // fetch url, init viewer
    currPage->uri = cfg.initUrl;
    currPage->documentPtr = fetcher.fetch_url(cfg.initUrl, headers);
    *currViewer = Viewer(cfg, currPage->documentPtr.get());

    // wait for keypress
    while (true)
    {
        switch ((key = wgetch(stdscr)))
        {
            // move cursor down
            case 'j':
                currViewer->curs_down();
                break;
            // move page up
            case 'J':
                currViewer->line_down();
                break;
            // move cursor up
            case 'k':
                currViewer->curs_up();
                break;
            // move page down
            case 'K':
                currViewer->line_up();
                break;
            // move cursor left
            case 'h':
                currViewer->curs_left();
                break;
            // move cursor right
            case 'l':
                currViewer->curs_right();
                break;
            // move cursor to first column
            case '0':
                currViewer->curs_left(SIZE_MAX);
                break;
            case '$':
                currViewer->curs_right(COLS);
                break;
            case 'b':
                currViewer->line_up(LINES);
                break;
            case 'c':
                currViewer->disp_status(currPage->uri.str());
                break;
            case ' ':
                currViewer->line_down(LINES);
                break;
            case 'g':
                currViewer->curs_up(SIZE_MAX);
                break;
            case 'G':
                currViewer->curs_down(doc.buffer().size() - 1);
                break;
            case 'u':
                {
                    const string&   str     = currViewer->curr_url();

                    if (not str.empty())
                    {
                        currViewer->disp_status(str);
                    }
                }
                break;
            case 'U':
                {
                    const string&   url     = currViewer->prompt_string("Goto URL:");

                    if (not url.empty())
                    {
                        Uri             uri         = Uri::from_relative(currPage->uri, url);
                        std::map<string, string>    headers;

                        // emplace new page
                        pages.emplace_back();
                        currPage = &pages.back();
                        currViewer = &currPage->viewer;

                        // fetch url, init viewer
                        currPage->uri = uri;
                        currPage->documentPtr = fetcher.fetch_url(uri.str(), headers);
                        *currViewer = Viewer(cfg, currPage->documentPtr.get());
                        currViewer->refresh();
                    }
                }
                break;
            case KEY_ENTER:
            case '\n':
                {
                    const string&   url         = currViewer->curr_url();
                    Uri             uri         = Uri::from_relative(currPage->uri, url);
                    std::map<string, string>    headers;

                    if (not url.empty())
                    {
                        // emplace new page
                        pages.emplace_back();
                        currPage = &pages.back();
                        currViewer = &currPage->viewer;

                        // fetch url, init viewer
                        currPage->uri = uri;
                        currPage->documentPtr = fetcher.fetch_url(uri.str(), headers);
                        *currViewer = Viewer(cfg, currPage->documentPtr.get());
                        currViewer->refresh();
                    }
                }
                break;
            case 'q':
                {
                    switch (currViewer->prompt_char("Are you sure you want to quit? (y/N):"))
                    {
                        case 'y':
                        case 'Y':
                            return EXIT_SUCCESS;
                    }// end switch
                }
                break;
            case 'Q':
                return EXIT_SUCCESS;
        }// end switch
    }// end while

    return EXIT_SUCCESS;
}// end runtime
