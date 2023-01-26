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
            int         thresholdDefault;
        };// end struct Config

        // --- public constructors ----------------------------------------
        Debugger(void);
        Debugger(const Config& cfg);

        // --- public accessors -------------------------------------------
        void printf(int priority, const string& fmt, ...);
        auto threshold(void)
            -> int;

        // --- public mutators --------------------------------------------
        void set_threshold(int value);
    private:
        // --- private member variables -----------------------------------
        string      m_filename          = "";
        int         m_threshold         = 0;
};// end class Debugger

#endif
