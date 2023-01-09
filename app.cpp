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
    m_baseCommandDispatcher["SWITCH_PAGE"] = &App::switch_page;
    m_baseCommandDispatcher["DELETE_PAGE"] = &App::delete_page;
    m_baseCommandDispatcher["DEFINE_COMMAND"] = &App::define_command;
    m_baseCommandDispatcher["BIND_KEY"] = &App::bind_key;
    m_baseCommandDispatcher["SET_ENV"] = &App::set_env;
    m_baseCommandDispatcher["UNSET_ENV"] = &App::unset_env;
    m_baseCommandDispatcher["SHIFT_PAGE"] = &App::shift_page;
    m_baseCommandDispatcher["SHIFT_CURSOR"] = &App::shift_cursor;
    m_baseCommandDispatcher["ADD_MAILCAP_ENTRY"] = &App::add_mailcap_entry;
    m_baseCommandDispatcher["CLEAR_MAILCAP"] = &App::clear_mailcap;
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

    Tab::Config                 tabCfg{ config.viewer };
    Tab                         currTab(tabCfg);
    std::vector<Mailcap>        mailcaps;
    int                         key;

    m_config = config;

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
    parse_mailcap_env(mailcaps, getenv("MAILCAPS"));

    // goto init url
    {
        goto_url(currTab, mailcaps, m_config, m_config.initUrl);
        m_currPage = currTab.curr_page();
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
                m_currPage->viewer().curs_down(w3mIndex);
                break;
            // move page up
            case KEY_SF:
            case 'J':
                m_currPage->viewer().line_down(w3mIndex);
                break;
            // move cursor up
            case KEY_UP:
            case 'k':
                m_currPage->viewer().curs_up(w3mIndex);
                break;
            // move page down
            case KEY_SR:
            case 'K':
                m_currPage->viewer().line_up(w3mIndex);
                break;
            // move cursor left
            case KEY_LEFT:
            case 'h':
                m_currPage->viewer().curs_left(w3mIndex);
                break;
            // move cursor right
            case KEY_RIGHT:
            case 'l':
                m_currPage->viewer().curs_right(w3mIndex);
                break;
            // move cursor to first column
            case CTRL('a'):
            case '0':
                m_currPage->viewer().curs_left(SIZE_MAX);
                break;
            case '$':
                m_currPage->viewer().curs_right(COLS);
                break;
            case 'b':
                m_currPage->viewer().line_up(LINES);
                break;
            case 'c':
                m_currPage->viewer().disp_status(m_currPage->uri().str());
                break;
            case ' ':
                m_currPage->viewer().line_down(LINES);
                break;
            case KEY_CLEAR:
            case CTRL('l'):
                m_currPage->viewer().refresh(true);
                break;
            case 'g':
                m_currPage->viewer().curs_up(SIZE_MAX);
                break;
            case 'G':
                m_currPage->viewer().curs_down(m_currPage->document().buffer().size() - 1);
                break;
            case 'u':
                {
                    const string&   str     = m_currPage->viewer().curr_url();

                    if (not str.empty())
                    {
                        m_currPage->viewer().disp_status(str);
                    }
                }
                break;
            case 'I':
                {
                    const string&   str     = m_currPage->viewer().curr_img();

                    if (not str.empty())
                    {
                        Uri         uri     = Uri::from_relative(m_currPage->uri(), str);

                        m_currPage->viewer().disp_status(uri.str());
                    }
                }
                break;
            case 'i':
                {
                    Uri     targetUrl   = m_currPage->viewer().curr_img();

                    if (not targetUrl.empty())
                    {
                        goto_url(currTab, mailcaps, m_config, targetUrl);
                        m_currPage = currTab.curr_page();
                    }
                }
                break;
            case 'U':
                {
                    string      url = m_currPage->viewer().curr_url();

                    if (m_currPage->viewer().prompt_string(url, "Goto URL:"))
                    {
                        Uri             uri     = url;

                        goto_url(currTab, mailcaps, m_config, uri);
                        m_currPage = currTab.curr_page();
                    }
                }
                break;
            case KEY_ENTER:
            case '\n':
                {
                    Document::FormInput     *input;
                    Uri                     targetUrl;

                    if ((input = m_currPage->viewer().curr_form_input()))
                    {
                        handle_form_input(currTab, m_config, mailcaps, *input);
                        m_currPage = currTab.curr_page();
                    }
                    else
                    {
                        targetUrl = m_currPage->viewer().curr_url();

                        if (not targetUrl.empty())
                        {
                            goto_url(currTab, mailcaps, m_config, targetUrl);
                            m_currPage = currTab.curr_page();
                        }
                    }
                }
                break;
            case 'M':
                {
                    Uri     targetUrl   = m_currPage->viewer().curr_url();

                    if (not targetUrl.empty())
                    {
                        // TODO: true external browser API
                        Uri         fullUrl = Uri::from_relative(
                                                m_currPage->uri(),
                                                targetUrl
                                            );
                        Command     cmd     = Command("mpv \"${W3M_URL}\"")
                                            .set_env("W3M_URL", fullUrl.str());

                        endwin();
                        cmd.spawn().wait();
                        doupdate();
                        m_currPage->viewer().refresh(true);
                    }
                }
                break;
            case 'm':
                {
                    Uri     targetUrl   = m_currPage->uri();

                    if (not targetUrl.empty())
                    {
                        // TODO: true external browser API
                        Uri         fullUrl = Uri::from_relative(
                                                m_currPage->uri(),
                                                targetUrl
                                            );
                        Command     cmd     = Command("mpv \"${W3M_URL}\"")
                                            .set_env("W3M_URL", fullUrl.str());

                        endwin();
                        cmd.spawn().wait();
                        doupdate();
                        m_currPage->viewer().refresh(true);
                    }
                }
                break;
            case '<':
                {
                    m_currPage = currTab.prev_page();
                    m_currPage->viewer().refresh(true);
                }
                break;
            case '>':
                {
                    m_currPage = currTab.next_page();
                    m_currPage->viewer().refresh(true);
                }
                break;
            case 'B':
                {
                    m_currPage = currTab.back_page();
                    m_currPage->viewer().refresh(true);
                }
                break;
            // submit form
            case 'p':
                {
                    Document::FormInput     *input;

                    if ((input = m_currPage->viewer().curr_form_input()))
                    {
                        const Document::Form&   form    = input->form();

                        submit_form(currTab, m_config, mailcaps, form);
                        m_currPage = currTab.curr_page();
                    }
                }
                break;
            // show current line number
            case CTRL('g'):
                {
                    stringstream    fmt;

                    fmt << "line ";
                    fmt << m_currPage->viewer().curr_curs_line();
                    fmt << " / ";
                    fmt << m_currPage->viewer().buffer_size();

                    m_currPage->viewer().disp_status(fmt.str());
                }
                break;
            case '!':
                exec_shell({ "EXEC_SHELL", "--hold" });
                break;
            case CTRL('x'):
                set_env({ "SET_ENV" });
                break;
            case CTRL('z'):
                suspend({});
                break;
            case 'q':
                {
                    switch (m_currPage->viewer().prompt_char(
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
auto App::get_uri_handler(const string& scheme) const
    -> HttpFetcher*
{
    if (not m_uriHandlerMap.count(scheme))
    {
        return nullptr;
    }

    return &(*m_uriHandlerMap.at(scheme));
}// end App::get_uri_handler

template <class CONT_T>
void    App::goto_url(
    Tab& tab,
    const CONT_T& mailcaps,
    const Config& cfg,
    const Uri& targetUrl,
    const string& requestMethod,
    const HttpFetcher::data_container& input
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

        if (tab.curr_page())
        {
            prevUri = tab.curr_page()->uri();
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
            data = fetcher->fetch_url(
                status, headers, fullUri, *inData, fetchEnv
            );

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
void    App::handle_data(
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

    if (viewer.prompt_string(value, name + ":"))
    {
        // TODO: differentiate between quitting, empty value
        input.set_value(value);
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
        goto_url(tab, mailcaps, cfg, url);
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

        goto_url(tab, mailcaps, cfg, url, method, data);
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
    // TODO: implement
}// end new_tab

void App::switch_tab(const command_args_container& args)
{
    // TODO: implement
}// end switch_tab

void App::swap_tabs(const command_args_container& args)
{
    // TODO: implement
}// end swap_tabs

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
            if (m_currPage->viewer().prompt_string(prompt, "[SETENV]:"))
            {
                size_t  splitIdx    = prompt.find('=');

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

void App::clear_mailcap(const command_args_container& args)
{
    // TODO: implement
}// end clear_mailcap

void App::read_mailcap_file(const command_args_container& args)
{
    // TODO: implement
}// end read_mailcap_file

void App::exec_shell(const command_args_container& args)
{
    const string&       name            = args.front();
    string              prompt          = "[SHELL]!";
    bool                shouldHold      = false;
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
            ++iter;
            if (iter != args.cend())
            {
                prompt = *iter;
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
                    (not m_currPage->viewer().prompt_string(shellCmd, prompt))
                    or shellCmd.empty()
                )
                {
                    return;
                }

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

    endwin();
    cmd.spawn().wait();
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

