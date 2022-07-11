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

    vector<string>      out     = { "" };

    utils::ignore_chars(ins, " \t");
    while (ins)
    {
        if (not ins)
            break;

        switch (ins.peek())
        {
            case EOF:
            case '\n':
            case '\r':
            case ';':
                ins.ignore(1);
                goto finally;
            case '#':
                utils::read_token_until(ins, "\r\n");
                break;
            case ' ':
            case '\t':
                out.emplace_back();
                utils::ignore_chars(ins, " \t");
                break;
            case '"':
                out.back() += utils::read_token_dquoted(ins);
                break;
            case '\'':
                out.back() += utils::read_token_squoted(ins);
                break;
            default:
                out.back() += utils::read_token_until(ins, " \r\n;#'\"");
                break;
        }// end switch (ins.peek())
    }// end while ins

finally:
    if (out.back().empty())
    {
        out.pop_back();
    }

    return out;
}// end CommandParser::parse_command

auto CommandParser::parse_command(const string& str) const
    -> std::vector<string>
{
    std::istringstream      inBuf(str);

    return parse_command(inBuf);
}// end CommandParser::parse_command
