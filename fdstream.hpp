#ifndef __FDSTREAM_HPP__
#define __FDSTREAM_HPP__

#include <climits>
#include <cstdio>
#include <ios>
#include <sstream>
#include <stack>

#include "deps.hpp"

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

class   ifdstream : public fdstream
{
    public:
        // === public constructor(s) ======================================
        ifdstream(void);// default
        ifdstream(const int fd);// type

        // === public accessor(s) =========================================
        operator bool(void) const;
        auto eof(void) const
            -> bool;

        // === public mutator(s) ==========================================
        auto get(void)
            -> int;
        auto get(char& c)
            -> ifdstream&;
        auto get(char *s, size_t n, char delim = EOF)
            -> ifdstream&;
        auto get(string& s, size_t n = 0, char delim = EOF)
            -> ifdstream&;
        auto getline(string& s)
            -> ifdstream&;
        auto read(char *buf, size_t n)
            -> ssize_t;
        auto read(std::vector<char>& buf, size_t n)
            -> ssize_t;
        auto peek(void)
            -> int;
        auto putback(char c)
            -> ifdstream&;
        void ignore(size_t n);

        template <typename T>
        auto operator>>(T& out)
            -> ifdstream&;
    protected:
        // === public member variable(s) ==================================
        std::stack<int>         m_charBuf       = {};
        bool                    m_eof           = false;
};// end class ifdstream

class   ofdstream : public fdstream, public std::ostream
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
