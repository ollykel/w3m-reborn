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
        class except_invalid_entry;

        // --- public accessor(s) -----------------------------------------
        auto get_entry(
                const string& superType,
                const string& subType
            ) const
            -> const Entry*;
        auto get_entry(
                const string& mimeType
            ) const
            -> const Entry*;
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
        auto parse_entry(const string& entryStr)
            -> Entry&;

        // --- public static function(s) ----------------------------------
        static auto split_mimetype(const string& mimeType)
            -> std::pair<string,string>;
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
        // --- public member type(s) --------------------------------------
        typedef     std::vector<string>     command_template_container;
        typedef     std::vector<string>     filename_template_container;

        // --- public constructor(s) --------------------------------------
        Entry(void);
        Entry(const string& command);

        // --- public accessor(s) -----------------------------------------
        auto command_template(void) const
            -> const command_template_container&;
        auto filename_template(void) const
            -> const filename_template_container&;
        auto test(void) const
            -> const Command&;
        auto has_test(void) const
            -> bool;
        auto passes_test(void) const
            -> bool;
        auto output_type(void) const
            -> const std::pair<string,string>&;
        auto output_type_str(void) const
            -> string;
        auto needs_terminal(void) const
            -> bool;
        auto file_piped(void) const
            -> bool;
        auto parse_filename(const string& fileBase) const
            -> string;
        auto create_command(
                const string& fileBase,
                const string& superType,
                const string& subType
            ) const
            -> Command;
        auto create_command(
                const string& fileBase,
                const string& mimeType
            ) const
            -> Command;

        // --- public mutator(s) ------------------------------------------
        auto set_command_template(const string& command)
            -> Entry&;
        auto set_filename_template(const string& filename)
            -> Entry&;
        auto set_test(const string& testCommand)
            -> Entry&;
        auto set_output_type(
                const string& superType,
                const string& subType
            )
            -> Entry&;
        auto set_output_type(const string& mimeType)
            -> Entry&;
        auto set_needs_terminal(const bool state)
            -> Entry&;
    protected:
        // --- protected member variable(s) -------------------------------
        command_template_container      m_commandTemplate       = {};
        filename_template_container     m_filenameTemplate      = {};
        Command                         m_test                  = {};
        std::pair<string,string>        m_outputType            = {};
        bool                            m_needsTerminal         = false;
        bool                            m_isFilePiped           = false;

        // --- protected static function(s) -------------------------------
        template <typename CONTAINER_T>
        static void tokenize_fmt_string(CONTAINER_T& dest, const string& str);
};// end class Mailcap::Entry

class Mailcap::except_invalid_entry : public StringException
{
    public:
        // --- public constructors ----------------------------------------
        except_invalid_entry(const string& what);
};// end class Mailcap::except_invalid_entry

#include "mailcap.tpp"

#endif
