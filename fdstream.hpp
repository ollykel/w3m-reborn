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
        virtual int overflow(int c = EOF)
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

class   ofdstream : public std::ostream
{
    public:
        // === public constructor(s) ======================================

        // === Type Constructor ===========================================
        //
        // Initializes an ofdstream that wraps a given file descriptor.
        // Assumes the file descriptor is already open and has the correct
        // permissions set.
        //
        // Input:
        //      fd      [IN]    -- file descriptor
        //
        // ================================================================
        ofdstream(const int fd);

        // === public mutator(s) ==========================================
        void close(void);
    protected:
        // === protected member variable(s) ===============================
        int                         m_fd        = -1;
        u_ptr<std::streambuf>       m_bufPtr    = nullptr;
};// end class ofdstream

#endif
