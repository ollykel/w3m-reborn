#include <unistd.h>
#include <climits>
#include <cctype>
#include <streambuf>
#include <stack>
#include <sstream>

#include "deps.hpp"
#include "fdstream.hpp"

// === class fdstream_streambuf Implementation ============================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
fdstream_streambuf::fdstream_streambuf(const int fd)
    : m_fd(fd), m_buffer(C_BUFFER_INCREMENT)
{
    // do nothing
}// end fdstream_streambuf::fdstream_streambuf

// --- protected virtual member function(s) -------------------------------
int fdstream_streambuf::underflow(void)
{
    const size_t        oldBufSize      = m_buffer.size();

    m_buffer.resize(oldBufSize + C_BUFFER_INCREMENT);
    m_buffer.resize(oldBufSize +
        ::read(m_fd, &m_buffer.at(oldBufSize), C_BUFFER_INCREMENT));

    if (m_buffer.size() == oldBufSize)
    {
        return traits_type::eof();
    }

    setg(&m_buffer.front(), &m_buffer.at(oldBufSize), &m_buffer.back());

    return traits_type::to_int_type(m_buffer.at(oldBufSize));
}// end fdstream_streambuf::underflow

std::streamsize fdstream_streambuf::xsputn(const char *s, std::streamsize n)
{
    return ::write(m_fd, s, n);
}// end fdstream_streambuf::xsputn

// === class ifdstream Implementation =====================================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
ifdstream::ifdstream(const int fd)
{
    m_fd = fd;
    m_bufPtr = std::make_unique<fdstream_streambuf>(fd);
    rdbuf(m_bufPtr.get());
}// end 

// --- public mutator(s) --------------------------------------------------
void ifdstream::close(void)
{
    ::close(m_fd);
    setstate(eofbit);
}// end ifdstream::close

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

// === class ofdstream Implementation =====================================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
ofdstream::ofdstream(void)
{
    // do nothing
}// end ofdstream::ofdstream

ofdstream::ofdstream(const int fd)
{
    set_fd(fd);
}// end ofdstream::ofdstream

// --- public accessor(s) -------------------------------------------------
auto ofdstream::flags(void) const
    -> std::ios_base::fmtflags
{
    return m_formatter.flags();
}// end ofdstream::flags

auto ofdstream::width(void) const
    -> std::streamsize
{
    return m_formatter.width();
}// end ofdstream::width

auto ofdstream::precision(void) const
    -> std::streamsize
{
    return m_formatter.precision();
}// end ofdstream::precision

auto ofdstream::fill(void) const
    -> char
{
    return m_formatter.fill();
}// end ofdstream::fill

// --- public mutator(s) --------------------------------------------------
auto ofdstream::flags(std::ios_base::fmtflags flags)
    -> std::ios_base::fmtflags
{
    return m_formatter.flags(flags);
}// end ofdstream::flags

auto ofdstream::setf(std::ios_base::fmtflags flags)
    -> std::ios_base::fmtflags
{
    return m_formatter.setf(flags);
}// end ofdstream::setf

auto ofdstream::setf(std::ios_base::fmtflags flags, std::ios_base::fmtflags mask)
    -> std::ios_base::fmtflags
{
    return m_formatter.setf(flags, mask);
}// end ofdstream::setf

void ofdstream::unsetf(std::ios_base::fmtflags mask)
{
    m_formatter.unsetf(mask);
}// end ofdstream::unsetf

auto ofdstream::width(std::streamsize w)
    -> std::streamsize
{
    return m_formatter.width(w);
}// end ofdstream::width

auto ofdstream::precision(std::streamsize p)
    -> std::streamsize
{
    return m_formatter.precision(p);
}// end ofdstream::precision

auto ofdstream::fill(char f)
    -> char
{
    return m_formatter.fill(f);
}// end ofdstream::fill

auto ofdstream::write(const char *s, size_t n)
    -> ssize_t
{
    return ::write(fd(), s, n);
}// end ofdstream::write

auto ofdstream::write(const std::vector<char>& s)
    -> ssize_t
{
    return write(&s[0], s.size());
}// end ofdstream::write

auto ofdstream::write(const string& s)
    -> ssize_t
{
    return write(s.c_str(), s.length());
}// end ofdstream::write
