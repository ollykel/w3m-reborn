#ifndef __FDSTREAM_HPP__
#define __FDSTREAM_HPP__

#include <climits>
#include <cstdio>
#include <streambuf>
#include <ios>
#include <sstream>
#include <stack>

#include "deps.hpp"

class   fdstream_streambuf : public std::streambuf
{
    public:
        // === public constructor(s) ======================================

        // === Type Constructor ===========================================
        //
        // Creates a streambuf that reads from/writes to a given file
        // descriptor.
        // Assumes that the file descriptor is already open and has the
        // correct permissions set.
        //
        // Input:
        //      fd      [IN]    -- file descriptor
        //
        // ================================================================
        fdstream_streambuf(const int fd);// type
    protected:
        // === protected member variable(s) ===============================
        const int           m_fd            = -1;
        std::stack<int>     m_charStack     = {};
        std::vector<char>   m_buffer        = {};

        // === protected static constant(s) ===============================
        const size_t        C_BUFFER_INCREMENT   = 256;

        // === protected virtual member function(s) =======================
        virtual int underflow(void)
            override;
        virtual std::streamsize xsputn(const char *s, std::streamsize n)
            override;
};// end class fdstream_streambuf

class   ifdstream : public std::istream
{
    public:
        // === public constructor(s) ======================================

        // === Type Constructor ===========================================
        //
        // Initializes an ifdstream that wraps a given file descriptor.
        // Assumes the file descriptor is already open and has the correct
        // permissions set.
        //
        // Input:
        //      fd      [IN]    -- file descriptor
        //
        // ================================================================
        ifdstream(const int fd);

        // === public mutator(s) ==========================================
        void close(void);
    protected:
        // === protected member variable(s) ===============================
        int                         m_fd        = -1;
        u_ptr<std::streambuf>       m_bufPtr    = nullptr;
};// end class ifdstream

class   ofdstream2 : public std::ostream
{
    public:
        // === public constructor(s) ======================================

        // === Type Constructor ===========================================
        //
        // Initializes an ofdstream2 that wraps a given file descriptor.
        // Assumes the file descriptor is already open and has the correct
        // permissions set.
        //
        // Input:
        //      fd      [IN]    -- file descriptor
        //
        // ================================================================
        ofdstream2(const int fd);

        // === public mutator(s) ==========================================
        void close(void);
    protected:
        // === protected member variable(s) ===============================
        int                         m_fd        = -1;
        u_ptr<std::streambuf>       m_bufPtr    = nullptr;
};// end class ofdstream2

class   fdstream
{
    public:
        // === public accessor(s) =========================================
        operator bool(void) const;
        auto fd(void) const         -> int;
        auto fail(void) const       -> bool;
        auto opened(void) const     -> bool;
        auto closed(void) const     -> bool;

        // === public mutator(s) ==========================================
        void set_fd(const int fd);
        void close(void);
    protected:
        // === protected member variable(s) ===============================
        int     m_fd                = INT_MAX;
        bool    m_hasInitialized    = false;
        bool    m_hasFailed         = false;
        bool    m_hasClosed         = false;
};// end class fdstream

class   ofdstream : public fdstream
{
    public:
        // === public constructor(s) ======================================
        ofdstream(void);// default
        ofdstream(const int fd);// type

        // === public accessor(s) =========================================
        auto flags(void) const
            -> std::ios_base::fmtflags;
        auto width(void) const
            -> std::streamsize;
        auto precision(void) const
            -> std::streamsize;
        auto fill(void) const
            -> char;

        // === public mutator(s) ==========================================
        auto flags(std::ios_base::fmtflags flags)
            -> std::ios_base::fmtflags;
        auto setf(std::ios_base::fmtflags flags)
            -> std::ios_base::fmtflags;
        auto setf(std::ios_base::fmtflags flags, std::ios_base::fmtflags mask)
            -> std::ios_base::fmtflags;
        void unsetf(std::ios_base::fmtflags mask);
        auto width(std::streamsize w)
            -> std::streamsize;
        auto precision(std::streamsize p)
            -> std::streamsize;
        auto fill(char f)
            -> char;
        auto write(const char *s, size_t n)
            -> ssize_t;
        auto write(const std::vector<char>& s)
            -> ssize_t;
        auto write(const string& s)
            -> ssize_t;

        template <typename T>
        auto operator<<(const T& in)
            -> ofdstream&;
    protected:
        // === protected member variable(s) ===============================
        std::stringstream       m_formatter     = {};
};// end class ofdstream

#include "fdstream.tpp"

#endif
