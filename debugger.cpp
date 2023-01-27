#include <time.h>
#include <stdarg.h>

#include "deps.hpp"
#include "debugger.hpp"

// === class Debugger Implementation ======================================
//
// ========================================================================

// --- public constructors ------------------------------------------------
Debugger::Debugger(void)
{
    // do nothing
}// end Debugger::Debugger

Debugger::Debugger(const Config& cfg)
{
    m_filename = cfg.filename;
    m_limit = cfg.limitDefault;
    m_prefix = cfg.prefix;
    m_timeFormat = cfg.timeFormat;
}// end Debugger::Debugger

// --- public accessors ---------------------------------------------------
void Debugger::printf(int priority, const string& fmt, ...) const
{
    using namespace std;

    va_list     ap;
    FILE        *out        = nullptr;

    if (priority >= m_limit)
    {
        return;
    }

    if (not (out = fopen(m_filename.c_str(), "a")))
    {
        throw FileIOException(m_filename);
    }

    if (not m_prefix.empty())
    {
        fprintf(out, "[%s]:", m_prefix.c_str());
    }

    va_start(ap, fmt);
    vfprintf(out, fmt.c_str(), ap);
    va_end(ap);

    fputs("\n", out);// TODO: generalize newline with a macro

    fclose(out);
}// end Debugger::printf

auto Debugger::limit(void) const
    -> int
{
    return m_limit;
}// end Debugger::limit

auto Debugger::prefix(void) const
    -> const string&
{
    return m_prefix;
}// end Debugger::prefix

auto Debugger::time_format(void) const
    -> const string&
{
    return m_timeFormat;
}// end Debugger::time_format

auto Debugger::format_curr_time(void) const
    -> string
{
    #define     BUFFER_LEN_INIT     0x100

    using namespace std;

    time_t                  currTime        = time(nullptr);
    std::vector<char>       buffer(BUFFER_LEN_INIT, '\0');

    while (strftime(
            buffer.data(),
            buffer.size(),
            m_timeFormat.c_str(),
            localtime(&currTime)
        ) == buffer.size()
    )
    {
        buffer.resize(buffer.size() * 2);
    }// end while

    return string(buffer.data());
    #undef      BUFFER_LEN_INIT
}// end Debugger::format_curr_time

// --- public mutators ----------------------------------------------------
void Debugger::set_limit(int value)
{
    m_limit = value;
}// end Debugger::set_limit

void Debugger::set_prefix(const string& value)
{
    m_prefix = value;
}// end Debugger::set_prefix

void Debugger::set_time_format(const string& fmt)
{
    m_timeFormat = fmt;
}// end Debugger::set_time_format

// === class Debugger::FileIOException Implementation =====================
//
// ========================================================================

// --- private constructors -----------------------------------------------
Debugger::FileIOException::FileIOException(const string& fname)
{
    set_text("could not open \"" + fname + "\"");
}// end Debugger::FileIOException::FileIOException
