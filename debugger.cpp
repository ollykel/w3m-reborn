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
    m_threshold = cfg.thresholdDefault;
}// end Debugger::Debugger

// --- public accessors ---------------------------------------------------
void Debugger::printf(int priority, const string& fmt, ...)
{
    using namespace std;

    va_list     ap;
    FILE        *out        = nullptr;

    if (not (out = fopen(m_filename.c_str(), "a")))
    {
        return;
    }

    va_start(ap, fmt);
    vfprintf(out, fmt.c_str(), ap);
    va_end(ap);

    fclose(out);
}// end Debugger::printf

auto Debugger::threshold(void)
    -> int
{
    return m_threshold;
}// end Debugger::threshold

// --- public mutators ----------------------------------------------------
void Debugger::set_threshold(int value)
{
    m_threshold = value;
}// end Debugger::set_threshold