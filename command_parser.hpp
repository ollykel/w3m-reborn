#ifndef __COMMAND_PARSER_HPP__
#define __COMMAND_PARSER_HPP__

#include <map>

#include "deps.hpp"

// === class CommandParser ================================================
//
// ========================================================================
class   CommandParser
{
    public:
        // === public member variable(s) ==================================
        std::map<string,string>     variables       = {};

        // === public constructor(s) ======================================
        CommandParser(void);// default

        // === public member function(s) ==================================

        // === parse_command ==============================================
        //
        // Reads a single command from an input stream (or string), and
        // outputs the command as a vector of tokens.
        //
        // ================================================================
        auto parse_command(std::istream& ins) const -> std::vector<string>;
        auto parse_command(const string& str) const -> std::vector<string>;
    protected:
};// end class CommandParser

#endif
