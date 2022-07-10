#include "sstream"

#include "deps.hpp"
#include "command_parser.hpp"

// === public constructor(s) ==============================================
CommandParser::CommandParser(void)
{
    // do nothing
}// end CommandParser::CommandParser(void)

// === public member function(s) ==========================================
auto CommandParser::parse_command(std::istream& ins) const
    -> std::vector<string>
{
    using namespace std;

    vector<string>      out     = {};

    return out;
}// end CommandParser::parse_command

auto CommandParser::parse_command(const string& str) const
    -> std::vector<string>
{
    std::istringstream      inBuf(str);

    return parse_command(inBuf);
}// end CommandParser::parse_command
