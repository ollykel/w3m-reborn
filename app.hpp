#ifndef __APP_HPP__
#define __APP_HPP__

#include <curses.h>

#include <list>
#include <map>

#include "deps.hpp"
#include "uri.hpp"
#include "command.hpp"
#include "http_fetcher.hpp"
#include "html_parser.hpp"
#include "dom_tree.hpp"
#include "document.hpp"
#include "tab.hpp"
#include "viewer.hpp"
#include "mailcap.hpp"

class App
{
    public:
        // --- public member types ----------------------------------------
        struct  Config
        {
            string                  fetchCommand;
            Uri                     initUrl;
            string                  tempdir;
            Viewer::Config          viewer;
            Document::Config        document;
        };// end struct Config
        typedef std::list<Tab>                  tabs_container;
        typedef tabs_container::iterator        tabs_iterator;
        typedef std::map<string,Command>        uri_method_map;

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
        uri_method_map          m_uriHandlers               = {};
        Mailcap                 *m_mailcap                  = nullptr;
        keymap                  m_keymap                    = {};
        base_commands_map       m_baseCommandDispatcher     = {};
        commands_map            m_commands                  = {};
        WINDOW                  *m_screen                   = nullptr;
        bool                    m_shouldTerminate           = false;

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

struct App::KeymapEntry
{
    string                          command     = "";
    std::map<int,KeymapEntry>       children    = {};
};// end struct App::KeymapEntry

#endif
