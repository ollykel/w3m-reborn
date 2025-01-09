#include <cstdio>
#include <climits>
#include <map>
#include <list>
#include <unordered_set>

#include <unistd.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "deps.hpp"
#include "uri.hpp"
#include "app.hpp"
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

// === Function Prototypes ================================================
int     runtime(const App::Config& cfg);

void    handle_sigwinch(int sig);

void    handle_signal_term(int sig);

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

// === Global Variables ========================================================
App     *MAIN_APP;

// === main ===============================================================
//
// ========================================================================
int main(const int argc, const char **argv, const char **envp)
{
    using namespace std;

    int             ret         = EXIT_FAILURE;
    int             wstatus;
    #define     CURL_COMMAND    \
    "curl " \
        "--include " \
        "--request \"${W3M_REQUEST_METHOD}\" " \
        "--data @- " \
        "--user-agent \"${W3M_USER_AGENT}\" " \
        "\"${W3M_URL}\""
    App::Config     config      = {
        // uriHandlers
        {
            // TODO: write scripts for each handler
            {
                "file",
                "echo "
                    "-n \"content-type: \"; mimetype --brief \"${W3M_URL}\"; "
                    CURL_COMMAND
            },
            {
                "http",
                CURL_COMMAND
            },
            {
                "https",
                CURL_COMMAND
            },
        },
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
        // Main debugger
        {
            // TODO: dynamically choose temp directory
            "/tmp/w3mdebug.log",        // filename
            0,                          // limitDefault
            "MAIN",                     // prefix
            "%a, %d %b %Y %T %z",       // timeFormat
        },
    };

    #undef  CURL_COMMAND

    // get debug level
    if (getenv("W3M_DEBUG_LEVEL"))
    {
        sscanf(
            getenv("W3M_DEBUG_LEVEL"),
            " %d",
            &config.debuggerMain.limitDefault
        );
    }

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

    // set up signal handler(s)
    signal(SIGINT, handle_signal_term);

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

    // main runtime
    ret = runtime(config);
finally:
    // reset screen
    use_default_colors();
    curs_set(1);
    endwin();

    return ret;
}// end main

// === Function Definitions ===============================================

int runtime(const App::Config& cfg)
{
    using namespace std;

    App     app;

    MAIN_APP = &app;
    signal(SIGWINCH, handle_sigwinch);

    return app.run(cfg);
}// end runtime

void    handle_sigwinch(int sig)
{
    MAIN_APP->redraw(true);
    wrefresh(stdscr);
}// end int handle_sigwinch(int sig)

void    handle_signal_term(int sig)
{
    // reset screen
    use_default_colors();
    curs_set(1);
    endwin();

    // if we get here, we aren't exiting under normal circumstances
    exit(EXIT_FAILURE);
}// end handle_signal_term

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
        Uri                         prevUri             = {};
        Uri                         fullUri;
        const string                *contentType        = nullptr;
        s_ptr<Document>             doc                 = nullptr;
        unordered_set<string>       visitedUris         = {};

        if (tab.curr_page())
        {
            prevUri = tab.curr_page()->uri();
        }

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
