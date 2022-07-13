#include <unistd.h>
#include <climits>
#include <stack>

#include "deps.hpp"
#include "fdstream.hpp"

// === class fdstream Implementation ======================================
//
// ========================================================================

// ------ public accessor(s) ----------------------------------------------
fdstream::operator bool(void) const
{
    return m_hasInitialized and not m_hasFailed;
}// end fdstream::operator bool

auto fdstream::fd(void) const     -> int
{
    return m_fd;
}// end fdstream::fd

auto fdstream::fail(void) const   -> bool
{
    return m_hasFailed;
}// end fdstream::fail

auto fdstream::opened(void) const     -> bool
{
    return m_hasInitialized and not m_hasClosed;
}// end fdstream::opened

auto fdstream::closed(void) const     -> bool
{
    return m_hasInitialized and m_hasClosed;
}// end fdstream::closed

// ------ public mutator(s) -----------------------------------------------
void fdstream::set_fd(const int fd)
{
    if (opened())
    {
        throw std::logic_error("attempting to assign new fd to open fdstream");
    }

    m_fd = fd;
    m_hasInitialized = true;
}// end fdstream::set_fd

void fdstream::close(void)
{
    if (not opened())
    {
        throw std::logic_error("attempting to close unopened fdstream");
    }

    ::close(m_fd);
    m_hasClosed = true;
}// end fdstream::close
