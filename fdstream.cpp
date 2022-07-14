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
    : m_fd(fd), m_buffer(0)
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

    setg(&m_buffer.front(), &m_buffer.at(oldBufSize), &m_buffer.back() + 1);

    return traits_type::to_int_type(m_buffer.at(oldBufSize));
}// end fdstream_streambuf::underflow

int fdstream_streambuf::overflow(int c)
{
    if (c != EOF)
    {
        char        ch      = c;

        ::write(m_fd, &ch, 1);
    }
    return c;
}// end fdstream_streambuf::overflow

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

// === class ofdstream Implementation =====================================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
ofdstream::ofdstream(const int fd)
{
    m_fd = fd;
    m_bufPtr = std::make_unique<fdstream_streambuf>(fd);
    rdbuf(m_bufPtr.get());
}// end 

// --- public mutator(s) --------------------------------------------------
void ofdstream::close(void)
{
    ::close(m_fd);
    setstate(eofbit);
}// end ofdstream::close
