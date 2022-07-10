#ifndef __COMMAND_PARSER_HPP__
#define __COMMAND_PARSER_HPP__

#include "deps.hpp"

// === class CommandParser ================================================
//
// ========================================================================
class   CommandParser
{
    public:
        // === public constructor(s) ======================================
        CommandParser(void);// default

        // === public member function(s) ==================================
        auto parse_command(std::istream& ins) const -> std::vector<string>;
        auto parse_command(const string& str) const -> std::vector<string>;
    protected:
};// end class CommandParser

#endif
