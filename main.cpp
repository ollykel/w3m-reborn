#include <cstdio>
#include <climits>
#include <map>
#include <list>
#include <unordered_set>

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
void    set_form_input(Document::FormInput& input, Viewer& viewer);

template <class CONT_T>
void    parse_mailcap_env(CONT_T& mailcaps, const string& env);

template <class CONT_T>
void    handle_form_input(
    Tab& tab,
    const Config& cfg,
    const CONT_T& mailcaps,
    const HttpFetcher& fetcher,
    Document::FormInput& input
);

template <class CONT_T>
void    submit_form(
    Tab& tab,
    const Config& cfg,
    const CONT_T& mailcaps,
    const HttpFetcher& fetcher,
    const Document::Form& form
);

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
        size_t                      w3mIndex        = 0;

        // read index
        while ((key = wgetch(stdscr)))
        {
            bool        increm      = false;

            switch (key)
            {
                case -1:
                    continue;
                case '0':
                    if (w3mIndex)
                    {
                        w3mIndex *= 10;
                        increm = true;
                    }
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    w3mIndex *= 10;
                    w3mIndex += (key - '0');
                    increm = true;
            }// end switch

            if (not increm)
            {
                break;
            }
        }// end while

        if (not w3mIndex)
        {
            ++w3mIndex;
        }

        switch (key)
        {
            // move cursor down
            case KEY_DOWN:
            case 'j':
                currPage->viewer().curs_down(w3mIndex);
                break;
            // move page up
            case KEY_SF:
            case 'J':
                currPage->viewer().line_down(w3mIndex);
                break;
            // move cursor up
            case KEY_UP:
            case 'k':
                currPage->viewer().curs_up(w3mIndex);
                break;
            // move page down
            case KEY_SR:
            case 'K':
                currPage->viewer().line_up(w3mIndex);
                break;
            // move cursor left
            case KEY_LEFT:
            case 'h':
                currPage->viewer().curs_left(w3mIndex);
                break;
            // move cursor right
            case KEY_RIGHT:
            case 'l':
                currPage->viewer().curs_right(w3mIndex);
                break;
            // move cursor to first column
            case CTRL('a'):
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
            case KEY_CLEAR:
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
                    Document::FormInput     *input;
                    Uri                     targetUrl;

                    if ((input = currPage->viewer().curr_form_input()))
                    {
                        handle_form_input(currTab, cfg, mailcaps, httpFetcher, *input);
                        currPage = currTab.curr_page();
                    }
                    else
                    {
                        targetUrl = currPage->viewer().curr_url();

                        if (not targetUrl.empty())
                        {
                            goto_url(currTab, httpFetcher, mailcaps, cfg, targetUrl);
                            currPage = currTab.curr_page();
                        }
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
            case 'm':
                {
                    Uri     targetUrl   = currPage->uri();

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
            // submit form
            case 'p':
                {
                    Document::FormInput     *input;

                    if ((input = currPage->viewer().curr_form_input()))
                    {
                        const Document::Form&   form    = input->form();
                        submit_form(currTab, cfg, mailcaps, httpFetcher, form);
                        currPage = currTab.curr_page();
                    }
                }
                break;
            // show current line number
            case CTRL('g'):
                {
                    stringstream    fmt;

                    fmt << "line ";
                    fmt << currPage->viewer().curr_curs_line();
                    fmt << " / ";
                    fmt << currPage->viewer().buffer_size();

                    currPage->viewer().disp_status(fmt.str());
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
    using namespace std;

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
        Uri                         target              = targetUrl;
        Uri                         prevUri             = tab.curr_page()->uri();
        Uri                         fullUri;
        const string                *contentType        = nullptr;
        s_ptr<Document>             doc                 = nullptr;
        unordered_set<string>       visitedUris         = {};

        do
        {
            status = {};
            headers.clear();

            fullUri = Uri::from_relative(prevUri, target);

            if (visitedUris.count(fullUri.str()))
            {
                break;
            }
            visitedUris.insert(fullUri.str());
            data = fetcher.fetch_url(status, headers, fullUri);

            prevUri = fullUri;
            if (headers.count("location")
                and (not headers.at("location").empty())
            )
            {
                target = headers.at("location").at(0);
            }
        } while (
            (status.code >= 300)
            and (status.code < 400)
            and headers.count("location")
        );// end do while

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
    sprintf(fbase, "%s/w3mtmp-%016zx", cfg.tempdir.c_str(), counter);
    fname = entry->parse_filename(fbase);

    // write data to tempfile
    if (not entry->file_piped())
    {
        tempFile.open(fname.c_str());
        if (tempFile.fail())
        {
            return;
        }
        ++counter;
        tempFile.write(data.data(), data.size());
        tempFile.close();
    }

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

void    set_form_input(Document::FormInput& input, Viewer& viewer)
{
    const string&   name        = input.name();
    const string&   initVal     = input.value();
    string          val         = viewer.prompt_string(name + ":", initVal);

    // TODO: differentiate between quitting, empty value
    input.set_value(val);
}// end set_form_input

template <class CONT_T>
void    handle_form_input(
    Tab& tab,
    const Config& cfg,
    const CONT_T& mailcaps,
    const HttpFetcher& fetcher,
    Document::FormInput& input
)
{
    switch (input.type())
    {
        case Document::FormInput::Type::text:
        case Document::FormInput::Type::search:
            {
                set_form_input(input, tab.curr_page()->viewer());
                tab.curr_page()->document().redraw(COLS);
                tab.curr_page()->viewer().redraw();
                tab.curr_page()->viewer().refresh();
            }
            break;
        case Document::FormInput::Type::button:
        case Document::FormInput::Type::submit:
            {
                const Document::Form&   form    = input.form();
                submit_form(tab, cfg, mailcaps, fetcher, form);
            }
            break;
        default:
            break;
    }// end switch
}// end handle_form_input

template <class CONT_T>
void    submit_form(
    Tab& tab,
    const Config& cfg,
    const CONT_T& mailcaps,
    const HttpFetcher& fetcher,
    const Document::Form& form
)
{
    std::vector<string>     values  = {};
    Uri                     url;

    url = Uri::from_relative(
        tab.curr_page()->uri(),
        form.action()
    );

    for (const auto& kv : form.values())
    {
        string  val =
            utils::percent_encode(kv.first) +
            "=" +
            utils::percent_encode(kv.second);

        values.push_back(val);
    }// end for kv

    url.query = utils::join_str(values, "&");
    goto_url(tab, fetcher, mailcaps, cfg, url);
}// end submit_form
