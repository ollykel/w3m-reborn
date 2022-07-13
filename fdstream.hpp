#ifndef __FDSTREAM_HPP__
#define __FDSTREAM_HPP__

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

// class   ifdstream : public fdstream, public std::istream
// {
//     public:
//         // === public constructor(s) ======================================
//         ifdstream(void);// default
//         ifdstream(const int fd);// type
//     protected:
//         // === public member variable(s) ==================================
//         std::stack<char>        m_charBuf       = {};
// };// end class ifdstream

#endif
