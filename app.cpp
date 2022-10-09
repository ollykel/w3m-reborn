#include <signal.h>
#include <curses.h>

#include "deps.hpp"
#include "app.hpp"

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

// --- protected mutators -------------------------------------------------

// ------ command functions -----------------------------------------------
void App::quit(const command_args_container& args)
{
    // TODO: implement
    bool        shouldPrompt        = false;

    for (auto arg = args.begin(); arg != args.end(); ++arg)
    {
    }// end for arg
}// end quit

void App::suspend(const command_args_container& args)
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
    // TODO: implement
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
    // TODO: implement
}// end exec_shell

void App::command(const command_args_container& args)
{
    // TODO: implement
}// end command

void App::source_commands(const command_args_container& args)
{
    // TODO: implement
}// end source_commands

// === class App::Tab Implementation ======================================
//
// ========================================================================

// --- public constructors ------------------------------------------------
App::Tab::Tab(void)
{
    m_currPage = m_pages.begin();
}// end App::Tab::Tab

// --- public accessors ---------------------------------------------------
auto App::Tab::empty(void) const
    -> bool
{
    return m_pages.empty();
}// end 

auto App::Tab::size(void) const
    -> size_t
{
    return m_pages.size();
}// end App::Tab::size

auto App::Tab::at_front(void) const
    -> bool
{
    return m_currPage == m_pages.begin();
}// end App::Tab::at_front

auto App::Tab::at_back(void) const
    -> bool
{
    return m_currPage == m_pages.end();
}// end App::Tab::at_back

auto App::Tab::current_page(void) const
    -> const Page&
{
    return *m_currPage;
}// end App::Tab::current_page

// --- public mutators ----------------------------------------------------
auto App::Tab::current_page(void)
    -> Page&
{
    return *m_currPage;
}// end App::Tab::current_page

auto App::Tab::forward_pages(size_t increm)
    -> Page&
{
    while (increm and m_currPage != m_pages.end())
    {
        ++m_currPage;
        --increm;
    }// end while
    if (not empty() and m_currPage == m_pages.end())
    {
        --m_currPage;
    }
    return *m_currPage;
}// end App::Tab::forward_pages

auto App::Tab::back_pages(size_t increm)
    -> Page&
{
    if (not empty())
    {
        while (increm)
        {
            --m_currPage;
            --increm;
            if (m_currPage == m_pages.begin())
            {
                break;
            }
        }// end while
    }

    return *m_currPage;
}// end App::Tab::back_pages

auto App::Tab::insert_page(const Page& page)
    -> Page&
{
    if (empty())
    {
        m_pages.emplace_back(page);
        m_currPage = m_pages.begin();
    }
    else
    {
        m_currPage = m_pages.insert(m_currPage, page);
    }

    return *m_currPage;
}// end App::Tab::insert_page
