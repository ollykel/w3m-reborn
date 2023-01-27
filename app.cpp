#include <signal.h>
#include <curses.h>

#include "deps.hpp"
#include "app.hpp"
#include "uri.hpp"
#include "document.hpp"
#include "document_text.hpp"
#include "document_html.hpp"
#include "http_fetcher.hpp"
#include "tab.hpp"

// === class App Implementation ===========================================
//
// ========================================================================

// --- public constructors ------------------------------------------------
App::App(void)
{
    // initialize base commands
    m_baseCommandDispatcher["QUIT"] = &App::quit;
    m_baseCommandDispatcher["SUSPEND"] = &App::suspend;
    m_baseCommandDispatcher["REDRAW"] = &App::redraw;
    m_baseCommandDispatcher["GOTO"] = &App::goto_url;
    m_baseCommandDispatcher["RELOAD"] = &App::reload;
    m_baseCommandDispatcher["NEW_TAB"] = &App::new_tab;
    m_baseCommandDispatcher["SWITCH_TAB"] = &App::switch_tab;
    m_baseCommandDispatcher["SWAP_TABS"] = &App::swap_tabs;
    m_baseCommandDispatcher["DELETE_TAB"] = &App::delete_tab;
    m_baseCommandDispatcher["SWITCH_PAGE"] = &App::switch_page;
    m_baseCommandDispatcher["DELETE_PAGE"] = &App::delete_page;
    m_baseCommandDispatcher["DEFINE_COMMAND"] = &App::define_command;
    m_baseCommandDispatcher["BIND_KEY"] = &App::bind_key;
    m_baseCommandDispatcher["SET_ENV"] = &App::set_env;
    m_baseCommandDispatcher["UNSET_ENV"] = &App::unset_env;
    m_baseCommandDispatcher["SHIFT_PAGE"] = &App::shift_page;
    m_baseCommandDispatcher["SHIFT_CURSOR"] = &App::shift_cursor;
    m_baseCommandDispatcher["ADD_MAILCAP_ENTRY"] = &App::add_mailcap_entry;
    m_baseCommandDispatcher["CLEAR_MAILCAP"] = &App::clear_mailcaps;
    m_baseCommandDispatcher["READ_MAILCAP_FILE"] = &App::read_mailcap_file;
    m_baseCommandDispatcher["EXEC_SHELL"] = &App::exec_shell;
    m_baseCommandDispatcher["COMMAND"] = &App::command;
    m_baseCommandDispatcher["SOURCE_COMMANDS"] = &App::source_commands;

    for (const auto& kv : m_baseCommandDispatcher)
    {
        m_commands[kv.first] = kv.first;
    }// end for kv
}// end App::App

App::App(Mailcap *mailcap)
    : App()
{
    set_mailcap(mailcap);
}// end App::App

// --- public accessors ---------------------------------------------------
auto App::empty(void) const
    -> bool
{
    return m_tabs.empty();
}// end App::empty

auto App::num_tabs(void) const
    -> size_t
{
    return m_tabs.size();
}// end App::num_tabs

auto App::curr_tab(void) const
    -> const Tab&
{
    return *m_currTab;
}// end App::curr_tab

auto App::tabs(void) const
    -> const tabs_container&
{
    return m_tabs;
}// end App::tabs

// --- public mutators ----------------------------------------------------
void App::set_mailcap(Mailcap *mailcap)
{
    m_mailcap = mailcap;
}// end App::set_mailcap

auto App::run(const Config& config)
    -> int
{
    using namespace std;

    // === TODO: move to header file ======================================
    #define     CTRL(KEY)   ((KEY) & 0x1f)

    Tab::Config                 tabCfg;
    int                         key;

    m_config = config;

    // init debugger
    m_debuggerMain = Debugger(config.debuggerMain);

    // init first tab
    tabCfg = { config.viewer };
    m_tabs.emplace_back(tabCfg);
    m_currTab = m_tabs.begin();

    // print error message and exit if no initial url
    if (m_config.initUrl.empty())
    {
        curs_set(0);
        waddnstr(stdscr, "ERROR: no url given", COLS);
        wrefresh(stdscr);

        sleep(3);
        curs_set(1);

        return EXIT_FAILURE;
    }

    // init uri handlers
    for (const auto& kv : m_config.uriHandlers)
    {
        const auto&     scheme          = kv.first;
        const auto&     shellCommand    = kv.second;

        m_uriHandlers.emplace_front(shellCommand, "W3M_URL");
        m_uriHandlerMap.emplace(scheme, m_uriHandlers.begin());
    }// end for

    // init colors
    init_pair(
        Viewer::COLOR_PAIR_STANDARD,
        m_config.viewer.attribs.standard.fg,
        m_config.viewer.attribs.standard.bg
    );
    init_pair(
        Viewer::COLOR_PAIR_INPUT,
        m_config.viewer.attribs.input.fg,
        m_config.viewer.attribs.input.bg
    );
    init_pair(
        Viewer::COLOR_PAIR_IMAGE,
        m_config.viewer.attribs.image.fg,
        m_config.viewer.attribs.image.bg
    );
    init_pair(
        Viewer::COLOR_PAIR_LINK,
        m_config.viewer.attribs.link.fg,
        m_config.viewer.attribs.link.bg
    );
    init_pair(
        Viewer::COLOR_PAIR_LINK_CURRENT,
        m_config.viewer.attribs.linkCurrent.fg,
        m_config.viewer.attribs.linkCurrent.bg
    );
    init_pair(
        Viewer::COLOR_PAIR_LINK_VISITED,
        m_config.viewer.attribs.linkVisited.fg,
        m_config.viewer.attribs.linkVisited.bg
    );

    // build mailcap file
    parse_mailcap_env(m_mailcaps, getenv("MAILCAPS"));

    // goto init url
    {
        goto_url(m_config.initUrl);
        m_currPage = m_currTab->curr_page();
    }

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
                curr_page().viewer().curs_down(w3mIndex);
                break;
            // move page up
            case KEY_SF:
            case 'J':
                curr_page().viewer().line_down(w3mIndex);
                break;
            // move cursor up
            case KEY_UP:
            case 'k':
                curr_page().viewer().curs_up(w3mIndex);
                break;
            // move page down
            case KEY_SR:
            case 'K':
                curr_page().viewer().line_up(w3mIndex);
                break;
            // move cursor left
            case KEY_LEFT:
            case 'h':
                curr_page().viewer().curs_left(w3mIndex);
                break;
            // move cursor right
            case KEY_RIGHT:
            case 'l':
                curr_page().viewer().curs_right(w3mIndex);
                break;
            // move cursor to first column
            case CTRL('a'):
            case '0':
                curr_page().viewer().curs_left(SIZE_MAX);
                break;
            case '$':
                curr_page().viewer().curs_right(COLS);
                break;
            case 'b':
                curr_page().viewer().line_up(LINES);
                break;
            case 'c':
                curr_page().viewer().disp_status(curr_page().uri().str());
                break;
            case ' ':
                curr_page().viewer().line_down(LINES);
                break;
            case KEY_CLEAR:
            case CTRL('l'):
                redraw(true);
                break;
            case 'g':
                curr_page().viewer().curs_up(SIZE_MAX);
                break;
            case 'G':
                curr_page().viewer().curs_down(curr_page().document().buffer().size() - 1);
                break;
            case 'u':
                {
                    const string&   str     = curr_page().viewer().curr_url();

                    if (not str.empty())
                    {
                        curr_page().viewer().disp_status(str);
                    }
                }
                break;
            case 'I':
                {
                    const string&   str     = curr_page().viewer().curr_img();

                    if (not str.empty())
                    {
                        Uri         uri     = Uri::from_relative(curr_page().uri(), str);

                        curr_page().viewer().disp_status(uri.str());
                    }
                }
                break;
            case 'i':
                {
                    Uri     targetUrl   = curr_page().viewer().curr_img();

                    if (not targetUrl.empty())
                    {
                        goto_url(targetUrl);
                        m_currPage = m_currTab->curr_page();
                    }
                }
                break;
            case 'U':
                {
                    string      url = curr_page().viewer().curr_url();

                    if (curr_page().viewer().prompt_string(
                        url, "Goto URL:", m_histories["URL"]
                    ))
                    {
                        Uri             uri     = url;

                        m_histories.at("URL").push_back(url);
                        goto_url(uri);
                        m_currPage = m_currTab->curr_page();
                    }
                }
                break;
            case 'T':
                new_tab({ "NEW_TAB" });
                break;
            case CTRL('q'):
                delete_tab({ "DELETE_TAB" });
                break;
            case '[':
                switch_tab({ "SWITCH_TAB", "-1" });
                break;
            case ']':
                switch_tab({ "SWITCH_TAB", "+1" });
                break;
            case KEY_ENTER:
            case '\n':
                {
                    Document::FormInput     *input;
                    Uri                     targetUrl;

                    if ((input = curr_page().viewer().curr_form_input()))
                    {
                        handle_form_input(*m_currTab, m_config, m_mailcaps, *input);
                        m_currPage = m_currTab->curr_page();
                    }
                    else
                    {
                        targetUrl = curr_page().viewer().curr_url();

                        if (not targetUrl.empty())
                        {
                            goto_url(targetUrl);
                            m_currPage = m_currTab->curr_page();
                        }
                    }
                }
                break;
            case 'M':
                {
                    Uri     targetUrl   = curr_page().viewer().curr_url();

                    if (not targetUrl.empty())
                    {
                        // TODO: true external browser API
                        Uri         fullUrl = Uri::from_relative(
                                                curr_page().uri(),
                                                targetUrl
                                            );
                        Command     cmd     = Command("mpv \"${W3M_URL}\"")
                                            .set_env("W3M_URL", fullUrl.str());

                        endwin();
                        cmd.spawn().wait();
                        doupdate();
                        curr_page().viewer().refresh(true);
                    }
                }
                break;
            case 'm':
                {
                    Uri     targetUrl   = curr_page().uri();

                    if (not targetUrl.empty())
                    {
                        // TODO: true external browser API
                        Uri         fullUrl = Uri::from_relative(
                                                curr_page().uri(),
                                                targetUrl
                                            );
                        Command     cmd     = Command("mpv \"${W3M_URL}\"")
                                            .set_env("W3M_URL", fullUrl.str());

                        endwin();
                        cmd.spawn().wait();
                        doupdate();
                        curr_page().viewer().refresh(true);
                    }
                }
                break;
            case '<':
                {
                    m_currPage = m_currTab->prev_page();
                    curr_page().viewer().refresh(true);
                }
                break;
            case '>':
                {
                    m_currPage = m_currTab->next_page();
                    curr_page().viewer().refresh(true);
                }
                break;
            case 'B':
                {
                    m_currPage = m_currTab->back_page();
                    curr_page().viewer().refresh(true);
                }
                break;
            // submit form
            case 'p':
                {
                    Document::FormInput     *input;

                    if ((input = curr_page().viewer().curr_form_input()))
                    {
                        const Document::Form&   form    = input->form();

                        submit_form(*m_currTab, m_config, m_mailcaps, form);
                        m_currPage = m_currTab->curr_page();
                    }
                }
                break;
            // show current line number
            case CTRL('g'):
                {
                    stringstream    fmt;

                    fmt << "line ";
                    fmt << curr_page().viewer().curr_curs_line();
                    fmt << " / ";
                    fmt << curr_page().viewer().buffer_size();

                    curr_page().viewer().disp_status(fmt.str());
                }
                break;
            case '!':
                exec_shell({ "EXEC_SHELL", "--hold" });
                break;
            case '@':
                exec_shell({
                    "READ_SHELL",
                    "--read-output",
                    "--prompt", "[READ_SHELL]!"
                });
                break;
            case '|':
                exec_shell({
                    "PIPE_SHELL",
                    "--hold",
                    "--write-input", "BUFFER",
                    "--prompt", "[PIPE_SHELL]!"
                });
                break;
            case 'f':
                // TODO: more configurable
                prompt_url({
                    "PROMPT_URL",
                    "--method", "GET",
                    "--prompt", "(search google):",
                    "https://www.google.com/search?q=%s"
                });
                break;
            case CTRL('x'):
                set_env({ "SET_ENV" });
                break;
            case CTRL('z'):
                suspend({});
                break;
            case 'q':
                {
                    switch (curr_page().viewer().prompt_char(
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

    // === TODO: move to header file ======================================
    #undef  CTRL
}// end App::run

// --- protected mutators -------------------------------------------------
auto App::curr_tab(void)
    -> Tab&
{
    return *m_currTab;
}// end App::curr_tab

auto App::curr_page(void)
    -> Tab::Page&
{
    return *m_currTab->curr_page();
}// end App::curr_page

auto App::get_uri_handler(const string& scheme) const
    -> HttpFetcher*
{
    if (not m_uriHandlerMap.count(scheme))
    {
        return nullptr;
    }

    return &(*m_uriHandlerMap.at(scheme));
}// end App::get_uri_handler

void App::draw_tab_headers(void)
{
    if (LINES < 2)
    {
        // we don't have enough room
        return;
    }
    else
    {
        static const string     OPEN    = "[";
        static const string     PART    = "][";
        static const string     CLOS    = "]";

        const Tab       *currTab        = &(*m_currTab);
        const size_t    numParts        = m_tabs.size() - 1;
        const size_t    nPartChars      = numParts * PART.length();
        const size_t    headerLen
            = (COLS - nPartChars - OPEN.length() - CLOS.length())
                / m_tabs.size();
        const string    headerLine
            = utils::join_str(m_tabs, PART,
            [&headerLen, &currTab](const Tab& tab)
            {
                if (&tab == currTab)
                {
                    static const string     marker  = "*";

                    string                  str     = marker;

                    str += tab.curr_page()->title().substr(
                        0, headerLen - (2 * marker.length())
                    );
                    str += marker;

                    return utils::pad_str(
                        str, headerLen, utils::Justify::CENTER, ' ', true
                    );
                }
                else
                {
                    return utils::pad_str(
                        tab.curr_page()->title(), headerLen,
                        utils::Justify::CENTER,
                        ' ', true
                    );
                }
            });

        // draw opening
        mvwaddnstr(stdscr, 0, 0, OPEN.c_str(), COLS);
        // draw tab headers
        mvwaddnstr(stdscr, 0, OPEN.length(), headerLine.c_str(), COLS - OPEN.length());
        // draw closing
        mvwaddnstr(stdscr, 0, COLS - CLOS.length(), CLOS.c_str(), COLS);
        mvwaddnstr(stdscr, 1, 0, string(COLS, '~').c_str(), COLS);
    }
}// end App::draw_tab_headers

void App::redraw(bool retouch)
{
    if (m_tabs.size() > 1)
    {
        for (auto& tab : m_tabs)
        {
            tab.set_start_line(2);
        }// end for
        draw_tab_headers();
    }
    else
    {
        for (auto& tab : m_tabs)
        {
            tab.set_start_line(0);
        }// end for
    }

    curr_page().viewer().refresh(retouch);
    wrefresh(stdscr);
}// end App::redraw

void    App::goto_url(
    const Uri& targetUrl,
    const string& requestMethod,
    const HttpFetcher::data_container& input
)
{
    using namespace std;

    if (targetUrl.is_fragment())
    {
        if (not curr_page().viewer().goto_section(targetUrl.fragment))
        {
            curr_page().viewer().disp_status(
                "ERROR: could not find #" + targetUrl.fragment
            );
        }
    }
    else if (not targetUrl.empty())
    {
        static const string                         defContentType
            = "text/plain";
        static const HttpFetcher::data_container    nullData
            = {};

        HttpFetcher::Status                 status              = {};
        HttpFetcher::header_type            headers             = {};
        std::vector<char>                   data                = {};
        std::map<string,string>             fetchEnv            = {};
        Uri                                 target              = targetUrl;
        const HttpFetcher::data_container   *inData             = &input;
        Uri                                 prevUri             = {};
        Uri                                 fullUri;
        const string                        *contentType        = nullptr;
        s_ptr<Document>                     doc                 = nullptr;
        unordered_set<string>               visitedUris         = {};

        fetchEnv["W3M_REQUEST_METHOD"] = requestMethod;

        if (curr_tab().curr_page())
        {
            prevUri = curr_page().uri();
        }

        do
        {
            HttpFetcher     *fetcher        = nullptr;

            status = {};
            headers.clear();

            fullUri = Uri::from_relative(prevUri, target);

            if (visitedUris.count(fullUri.str()))
            {
                break;
            }

            if (not (fetcher = get_uri_handler(fullUri.scheme)))
            {
                break;
            }

            visitedUris.insert(fullUri.str());

            try
            {
                data = fetcher->fetch_url(
                    status, headers, fullUri, *inData, fetchEnv
                );
            }
            catch (const std::exception& e)
            {
                m_debuggerMain.printf(
                    1,
                    "%s: could not fetch %s",
                    m_debuggerMain.format_curr_time().c_str(),
                    fullUri.str().c_str()
                );
                throw e;
            }

            fetchEnv["W3M_REQUEST_METHOD"] = "GET";
            inData = &nullData;
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
            curr_page().viewer().refresh(true);
            curr_page().viewer().disp_status(
                "ERROR: could not identify content type"
            );
            goto finally;
        }
        else if (*contentType == "text/plain")
        {
            doc.reset(new DocumentText(
                m_config.document,
                string(data.cbegin(), data.cend()),
                COLS
            ));
        }
        else if (*contentType == "text/html")
        {
            doc.reset(new DocumentHtml(
                m_config.document,
                string(data.cbegin(), data.cend()),
                COLS
            ));
        }

        if (doc)
        {
            curr_tab().push_document(doc, fullUri);
        }
        else
        {
            handle_data(*contentType, data);
        }
finally:
        redraw(true);
    }
}// end goto_url

void    App::handle_data(
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

    for (const auto& mailcap : m_mailcaps)
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
    sprintf(fbase, "%s/w3mtmp-%016zx", m_config.tempdir.c_str(), counter);
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

void    App::parse_mailcap_file(Mailcap& mailcap, const string& fname)
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
void    App::parse_mailcap_env(CONT_T& mailcaps, const string& env)
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

void    App::set_form_input(Document::FormInput& input, Viewer& viewer)
{
    const string&   name        = input.name();
    string          value       = input.value();

    if (viewer.prompt_string(value, name + ":", m_histories["TEXT"]))
    {
        // TODO: differentiate between quitting, empty value
        input.set_value(value);
        m_histories.at("TEXT").push_back(value);
    }
}// end set_form_input

template <class CONT_T>
void    App::handle_form_input(
    Tab& tab,
    const Config& cfg,
    const CONT_T& mailcaps,
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
                Document::Form&     form    = input.form();

                if (
                    (input.type() == Document::FormInput::Type::submit)
                    and (not input.name().empty())
                )
                {
                    input.set_is_active(true);
                }

                submit_form(tab, cfg, mailcaps, form);

                if (
                    (input.type() == Document::FormInput::Type::submit)
                    and (not input.name().empty())
                )
                {
                    input.set_is_active(false);
                }
            }
            break;
        case Document::FormInput::Type::checkbox:
        case Document::FormInput::Type::radio:
            {
                input.set_is_active(not input.is_active());
                tab.curr_page()->document().redraw(COLS);
                tab.curr_page()->viewer().redraw();
                tab.curr_page()->viewer().refresh();
            }
            break;
        default:
            break;
    }// end switch
}// end handle_form_input

template <class CONT_T>
void    App::submit_form(
    Tab& tab,
    const Config& cfg,
    const CONT_T& mailcaps,
    const Document::Form& form
)
{
    std::vector<string>     values          = {};
    Uri                     url;
    HttpFetcher             *fetcher;
    string                  method          = form.method();

    url = Uri::from_relative(
        tab.curr_page()->uri(),
        form.action()
    );

    utils::to_upper(method);

    if (not (fetcher = get_uri_handler(url.scheme)))
    {
        return;
    }

    for (const auto& kv : form.values())
    {
        string      val     = "";

        val += utils::percent_encode(kv.first);
        val.push_back('=');
        val += utils::percent_encode(kv.second);

        values.push_back(val);
    }// end for kv

    // Case 1: empty method
    // Case 2: GET method
    if (
        method.empty()
        or ("GET" == method)
    )
    {
        url.query = utils::join_str(values, "&");
        goto_url(url);
    }
    // Case 3: other methods (i.e. POST, PUT, DELETE, etc.)
    else
    {
        // TODO: create 'slice' interface, or make
        // HttpFetcher::fetch_url templated
        const string                        dataStr
            = utils::join_str(values, "&");
        const HttpFetcher::data_container   data
            = { dataStr.cbegin(), dataStr.cend() };

        goto_url(url, method, data);
    }
}// end submit_form

// ------ command functions -----------------------------------------------
void App::quit(const command_args_container& args)
{
    // TODO: implement
    bool        shouldPrompt        = false;

    for (auto arg = args.begin(); arg != args.end(); ++arg)
    {
        if ("--prompt" == *arg or "-p" == *arg)
        {
            shouldPrompt = true;
        }
    }// end for arg

    endwin();
    // TODO: teardown, call closing scripts
    
    m_shouldTerminate = true;
}// end quit

void App::suspend(const command_args_container& _)
{
    endwin();

    // suspend process
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_DFL);
    kill((pid_t) 0x00, SIGTSTP);
#else
    kill((pid_t) 0x00, SIGSTOP);
#endif

    refresh();
    redraw({});
}// end suspend

void App::redraw(const command_args_container& args)
{
    if (m_currPage)
    {
        m_currPage->viewer().refresh(true);
    }
}// end redraw

void App::goto_url(const command_args_container& args)
{
    // TODO: implement
}// end goto_url

void App::reload(const command_args_container& args)
{
    // TODO: implement
}// end reload

void App::new_tab(const command_args_container& args)
{
    Tab::Config     cfg         = { m_config.viewer };
    auto&           currPage    = curr_page();
    auto            pos         = m_currTab;

    ++pos;
    m_currTab = m_tabs.emplace(pos, cfg);
    m_currTab->set_start_line(m_tabs.size() > 1 ? 2 : 0);
    m_currTab->push_page(currPage);
    redraw(true);
}// end new_tab

void App::switch_tab(const command_args_container& args)
{
    using namespace std;

    const string    *arg    = nullptr;

    if (args.size() != 2)
    {
        goto print_usage;
    }

    arg = &args.at(1);

    if (arg->empty())
    {
        goto print_usage;
    }

    switch (arg->front())
    {
        case '+':
            {
                int     diff;

                if (sscanf(arg->c_str() + 1, " %d", &diff) != 1)
                {
                    goto print_usage;
                }

                diff %= m_tabs.size();

                for (; diff; --diff)
                {
                    ++m_currTab;

                    if (m_currTab == m_tabs.end())
                    {
                        m_currTab = m_tabs.begin();
                    }
                }// end for
            }
            break;
        case '-':
            {
                int     diff;

                if (sscanf(arg->c_str() + 1, " %d", &diff) != 1)
                {
                    goto print_usage;
                }

                diff %= m_tabs.size();

                for (; diff; --diff)
                {
                    if (m_currTab == m_tabs.begin())
                    {
                        m_currTab = m_tabs.end();
                    }

                    --m_currTab;
                }// end for
            }
            break;
        default:
            {
                int     index;

                if (
                    (sscanf(arg->c_str(), " %d", &index) != 1)
                    or (index < 0)
                )
                {
                    goto print_usage;
                }

                index %= m_tabs.size();
                m_currTab = m_tabs.begin();

                for (; index; --index)
                {
                    ++m_currTab;
                }// end for
            }
            break;
    }// end switch

    redraw(true);
    return;
print_usage:
    curr_page().viewer().disp_status(
        "usage: " + args.front() + " INDEX"
    );
}// end switch_tab

void App::swap_tabs(const command_args_container& args)
{
    // TODO: implement
}// end swap_tabs

void App::delete_tab(const command_args_container& args)
{
    if (m_tabs.size() < 2)
    {
        return;
    }

    const auto      oldIter     = m_currTab;

    if (m_currTab == m_tabs.begin())
    {
        ++m_currTab;
    }
    else
    {
        --m_currTab;
    }

    m_tabs.erase(oldIter);
    redraw(true);
}// end App::delete_tab

void App::switch_page(const command_args_container& args)
{
    // TODO: implement
}// end switch_page

void App::delete_page(const command_args_container& args)
{
    // TODO: implement
}// end delete_page

void App::define_command(const command_args_container& args)
{
    // TODO: implement
}// end define_command

void App::bind_key(const command_args_container& args)
{
    // TODO: implement
}// end bind_key

void App::set_env(const command_args_container& args)
{
    string      prompt      = "";

    switch (args.size())
    {
        case 2:
            prompt = args.at(1);
            prompt.push_back('=');
        case 1:
            if (m_currPage->viewer().prompt_string(
                prompt, "[SETENV]:", m_histories["TEXT"]
            ))
            {
                size_t  splitIdx    = prompt.find('=');

                m_histories.at("TEXT").push_back(prompt);
                if (splitIdx and (splitIdx != string::npos))
                {
                    string      key     = prompt.substr(0, splitIdx);
                    string      val     = prompt.substr(splitIdx + 1);

                    setenv(key.c_str(), val.c_str(), 1);
                }
            }
            break;
        case 3:
            setenv(args.at(1).c_str(), args.at(2).c_str(), 1);
            break;
        default:
            m_currPage->viewer().disp_status(
                "usage: " + args.at(0) + " [ENV] [VALUE]"
            );
            break;
    }
    // TODO: implement
}// end set_env

void App::unset_env(const command_args_container& args)
{
    // TODO: implement
}// end unset_env

void App::shift_page(const command_args_container& args)
{
    // TODO: implement
}// end shift_page

void App::shift_cursor(const command_args_container& args)
{
    // TODO: implement
}// end shift_cursor

void App::add_mailcap_entry(const command_args_container& args)
{
    // TODO: implement
}// end add_mailcap_entry

void App::clear_mailcaps(const command_args_container& args)
{
    m_mailcaps.clear();
}// end clear_mailcap

void App::read_mailcap_file(const command_args_container& args)
{
    string              fName   = "";
    enum class End
    {
        FRONT   = 1,
        BACK    = 2,
    }                   end     = End::FRONT;
    Mailcap             *dest   = nullptr;
    auto                iter    = args.cbegin() + 1;

    for (; iter != args.cend(); ++iter)
    {
        const string&   arg     = *iter;

        if (arg == "--prepend")
        {
            end = End::FRONT;
        }
        else if (arg == "--append")
        {
            end = End::BACK;
        }
        else
        {
            break;
        }
    }// end for

    if (iter == args.cend())
    {
        if (not curr_page().viewer().prompt_string(
            fName, "Mailcap file:", m_histories["FILE"]
        ))
        {
            return;
        }
        else
        {
            m_histories.at("FILE").push_back(fName);
        }
    }
    else
    {
        fName = args.at(1);
    }

    switch (end)
    {
        case End::FRONT:
            m_mailcaps.emplace_front();
            dest = &m_mailcaps.front();
            break;
        case End::BACK:
            m_mailcaps.emplace_back();
            dest = &m_mailcaps.back();
            break;
    }// end switch

    // TODO: handle invalid file
    parse_mailcap_file(*dest, fName);
}// end read_mailcap_file

void App::exec_shell(const command_args_container& args)
{
    const string&       name            = args.front();
    string              prompt          = "[SHELL]!";
    bool                shouldHold      = false;
    bool                shouldRead      = false;
    enum class WriteInput
    {
        NONE    = 0,
        SOURCE  = 1,
        BUFFER  = 2,
    }                   writeInput      = WriteInput::NONE;
    auto                iter            = args.cbegin() + 1;
    Command             cmd;

    for (; iter != args.cend(); ++iter)
    {
        const string&   arg     = *iter;

        if ((arg == "--hold") or (arg == "-h"))
        {
            shouldHold = true;
        }
        else if ((arg == "--prompt") or (arg == "-p"))
        {
            if (++iter != args.cend())
            {
                prompt = *iter;
            }
        }
        else if ((arg == "--read-output") or (arg == "-R"))
        {
            shouldRead = true;
        }
        else if ((arg == "--write-input") or (arg == "-W"))
        {
            string      inputState      = "";

            if (++iter == args.cend())
            {
                break;
            }

            utils::to_upper(inputState = *iter);

            if (inputState == "NONE")
            {
                writeInput = WriteInput::NONE;
            }
            else if (inputState == "SOURCE")
            {
                writeInput = WriteInput::SOURCE;
            }
            else if (inputState == "BUFFER")
            {
                writeInput = WriteInput::BUFFER;
            }
        }
        // end command args; rest are shell args
        else if (arg == "--")
        {
            break;
        }
        else
        {
            break;
        }
    }// end for

    switch (args.cend() - iter)
    {
        case 0:
            {
                string  shellCmd  = "";

                if (
                    (not m_currPage->viewer().prompt_string(
                        shellCmd, prompt, m_histories["SHELL"]
                    ))
                    or shellCmd.empty()
                )
                {
                    return;
                }

                m_histories.at("SHELL").push_back(shellCmd);
                cmd = Command(shellCmd);
            }
            break;
        default:
            {
                const std::vector<string>   cmdArgs(iter, args.cend());

                cmd = Command(cmdArgs);
            }
            break;
    }// end switch

    cmd.set_stdin_piped(writeInput != WriteInput::NONE);
    cmd.set_stdout_piped(shouldRead);

    endwin();
    {
        auto    sproc   = cmd.spawn();

        // write to stdin, if applicable
        switch (writeInput)
        {
            case WriteInput::NONE:
                // do nothing
                break;
            case WriteInput::SOURCE:
                // TODO: implement
                sproc.stdin().close();
                break;
            case WriteInput::BUFFER:
                {
                    string  input   = curr_page().document().buffer_string();

                    sproc.stdin().write(input.data(), input.length());
                    sproc.stdin().close();
                }
                break;
        }// end switch

        // read output as new document
        if (shouldRead)
        {
            s_ptr<Document>     doc;

            doc.reset(new DocumentText(
                m_config.document,
                sproc.stdout(),
                COLS
            ));
            m_currPage = m_currTab->push_document(doc, {});
        }

        sproc.wait();
    }
    if (shouldHold)
    {
        std::cout << std::endl;
        std::cout << "[Press any key to return]";
        std::cin.get();
    }
    m_currPage->viewer().refresh(1);
}// end exec_shell

void App::command(const command_args_container& args)
{
    // TODO: implement
}// end command

void App::source_commands(const command_args_container& args)
{
    // TODO: implement
}// end source_commands

void App::prompt_url(const command_args_container& args)
{
    using namespace std;

    string      method          = "GET";
    string      fmt             = "";
    string      prompt          = "";
    string      value           = "";
    auto        iter            = args.cbegin() + 1;

    for (; iter != args.cend(); ++iter)
    {
        const string&   arg     = *iter;

        if ((arg == "--method") or (arg == "-m"))
        {
            if (++iter == args.cend())
            {
                break;
            }

            method = *iter;
        }
        else if ((arg == "--prompt") or (arg == "-p"))
        {
            if (++iter == args.cend())
            {
                break;
            }

            prompt = *iter;
        }
        else if ((arg == "--default") or (arg == "-d"))
        {
            if (++iter == args.cend())
            {
                break;
            }

            value = *iter;
        }
        else
        {
            break;
        }
    }// end for

    if (iter == args.cend())
    {
        curr_page().viewer().disp_status(
            "usage: " + args.front()
                + " [--method|-m METHOD]"
                + " [--prompt|-p PROMPT]"
                + " [--default|-d DEFAULT]"
                + " URL_FORMAT"
        );
    }

    fmt = *iter;
    if (prompt.empty())
    {
        prompt = fmt + ":";
    }

    if (curr_page().viewer().prompt_string(value, prompt, m_histories["URL"]))
    {
        // TODO: handle escapes (%%)
        size_t      pos             = 0;
        string      valueEncoded    = utils::percent_encode(value);

        m_histories.at("URL").push_back(value);

        while ((pos = fmt.find("%s", pos)) != string::npos)
        {
            fmt.replace(pos, 2, valueEncoded);
            pos += valueEncoded.length();
        }// end while

        goto_url(fmt, method);
    }
}// end App::prompt_url

