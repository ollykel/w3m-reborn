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
#include "http_fetcher.hpp"
#include "html_parser.hpp"
#include "dom_tree.hpp"
#include "document_text.hpp"
#include "document_html.hpp"
#include "document_fetcher.hpp"
#include "tab.hpp"
#include "viewer.hpp"
#include "mailcap.hpp"

// === TODO: move to header file ==========================================
#define     CTRL(KEY)   ((KEY) & 0x1f)

struct  Config
{
    string                  fetchCommand;
    string                  initUrl;
    string                  tempdir;
    Viewer::Config          viewer;
    Document::Config        document;
};// end struct Config

struct Page
{
    s_ptr<Document>     documentPtr;
    Viewer              viewer;
    Uri                 uri;
};// end struct Page

// === Function Prototypes ================================================
int     runtime(const Config& cfg);

template <class CONT_T>
void    goto_url(
    Tab& tab,
    const HttpFetcher& fetcher,
    const CONT_T& mailcaps,
    const Config& cfg,
    const Uri& targetUrl
);

template <class CONT_T>
void    handle_data(
    const CONT_T& mailcaps,
    const Config& cfg,
    const string& mimeType,
    const std::vector<char>& data
);
void    parse_mailcap_file(Mailcap& mailcap, const string& fname);

template <class CONT_T>
void    parse_mailcap_env(CONT_T& mailcaps, const string& env);

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
        // tempdir
        // TODO: actually set from ENV
        "/tmp",
        // viewer
        {
            // attribs
            {
                { Viewer::COLOR_DEFAULT, Viewer::COLOR_DEFAULT, A_NORMAL },// standard
                { COLOR_RED, Viewer::COLOR_DEFAULT, A_UNDERLINE },// input
                { COLOR_GREEN, Viewer::COLOR_DEFAULT, A_BOLD },// image
                { COLOR_BLUE, Viewer::COLOR_DEFAULT, A_NORMAL },// link
                { COLOR_CYAN, Viewer::COLOR_DEFAULT, A_NORMAL },// linkCurrent
                { COLOR_MAGENTA, Viewer::COLOR_DEFAULT, A_NORMAL },// linkVisited
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
    raw();
    noecho();
    setlocale(LC_ALL, "");

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

    Tab::Config                 tabCfg{ cfg.viewer };
    HttpFetcher                 httpFetcher(cfg.fetchCommand, "W3M_URL");
    DocumentFetcher             fetcher(cfg.fetchCommand, cfg.document);
    Tab                         currTab(tabCfg, fetcher);
    Tab::Page                   *currPage;
    std::vector<Mailcap>        mailcaps;
    int                         key;

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
        Viewer::COLOR_PAIR_STANDARD,
        cfg.viewer.attribs.standard.fg,
        cfg.viewer.attribs.standard.bg
    );
    init_pair(
        Viewer::COLOR_PAIR_INPUT,
        cfg.viewer.attribs.input.fg,
        cfg.viewer.attribs.input.bg
    );
    init_pair(
        Viewer::COLOR_PAIR_IMAGE,
        cfg.viewer.attribs.image.fg,
        cfg.viewer.attribs.image.bg
    );
    init_pair(
        Viewer::COLOR_PAIR_LINK,
        cfg.viewer.attribs.link.fg,
        cfg.viewer.attribs.link.bg
    );
    init_pair(
        Viewer::COLOR_PAIR_LINK_CURRENT,
        cfg.viewer.attribs.linkCurrent.fg,
        cfg.viewer.attribs.linkCurrent.bg
    );
    init_pair(
        Viewer::COLOR_PAIR_LINK_VISITED,
        cfg.viewer.attribs.linkVisited.fg,
        cfg.viewer.attribs.linkVisited.bg
    );

    // build mailcap file
    parse_mailcap_env(mailcaps, getenv("MAILCAPS"));

    currPage = currTab.goto_uri(cfg.initUrl);
    currPage->viewer().refresh(true);

    // wait for keypress
    while (true)
    {
        switch ((key = wgetch(stdscr)))
        {
            // move cursor down
            case 'j':
                currPage->viewer().curs_down();
                break;
            // move page up
            case 'J':
                currPage->viewer().line_down();
                break;
            // move cursor up
            case 'k':
                currPage->viewer().curs_up();
                break;
            // move page down
            case 'K':
                currPage->viewer().line_up();
                break;
            // move cursor left
            case 'h':
                currPage->viewer().curs_left();
                break;
            // move cursor right
            case 'l':
                currPage->viewer().curs_right();
                break;
            // move cursor to first column
            case '0':
                currPage->viewer().curs_left(SIZE_MAX);
                break;
            case '$':
                currPage->viewer().curs_right(COLS);
                break;
            case 'b':
                currPage->viewer().line_up(LINES);
                break;
            case 'c':
                currPage->viewer().disp_status(currPage->uri().str());
                break;
            case ' ':
                currPage->viewer().line_down(LINES);
                break;
            case CTRL('l'):
                currPage->viewer().refresh(true);
                break;
            case 'g':
                currPage->viewer().curs_up(SIZE_MAX);
                break;
            case 'G':
                currPage->viewer().curs_down(currPage->document().buffer().size() - 1);
                break;
            case 'u':
                {
                    const string&   str     = currPage->viewer().curr_url();

                    if (not str.empty())
                    {
                        currPage->viewer().disp_status(str);
                    }
                }
                break;
            case 'I':
                {
                    const string&   str     = currPage->viewer().curr_img();

                    if (not str.empty())
                    {
                        Uri         uri     = Uri::from_relative(currPage->uri(), str);

                        currPage->viewer().disp_status(uri.str());
                    }
                }
                break;
            case 'i':
                {
                    Uri     targetUrl   = currPage->viewer().curr_img();

                    if (not targetUrl.empty())
                    {
                        goto_url(currTab, httpFetcher, mailcaps, cfg, targetUrl);
                        currPage = currTab.curr_page();
                    }
                }
                break;
            case 'U':
                {
                    const string&   currUrl = currPage->viewer().curr_url();
                    const string&   url     = currPage->viewer()
                                            .prompt_string("Goto URL:", currUrl);

                    if (not url.empty())
                    {
                        goto_url(currTab, httpFetcher, mailcaps, cfg, url);
                        currPage = currTab.curr_page();
                    }
                }
                break;
            case KEY_ENTER:
            case '\n':
                {
                    Uri     targetUrl   = currPage->viewer().curr_url();

                    if (not targetUrl.empty())
                    {
                        goto_url(currTab, httpFetcher, mailcaps, cfg, targetUrl);
                        currPage = currTab.curr_page();
                    }
                }
                break;
            case 'M':
                {
                    Uri     targetUrl   = currPage->viewer().curr_url();

                    if (not targetUrl.empty())
                    {
                        // TODO: true external browser API
                        Uri         fullUrl = Uri::from_relative(
                                                currPage->uri(),
                                                targetUrl
                                            );
                        Command     cmd     = Command("mpv \"${W3M_URL}\"")
                                            .set_env("W3M_URL", fullUrl.str());

                        endwin();
                        cmd.spawn().wait();
                        doupdate();
                        currPage->viewer().refresh(true);
                    }
                }
                break;
            case '<':
                {
                    currPage = currTab.prev_page();
                    currPage->viewer().refresh(true);
                }
                break;
            case '>':
                {
                    currPage = currTab.next_page();
                    currPage->viewer().refresh(true);
                }
                break;
            case 'B':
                {
                    currPage = currTab.back_page();
                    currPage->viewer().refresh(true);
                }
                break;
            case 'q':
                {
                    switch (currPage->viewer().prompt_char(
                        "Are you sure you want to quit? (y/N):"
                    ))
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

template <class CONT_T>
void    goto_url(
    Tab& tab,
    const HttpFetcher& fetcher,
    const CONT_T& mailcaps,
    const Config& cfg,
    const Uri& targetUrl
)
{
    if (targetUrl.is_fragment())
    {
        if (not tab.curr_page()->viewer().goto_section(targetUrl.fragment))
        {
            tab.curr_page()->viewer().disp_status(
                "ERROR: could not find #" + targetUrl.fragment
            );
        }
    }
    else if (not targetUrl.empty())
    {
        static const string         defContentType      = "text/plain";
        HttpFetcher::Status         status              = {};
        HttpFetcher::header_type    headers             = {};
        std::vector<char>           data                = {};
        Uri                         fullUri;
        const string                *contentType        = nullptr;
        s_ptr<Document>             doc                 = nullptr;

        fullUri = Uri::from_relative(tab.curr_page()->uri(), targetUrl);
        data = fetcher.fetch_url(status, headers, fullUri);

        if (
            headers.count("content-type")
            and (not headers.at("content-type").empty())
        )
        {
            contentType = &headers.at("content-type").front();
        }

        // create document, if applicable
        if (not contentType)
        {
            tab.curr_page()->viewer().refresh(true);
            tab.curr_page()->viewer().disp_status(
                "ERROR: could not identify content type"
            );
            goto finally;
        }
        else if (*contentType == "text/plain")
        {
            doc.reset(new DocumentText(
                cfg.document,
                string(data.cbegin(), data.cend()),
                COLS
            ));
        }
        else if (*contentType == "text/html")
        {
            doc.reset(new DocumentHtml(
                cfg.document,
                string(data.cbegin(), data.cend()),
                COLS
            ));
        }

        if (doc)
        {
            tab.push_document(doc, fullUri);
        }
        else
        {
            handle_data(mailcaps, cfg, *contentType, data);
        }
finally:
        tab.curr_page()->viewer().refresh(true);
    }
}// end goto_url

template <class CONT_T>
void    handle_data(
    const CONT_T& mailcaps,
    const Config& cfg,
    const string& mimeType,
    const std::vector<char>& data
)
{
    using namespace std;

    static size_t           counter         = 0;
    char                    fbase[0x100]    = {};
    string                  fname           = {};
    const Mailcap::Entry    *entry          = nullptr;
    ofstream                tempFile;
    Command                 cmd;

    for (const auto& mailcap : mailcaps)
    {
        if ((entry = mailcap.get_entry(mimeType)))
        {
            break;
        }
    }// end for
    
    if (not entry)
    {
        return;
    }

    // TODO: handle path separators, maximum filename length
    sprintf(fbase, "%s/w3mtmp-%016zu", cfg.tempdir.c_str(), counter);
    fname = entry->parse_filename(fbase);

    // write data to tempfile
    tempFile.open(fname.c_str());
    if (tempFile.fail())
    {
        return;
    }
    ++counter;
    tempFile.write(data.data(), data.size());
    tempFile.close();

    cmd = entry->create_command(fbase, mimeType);

    if (entry->needs_terminal())
    {
        endwin();
    }

    auto sproc = cmd.spawn();

    // pipe file contents to process, if necessary
    if (entry->file_piped())
    {
        sproc.stdin().write(data.data(), data.size());
        sproc.stdin().close();
    }

    if (entry->needs_terminal())
    {
        sproc.wait();
        doupdate();
    }
}// end handle_data

void    parse_mailcap_file(Mailcap& mailcap, const string& fname)
{
    using namespace std;

    ifstream    inFile;
    string      line;

    inFile.open(fname);
    if (inFile.fail())
    {
        return;
    }

    while (getline(inFile, line))
    {
        size_t      idx     = line.find('#');

        if (string::npos != idx)
        {
            line.erase(idx);
        }
        if (line.empty())
        {
            continue;
        }

        mailcap.parse_entry(line);
    }// end while

    inFile.close();
}// end parse_mailcap_file

template <class CONT_T>
void    parse_mailcap_env(CONT_T& mailcaps, const string& env)
{
    using namespace std;

    size_t      idx     = 0;
    size_t      beg     = 0;
    size_t      end     = 0;

    while (idx < env.length())
    {
        mailcaps.emplace_back();
        Mailcap&    mailcap     = mailcaps.back();

        string      fname   = "";

        // TODO: handle different env separators
        idx = env.find(':', beg);
        if (string::npos == idx)
        {
            end = env.length();
        }
        else
        {
            end = idx;
            ++idx;
        }

        fname = env.substr(beg, end);
        parse_mailcap_file(mailcap, fname);

        beg = idx;
    }// end while
}// end parse_mailcap_env
