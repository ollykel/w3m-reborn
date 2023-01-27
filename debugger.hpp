#ifndef __DEBUGGER_HPP__
#define __DEBUGGER_HPP__

#include "deps.hpp"

class   Debugger
{
    public:
        // --- public member types ----------------------------------------
        struct  Config
        {
            string      filename;
            int         limitDefault;
            string      prefix;
            string      timeFormat;
        };// end struct Config
        class   FileIOException : public StringException
        {
            friend class Debugger;

            private:
                // --- private constructors -------------------------------
                FileIOException(const string& fname);
        };// end class File

        // --- public constructors ----------------------------------------
        Debugger(void);
        Debugger(const Config& cfg);

        // --- public accessors -------------------------------------------
        void printf(int priority, const string& fmt, ...);
        auto limit(void)
            -> int;
        auto prefix(void)
            -> const string&;
        auto time_format(void)
            -> const string&;

        // --- public mutators --------------------------------------------
        void set_limit(int value);
        void set_prefix(const string& value);
        void set_time_format(const string& fmt);
    private:
        // --- private member variables -----------------------------------
        string      m_filename          = "";
        int         m_limit             = 0;
        string      m_prefix            = "";
        // default to RFC 2822-compliant date format
        string      m_timeFormat        = "%a, %d %b %Y %T %z";
};// end class Debugger

#endif
