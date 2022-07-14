#include <unistd.h>
#include <climits>
#include <cctype>
#include <stack>
#include <sstream>

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

// === class ifdstream Implementation =====================================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
ifdstream::ifdstream(void)
{
    // do nothing
}// end ifdstream::ifdstream

ifdstream::ifdstream(const int fd)
{
    set_fd(fd);
}// end ifdstream::ifdstream

// --- public accessor(s) -------------------------------------------------
ifdstream::operator bool(void) const
{
    return opened() and not closed() and not eof();
}// end ifdstream::operator bool

auto ifdstream::eof(void) const
    -> bool
{
    return m_eof;
}// end ifdstream::eof

// --- public mutator(s) --------------------------------------------------
auto ifdstream::get(void)
    -> int
{
    if (not opened() or closed())
    {
        throw std::logic_error("attempting to read from unopened ifdstream");
    }
    else if (eof())
    {
        return EOF;
    }
    else
    {
        int     out     = 0;

        if (not m_charBuf.empty())
        {
            out = m_charBuf.top();
            m_charBuf.pop();
        }
        else
        {
            char        c       = 0;

            if (not ::read(fd(), &c, 1))
                c = EOF;

            out = c;
        }

        if (out == EOF)
            m_eof = true;

        return out;
    }
}// end ifdstream::get

auto ifdstream::get(char& c)
    -> ifdstream&
{
    if (not opened() or closed())
    {
        throw std::logic_error("attempting to read from unopened ifdstream");
    }
    else if (eof())
    {
        c = EOF;
    }
    else if (not m_charBuf.empty())
    {
        c = m_charBuf.top();
        m_charBuf.pop();
    }
    else
    {
        if (not ::read(fd(), &c, 1))
            c = EOF;
    }

    if (c == EOF)
        m_eof = true;

    return *this;
}// end ifdstream::get

auto ifdstream::get(char *s, size_t n, char delim)
    -> ifdstream&
{
    if (not opened() or closed())
    {
        throw std::logic_error("attempting to read from unopened ifdstream");
    }
    else if (eof())
    {
        s[0] = '\0';
    }
    else
    {
        size_t      idx     = 0;

        // first, 
        while (idx < n - 1 and not m_charBuf.empty())
        {
            if (m_charBuf.top() == EOF)
            {
                m_eof = true;
                s[idx] = '\0';
                goto finally;
            }
            else if (m_charBuf.top() == delim)
            {
                m_charBuf.pop();
                s[idx] = '\0';
                goto finally;
            }
            else
            {
                s[idx++] = m_charBuf.top();
                m_charBuf.pop();
            }
        }// end while

        while (idx < n - 1)
        {
            char        c       = 0;

            if (not ::read(fd(), &c, 1))
                c = EOF;

            if (c == EOF)
            {
                s[idx] = '\0';
                m_eof = true;
                goto finally;
            }
            else if (c == delim)
            {
                s[idx] = '\0';
                goto finally;
            }
            else
            {
                s[idx++] = c;
            }
        }// end while
        
        s[idx] = '\0';
    }
finally:
    return *this;
}// end ifdstream::get

auto ifdstream::get(string& s, size_t n, char delim)
    -> ifdstream&
{
    if (not opened() or closed())
    {
        throw std::logic_error("attempting to read from unopened ifdstream");
    }
    else if (eof())
    {
        // do nothing
    }
    else
    {
        size_t      idx     = 0;

        // first, 
        while ((not n or idx < n - 1) and not m_charBuf.empty())
        {
            if (m_charBuf.top() == EOF)
            {
                m_eof = true;
                goto finally;
            }
            else if (m_charBuf.top() == delim)
            {
                m_charBuf.pop();
                goto finally;
            }
            else
            {
                s += m_charBuf.top();
                ++idx;
                m_charBuf.pop();
            }
        }// end while

        while (not n or idx < n - 1)
        {
            char        c       = 0;

            if (not ::read(fd(), &c, 1))
                c = EOF;

            if (c == EOF)
            {
                m_eof = true;
                goto finally;
            }
            else if (c == delim)
            {
                goto finally;
            }
            else
            {
                s += c;
                ++idx;
            }
        }// end while
    }
finally:
    return *this;
}// end ifdstream::get

auto ifdstream::getline(string& s)
    -> ifdstream&
{
    return get(s, 0, '\n');
}

auto ifdstream::read(char *buf, size_t n)
    -> ssize_t
{
    ssize_t idx = 0;

    while (idx < n and not m_charBuf.empty())
    {
        buf[idx++] = m_charBuf.top();
        m_charBuf.pop();
    }// end while

    if (idx == n)
        return idx;

    idx += ::read(fd(), buf + idx, n - idx);

    if (idx < n)
        m_eof = true;

    return idx;
}// end ifdstream::read

auto ifdstream::read(std::vector<char>& buf, size_t n)
    -> ssize_t
{
    size_t      idx     = buf.size();
    ssize_t     count   = 0;

    buf.resize(buf.size() + n);

    while (count < n and not m_charBuf.empty())
    {
        buf[idx++] = m_charBuf.top();
        m_charBuf.pop();
        ++count;
    }// end while

    if (count == n)
        return n;

    count += ::read(fd(), &buf[idx], n - count);

    if (count < n)
        m_eof = true;

    return count;
}// end ifdstream::read

auto ifdstream::peek(void)
    -> int
{
    if (eof())
    {
        return EOF;
    }
    else if (m_charBuf.empty())
    {
        char        c       = 0;

        ::read(fd(), &c, 1);
        m_charBuf.push(c);
    }

    return m_charBuf.top();
}// end ifdstream::peek

auto ifdstream::putback(char c)
    -> ifdstream&
{
    m_charBuf.push(c);
    m_eof = false;

    return *this;
}// end ifdstream::putback

void ifdstream::ignore(size_t n)
{
    while (n and not m_charBuf.empty())
    {
        if (m_charBuf.top() == EOF)
        {
            m_eof = true;
            return;
        }
        m_charBuf.pop();
        --n;
    }// end while
    
    if (not n)
        return;

    std::vector<char>       buf(n);

    if (::read(fd(), &buf[0], n) < n)
    {
        m_eof = true;
    }
}// end ifdstream::ignore

template <typename T>
auto ifdstream::operator>>(T& out)
    -> ifdstream&
{
    using namespace std;

    string      buf     = "";

    while (not eof() and isspace(peek()))
    {
        ignore(1);
    }// end while

    while (not eof() and not isspace(peek()))
    {
        buf += get();
    }// end while

    istringstream       stream(buf);

    stream >> out;

    return *this;
}// end ifdstream::operator>>
