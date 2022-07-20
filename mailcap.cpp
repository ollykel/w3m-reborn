#include <list>
#include <map>

#include "deps.hpp"
#include "mailcap.hpp"

// === class Mailcap Implementation =======================================
//
// ========================================================================

// --- public accessor(s) -------------------------------------------------
auto Mailcap::get_entry(
        const string& superType,
        const string& subType
    ) const
    -> const Entry*
{
    if (m_typeMap.count(superType))
    {
        const auto&     superTypeMap    = m_typeMap.at(superType);

        if (superTypeMap.count(subType))
        {
            for (const auto& entry : superTypeMap.at(subType))
            {
                if (entry.passes_test())
                {
                    return &entry;
                }
            }// end for entry
        }

        if (superTypeMap.count("*"))
        {
            for (const auto& entry : superTypeMap.at("*"))
            {
                if (entry.passes_test())
                {
                    return &entry;
                }
            }// end for entry
        }
    }

    if (m_typeMap.count("*"))
    {
        const auto&     superTypeMap    = m_typeMap.at("*");

        if (superTypeMap.count(subType))
        {
            for (const auto& entry : superTypeMap.at(subType))
            {
                if (entry.passes_test())
                {
                    return &entry;
                }
            }// end for entry
        }

        if (superTypeMap.count("*"))
        {
            for (const auto& entry : superTypeMap.at("*"))
            {
                if (entry.passes_test())
                {
                    return &entry;
                }
            }// end for entry
        }
    }

    // if we got here, no suitable command found
    return nullptr;
}// end Mailcap::get_entry

auto Mailcap::get_entry(
        const string& mimeType
    ) const
    -> const Entry*
{
    auto        mimePair        = split_mimetype(mimeType);

    return get_entry(mimePair.first, mimePair.second);
}// end Mailcap::get_entry

// --- public mutator(s) ------------------------------------------
void Mailcap::clear(
    const string& superType,
    const string& subType
)
{
    if (superType.empty())
    {
        m_typeMap.clear();
    }
    else
    {
        auto&   typeMap     = m_typeMap.at(superType);

        if (subType.empty())
        {
            typeMap.clear();
        }
        else
        {
            typeMap.at(subType).clear();
        }
    }
}// end Mailcap::clear

auto Mailcap::append_entry(
        const string& superType,
        const string& subType,
        const Entry& entry
    )
    -> Entry&
{
    if (superType.empty() or subType.empty())
    {
        throw std::logic_error(
            "mailcap entry must be indexed at non-empty types"
        );
    }

    auto&   entryCont   = m_typeMap[superType][subType];

    entryCont.emplace_front(entry);
    return entryCont.front();
}// end Mailcap::append_entry

auto Mailcap::append_entry(
        const string& mimeType,
        const Entry& entry
    )
    -> Entry&
{
    auto        mimePair    = split_mimetype(mimeType);

    return append_entry(mimePair.first, mimePair.second, entry);
}// end Mailcap::append_entry

// --- public static function(s) ------------------------------------------
auto Mailcap::split_mimetype(const string& mimeType)
    -> std::pair<string,string>
{
    std::pair<string,string>    out         = { "", "" };
    const size_t                splitIdx    = mimeType.find('/');

    if (mimeType.empty() or splitIdx == string::npos
        or splitIdx >= mimeType.length() - 1)
    {
        throw std::logic_error(
            "mimetype must be of form SUPERTYPE/SUBTYPE"
        );
    }

    out.first = mimeType.substr(0, splitIdx);
    out.second = mimeType.substr(splitIdx + 1);

    return out;
}// end Mailcap::split_mimetype

// === class Mailcap::Entry Implementation ================================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------

Mailcap::Entry::Entry(void)
{
    // do nothing
}// end Mailcap::Entry::Entry

Mailcap::Entry::Entry(const string& command)
    : Entry()
{
    set_command_template(command);
}// end Mailcap::Entry::Entry

// --- public accessor(s) -------------------------------------------------
auto Mailcap::Entry::command_template(void) const
    -> const command_template_container&
{
    return m_commandTemplate;
}// end Mailcap::Entry::command_template

auto Mailcap::Entry::filename_template(void) const
    -> const filename_template_container&
{
    return m_filenameTemplate;
}// end Mailcap::Entry::filename_template

auto Mailcap::Entry::test(void) const
    -> const Command&
{
    return m_test;
}// end Mailcap::Entry::test

auto Mailcap::Entry::has_test(void) const
    -> bool
{
    return not m_test.args().empty();
}// end Mailcap::Entry::has_test

auto Mailcap::Entry::passes_test(void) const
    -> bool
{
    if (not has_test())
    {
        return true;
    }

    auto    sproc   = m_test.spawn();

    sproc.stdin().close();

    while (sproc.stdout())
    {
        sproc.stdout().ignore(INT_MAX);
    }
    sproc.stdout().close();

    while (sproc.stderr())
    {
        sproc.stderr().ignore(INT_MAX);
    }
    sproc.stderr().close();

    return sproc.wait() == 0;
}// end Mailcap::Entry::passes_test

auto Mailcap::Entry::output_type(void) const
    -> const std::pair<string,string>&
{
    return m_outputType;
}// end Mailcap::Entry::output_type

auto Mailcap::Entry::output_type_str(void) const
    -> string
{
    if (m_outputType.first.empty() and m_outputType.second.empty())
    {
        return "";
    }
    return m_outputType.first + '/' + m_outputType.second;
}// end Mailcap::Entry::output_type_str

auto Mailcap::Entry::needs_terminal(void) const
    -> bool
{
    return m_needsTerminal;
}// end Mailcap::Entry::needs_terminal

auto Mailcap::Entry::file_piped(void) const
    -> bool
{
    return m_isFilePiped;
}// end Mailcap::Entry::file_piped

auto Mailcap::Entry::parse_filename(const string& fileBase) const
    -> string
{
    if (m_filenameTemplate.empty())
    {
        return fileBase;
    }

    string      output      = "";

    for (const auto& str : m_filenameTemplate)
    {
        if (str == "%s")
        {
            output += fileBase;
        }
        else
        {
            output += str;
        }
    }// end for str

    return output;
}// end Mailcap::Entry::parse_filename

auto Mailcap::Entry::create_command(
        const string& fileBase,
        const string& superType,
        const string& subType
    ) const
    -> Command
{
    const string    mimeType    = superType + '/' + subType;

    return create_command(fileBase, mimeType);
}// end Mailcap::Entry::create_command

auto Mailcap::Entry::create_command(
        const string& fileBase,
        const string& mimeType
    ) const
    -> Command
{
    const string    fileName        = parse_filename(fileBase);
    string          commandStr      = "";

    for (const auto& str : m_commandTemplate)
    {
        if (str == "%s")
        {
            commandStr += fileName;
        }
        else if (str == "%t")
        {
            commandStr += mimeType;
        }
        else
        {
            commandStr += str;
        }
    }// end for str

    return Command(commandStr)
        .set_stdin_piped(file_piped())
        .set_stdout_piped(not output_type_str().empty());
}// end Mailcap::Entry::create_command

// --- public mutator(s) --------------------------------------------------
auto Mailcap::Entry::set_command_template(const string& command)
    -> Entry&
{
    m_commandTemplate.clear();
    tokenize_fmt_string(m_commandTemplate, command);
    m_isFilePiped = true;
    for (const auto& str : m_commandTemplate)
    {
        if (str == "%s")
        {
            m_isFilePiped = false;
            break;
        }
    }// end for str
    return *this;
}// end Mailcap::Entry::set_command_template

auto Mailcap::Entry::set_filename_template(const string& filename)
    -> Entry&
{
    m_filenameTemplate.clear();
    tokenize_fmt_string(m_filenameTemplate, filename);
    return *this;
}// end Mailcap::Entry::set_filename_template

auto Mailcap::Entry::set_test(const string& testCommand)
    -> Entry&
{
    m_test = Command(testCommand, true, true, true);
    return *this;
}// end Mailcap::Entry::set_test

auto Mailcap::Entry::set_output_type(
        const string& superType,
        const string& subType
    )
    -> Entry&
{
    m_outputType.first = superType;
    m_outputType.second = subType;
    return *this;
}// end Mailcap::Entry::set_output_type

auto Mailcap::Entry::set_output_type(const string& mimeType)
    -> Entry&
{
    m_outputType = split_mimetype(mimeType);
    return *this;
}// end Mailcap::Entry::set_output_type

auto Mailcap::Entry::set_needs_terminal(const bool state)
    -> Entry&
{
    m_needsTerminal = state;
    return *this;
}// end Mailcap::Entry::set_needs_terminal