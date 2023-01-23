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
        typedef std::map<string,string>         uri_command_map;
        struct  Config
        {
            uri_command_map         uriHandlers;
            Uri                     initUrl;
            string                  tempdir;
            Viewer::Config          viewer;
            Document::Config        document;
        };// end struct Config
        typedef std::list<Tab>
            tabs_container;
        typedef tabs_container::iterator
            tabs_iterator;
        typedef std::map<string,Viewer::history_container>
            history_map;

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
        auto run(const Config& config)
            -> int;
    protected:
        // --- protected member classes -----------------------------------
        struct  KeymapEntry;

        // --- protected member types -------------------------------------
        typedef std::map<int, KeymapEntry>              keymap;
        typedef std::vector<string>                     command_args_container;
        typedef void (App::*command_func)(const command_args_container&);
        typedef std::map<string,command_func>           base_commands_map;
        typedef std::map<string,string>                 commands_map;
        typedef std::list<HttpFetcher>                  uri_handler_container;
        typedef uri_handler_container::iterator         uri_handler_pointer;
        typedef std::map<string,uri_handler_pointer>    uri_handler_map;
        typedef std::deque<Mailcap>                     mailcap_container;

        // --- protected member variables ---------------------------------
        Config                  m_config                    = {};
        tabs_container          m_tabs                      = {};
        tabs_iterator           m_currTab                   = {};
        Tab::Page               *m_currPage                 = nullptr;
        uri_handler_container   m_uriHandlers               = {};
        uri_handler_map         m_uriHandlerMap             = {};
        Mailcap                 *m_mailcap                  = nullptr;
        mailcap_container       m_mailcaps                  = {};
        keymap                  m_keymap                    = {};
        base_commands_map       m_baseCommandDispatcher     = {};
        commands_map            m_commands                  = {};
        WINDOW                  *m_screen                   = nullptr;
        bool                    m_shouldTerminate           = false;
        history_map             m_histories                 = {};

        // --- protected mutators -----------------------------------------
        auto curr_tab(void)
            -> Tab&;
        auto curr_page(void)
            -> Tab::Page&;
        auto get_uri_handler(const string& scheme) const
            -> HttpFetcher*;
        void draw_tab_headers(void);
        void redraw(bool retouch = false);

        void    goto_url(
            const Uri& targetUrl,
            const string& requestMethod = "GET",
            const HttpFetcher::data_container& input = {}
        );

        void    handle_data(
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
            Document::FormInput& input
        );

        template <class CONT_T>
        void    submit_form(
            Tab& tab,
            const Config& cfg,
            const CONT_T& mailcaps,
            const Document::Form& form
        );

        // ------ command functions ---------------------------------------
        void quit(const command_args_container& args);
        void suspend(const command_args_container& _);
        void redraw(const command_args_container& args);
        void goto_url(const command_args_container& args);
        void reload(const command_args_container& args);
        void new_tab(const command_args_container& args);
        void switch_tab(const command_args_container& args);
        void swap_tabs(const command_args_container& args);
        void delete_tab(const command_args_container& args);
        void switch_page(const command_args_container& args);
        void delete_page(const command_args_container& args);
        void define_command(const command_args_container& args);
        void bind_key(const command_args_container& args);
        void set_env(const command_args_container& args);
        void unset_env(const command_args_container& args);
        void shift_page(const command_args_container& args);
        void shift_cursor(const command_args_container& args);
        void add_mailcap_entry(const command_args_container& args);
        void clear_mailcaps(const command_args_container& args);
        void read_mailcap_file(const command_args_container& args);
        void exec_shell(const command_args_container& args);
        void command(const command_args_container& args);
        void source_commands(const command_args_container& args);
        void prompt_url(const command_args_container& args);
};// end class App

struct App::KeymapEntry
{
    string                          command     = "";
    std::map<int,KeymapEntry>       children    = {};
};// end struct App::KeymapEntry

#endif
