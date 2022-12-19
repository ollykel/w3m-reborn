#include <cstdio>
#include <climits>
#include <map>
#include <list>

#include <curses.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "deps.hpp"
#include "uri.hpp"
#include "command.hpp"
#include "html_parser.hpp"
#include "dom_tree.hpp"
#include "document_html.hpp"
#include "viewer.hpp"

// === TODO: move to header file ==========================================
#define     CTRL(KEY)   ((KEY) & 0x1f)

struct  Config
{
    string                  fetchCommand;
    string                  initUrl;
    Viewer::Config          viewer;
    Document::Config        document;
};// end struct Config

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
        "curl --include --user-agent \"${W3M_USER_AGENT}\" ${W3M_URL}",
        // initUrl
        "",
        // viewer
        {
            // attribs
            {
                { COLOR_WHITE, COLOR_DEFAULT, A_NORMAL },// standard
                { COLOR_RED, COLOR_DEFAULT, A_UNDERLINE },// input
                { COLOR_GREEN, COLOR_DEFAULT, A_BOLD },// image
                { COLOR_BLUE, COLOR_DEFAULT, A_NORMAL },// link
                { COLOR_CYAN, COLOR_DEFAULT, A_NORMAL },// linkCurrent
                { COLOR_MAGENTA, COLOR_DEFAULT, A_NORMAL },// linkVisited
            },
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
        cfg.viewer.attribs.standard.fg,
        cfg.viewer.attribs.standard.bg
    );
    init_pair(
        COLOR_PAIR_INPUT,
        cfg.viewer.attribs.input.fg,
        cfg.viewer.attribs.input.bg
    );
    init_pair(
        COLOR_PAIR_IMAGE,
        cfg.viewer.attribs.image.fg,
        cfg.viewer.attribs.image.bg
    );
    init_pair(
        COLOR_PAIR_LINK,
        cfg.viewer.attribs.link.fg,
        cfg.viewer.attribs.link.bg
    );
    init_pair(
        COLOR_PAIR_LINK_CURRENT,
        cfg.viewer.attribs.linkCurrent.fg,
        cfg.viewer.attribs.linkCurrent.bg
    );
    init_pair(
        COLOR_PAIR_LINK_VISITED,
        cfg.viewer.attribs.linkVisited.fg,
        cfg.viewer.attribs.linkVisited.bg
    );

    // emplace new page
    pages.emplace_back();
    currPage = &pages.back();
    currViewer = &currPage->viewer;

    // fetch url, init viewer
    currPage->uri = cfg.initUrl;
    currPage->documentPtr = fetcher.fetch_url(cfg.initUrl, headers);
    *currViewer = Viewer(cfg.viewer, currPage->documentPtr.get());

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
            case CTRL('l'):
                currViewer->refresh(true);
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
                        *currViewer = Viewer(cfg.viewer, currPage->documentPtr.get());
                        currViewer->refresh(true);
                    }
                }
                break;
            case KEY_ENTER:
            case '\n':
                {
                    std::map<string, string>    headers;
                    Uri     currUrl     = currViewer->curr_url();
                    Uri     targetUri;

                    if (not currUrl.empty())
                    {
                        if (currUrl.is_fragment())
                        {
                            const string&   section = currUrl.fragment;
                            auto    idx
                                = currPage->documentPtr->get_section_index(section);

                            if (idx)
                            {
                                currViewer->goto_point(idx.line, 0);
                            }
                            else
                            {
                                currViewer->disp_status(
                                    "ERROR: could not find #" +
                                    currUrl.fragment
                                );
                            }
                        }
                        else
                        {
                            targetUri = Uri::from_relative(currPage->uri, currUrl);

                            // emplace new page
                            pages.emplace_back();
                            currPage = &pages.back();
                            currViewer = &currPage->viewer;

                            // fetch url, init viewer
                            currPage->uri = targetUri;
                            currPage->documentPtr = fetcher.fetch_url(targetUri.str(), headers);
                            *currViewer = Viewer(cfg.viewer, currPage->documentPtr.get());
                            currViewer->refresh(true);
                        }
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
