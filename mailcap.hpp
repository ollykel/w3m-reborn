#ifndef __MAILCAP_HPP__
#define __MAILCAP_HPP__

#include <list>
#include <map>

#include "deps.hpp"
#include "command.hpp"

class Mailcap
{
    public:
        // --- public member type(s) --------------------------------------
        class Entry;

        // --- public accessor(s) -----------------------------------------
        auto get_entry(
                const string& superType,
                const string& subType,
                const string& filenameBase
            ) const
            -> string;
        auto get_entry(
                const string& mimeType,
                const string& filenameBase
            ) const
            -> string;
        // --- public mutator(s) ------------------------------------------
        void clear(
            const string& superType = "",
            const string& subType = ""
        );
        auto append_entry(
                const string& superType,
                const string& subType,
                const Entry& entry
            )
            -> Entry&;
        auto append_entry(
                const string& mimeType,
                const Entry& entry
            )
            -> Entry&;
    protected:
        // --- protected member type(s) -----------------------------------
        typedef     std::list<Entry>                    entry_container;
        typedef     std::map<string,entry_container>    subtype_map;
        typedef     std::map<string,subtype_map>        supertype_map;

        // --- protected member variable(s) -------------------------------
        supertype_map       m_typeMap       = {};
};// end class Mailcap

class Mailcap::Entry
{
    public:
        // --- public constructor(s) --------------------------------------
        Entry(void);
        Entry(const string& command);

        // --- public accessor(s) -----------------------------------------
        auto command_template(void) const
            -> string;
        auto filename_template(void) const
            -> string;
        auto test(void) const
            -> const string&;
        auto output_type(void) const
            -> const string&;
        auto needs_terminal(void) const
            -> bool;

        // --- public mutator(s) ------------------------------------------
        auto set_command_template(const string& command)
            -> Entry&;
        auto set_filename_template(const string& filename)
            -> Entry&;
        auto set_test(const string& testCommand)
            -> Entry&;
        auto set_output_type(const string& mimeType)
            -> Entry&;
        auto set_needs_terminal(const bool state)
            -> Entry&;
    protected:
        // --- protected member type(s) -----------------------------------
        typedef     std::vector<string>     command_template_container;
        typedef     std::vector<string>     filename_template_container;

        // --- protected member variable(s) -------------------------------
        command_template_container      m_commandTemplate       = {};
        filename_template_container     m_filenameTemplate      = {};
        Command                         m_test                  = {};
        string                          m_outputType            = "";
        bool                            m_needsTerminal         = false;
};// end class Mailcap::Entry

#endif
