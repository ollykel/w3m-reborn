#ifndef __APP_HPP__
#define __APP_HPP__

#include <curses.h>

#include <list>
#include <map>

#include "deps.hpp"
#include "command.hpp"
#include "page.hpp"
#include "mailcap.hpp"

class App
{
    public:
        // --- public member classes --------------------------------------
        class   Tab;

        // --- public member types ----------------------------------------
        typedef std::list<Tab>                  tabs_container;
        typedef tabs_container::iterator        tabs_iterator;
        typedef std::map<string,Command>        uri_map;

        // --- public constructors ----------------------------------------
        App(void);
        App(Mailcap *mailcap);

        // --- public accessors -------------------------------------------
        auto empty(void) const
            -> bool;
        auto num_tabs(void) const
            -> size_t;
        auto curr_tab(void) const
            -> const Tab&;
        auto tabs(void) const
            -> const tabs_container&;

        // --- public mutators --------------------------------------------
        void set_mailcap(Mailcap *mailcap);
    protected:
        // --- protected member classes -----------------------------------
        struct  KeymapEntry;

        // --- protected member types -------------------------------------
        typedef std::map<int, KeymapEntry>      keymap;
        typedef std::vector<string>             command_args_container;
        typedef void (App::*command_func)(const command_args_container&);
        typedef std::map<string,command_func>   base_commands_map;
        typedef std::map<string,string>         commands_map;

        // --- protected member variables ---------------------------------
        tabs_container          m_tabs                      = {};
        tabs_iterator           m_currTab                   = {};
        uri_map                 m_uriHandlers               = {};
        Mailcap                 *m_mailcap                  = nullptr;
        keymap                  m_keymap                    = {};
        base_commands_map       m_baseCommandDispatcher     = {};
        commands_map            m_commands                  = {};
        WINDOW                  *m_screen                   = nullptr;

        // --- protected mutators -----------------------------------------
        // ------ command functions ---------------------------------------
        void quit(const command_args_container& args);
        void suspend(const command_args_container& args);
        void redraw(const command_args_container& args);
        void goto_url(const command_args_container& args);
        void reload(const command_args_container& args);
        void new_tab(const command_args_container& args);
        void switch_tab(const command_args_container& args);
        void swap_tabs(const command_args_container& args);
        void switch_page(const command_args_container& args);
        void delete_page(const command_args_container& args);
        void define_command(const command_args_container& args);
        void bind_key(const command_args_container& args);
        void set_env(const command_args_container& args);
        void unset_env(const command_args_container& args);
        void shift_page(const command_args_container& args);
        void shift_cursor(const command_args_container& args);
        void add_mailcap_entry(const command_args_container& args);
        void clear_mailcap(const command_args_container& args);
        void read_mailcap_file(const command_args_container& args);
        void exec_shell(const command_args_container& args);
        void command(const command_args_container& args);
        void source_commands(const command_args_container& args);
};// end class App

class App::Tab
{
    public:
        // --- public constructors ----------------------------------------
        Tab(void);

        // --- public accessors -------------------------------------------
        auto empty(void) const
            -> bool;
        auto size(void) const
            -> size_t;
        auto at_front(void) const
            -> bool;
        auto at_back(void) const
            -> bool;
        auto current_page(void) const
            -> const Page&;

        // --- public mutators --------------------------------------------
        auto current_page(void)
            -> Page&;
        auto forward_pages(size_t increm = 1)
            -> Page&;
        auto back_pages(size_t increm = 1)
            -> Page&;
        auto insert_page(const Page& page)
            -> Page&;
    protected:
        // --- protected member types -------------------------------------
        typedef std::list<Page>                  pages_container;
        typedef pages_container::iterator        pages_iter;

        // --- protected member variables ---------------------------------
        pages_container     m_pages         = {};
        pages_iter          m_currPage      = {};
};// end class App::Tab

struct App::KeymapEntry
{
    string                          command     = "";
    std::map<int,KeymapEntry>       children    = {};
};// end struct App::KeymapEntry

#endif
