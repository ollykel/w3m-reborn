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
}// end Debugger::Debugger

// --- public accessors ---------------------------------------------------
void Debugger::printf(int priority, const string& fmt, ...)
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
        return;
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

auto Debugger::limit(void)
    -> int
{
    return m_limit;
}// end Debugger::limit

auto Debugger::prefix(void)
    -> const string&
{
    return m_prefix;
}// end Debugger::prefix

// --- public mutators ----------------------------------------------------
void Debugger::set_limit(int value)
{
    m_limit = value;
}// end Debugger::set_limit

void Debugger::set_prefix(const string& value)
{
    m_prefix = value;
}// end Debugger::set_prefix
