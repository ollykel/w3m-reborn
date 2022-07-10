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

    while (ins)
    {
        utils::ignore_chars(ins, " \t");

        if (not ins)
            break;

        switch (ins.peek())
        {
            case '\n':
            case '\r':
            case ';':
                return out;
            case '"':
                out.push_back(utils::read_token_dquoted(ins));
                break;
            case '\'':
                out.push_back(utils::read_token_squoted(ins));
                break;
            default:
                out.emplace_back();
                ins >> out.back();
                break;
        }// end switch (ins.peek())
    }// end while ins

    return out;
}// end CommandParser::parse_command

auto CommandParser::parse_command(const string& str) const
    -> std::vector<string>
{
    std::istringstream      inBuf(str);

    return parse_command(inBuf);
}// end CommandParser::parse_command
