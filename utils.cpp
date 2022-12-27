#include <climits>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <string>
#include <cctype>
#include <set>

#include "utils.hpp"

namespace utils
{

std::istream&       ignore_whitespace(std::istream& ins)
{
    while (ins and std::isspace(ins.peek()))
    {
        ins.ignore(1);
    }
    return ins;
}// end ignore_whitespace(std::istream& ins)

std::istream&       ignore_chars(
    std::istream& ins,
    const std::string& ignoreStr
)
{
    using namespace std;

    set<char>   ignoreSet(ignoreStr.cbegin(), ignoreStr.cend());

    while (ins and ignoreSet.count(ins.peek()))
    {
        ins.ignore(1);
    }// end while

    return ins;
}// end ignore_chars

std::string         read_token_until(
    std::istream& ins,
    const std::string& avoid,
    const bool inclusive
)
{
    using namespace std;

    string      output                  = "";
    set<char>   avoidSet(avoid.cbegin(), avoid.cend());

    while (ins.peek() != EOF and not avoidSet.count(ins.peek()))
    {
        output += ins.get();
    }// end while (ins && !avoidSet[ins.peek()])

    if (ins.peek() != EOF and inclusive)
    {
        output += ins.get();
    }

    return output;
}// end read_token_until

// === read_token_snake_case(std::istream& ins) ====================
//
// ========================================================================
std::string         read_token_snake_case(std::istream& ins)
{
    using namespace std;

    string      output      = "";

    while (ins)
    {
        const char      curr        = ins.peek();

        switch (curr)
        {
            case '_':
            case '-':
            case ':':
                output += ins.get();
                break;
            default:
                if (isalnum(curr))
                    output += ins.get();
                else
                    return output;
        }// end switch (ins.peek())
    }// end while (ins)

    return output;
}// end read_token_snake_case(std::istream& ins)

// === read_token_squoted(std::istream& ins) =======================
//
// ========================================================================
std::string         read_token_squoted(std::istream& ins)
{
    using namespace std;

    string      output      = "";

    // Weird Case: doesn't actually begin with a single quote
    if (!ins || ins.peek() != '\'')
        return output;
    
    ins.ignore(1);

    output = read_token_until(ins, "'");

    if (ins)
        ins.ignore(1);

    return output;
}// end read_token_squoted(std::istream& ins)

// === read_token_dquoted(std::istream& ins) =======================
//
// ========================================================================
std::string         read_token_dquoted(std::istream& ins)
{
    using namespace std;

    string      output      = "";

    // Weird case: initial char not a double quote
    if (!ins || ins.peek() != '"')
        return output;

    ins.ignore(1);

    while (ins && ins.peek() != '"')
    {
        output += read_token_until(ins, "\"\\");
        if (ins && ins.peek() == '\\')
            output += ins.get();
    }// end while (ins && ins.peek() != '"')

    if (ins)
        ins.ignore();// ignore terminal dquote

    return output;
}// end read_token_dquoted(std::istream& ins)

// === read_token_to_pattern =======================================
//
// TODO: fix implementation
//
// ========================================================================
std::string         read_token_to_pattern(
    std::istream& ins,
    const char *pattern
)
{
    using namespace std;

    string      output          = "";
    char        sentinel[2]     = { pattern[0], '\0' };
    const char  *match          = pattern;

    if (!pattern[0])
    {
        return output;
    }

    while (ins && *match)
    {
        match = pattern;

        output += read_token_until(ins, sentinel);

        for (char curr = ins.get(); ins && *match && curr == *match;
            curr = ins.get())
        {
            output += curr;
            ++match;
            if (!*match)
                break;
        }// end for
    }// end while

    return output;
}// end read_token_to_pattern

std::string     path_base(const std::string& str, const char pathSep)
{
    size_t      lastPos     = str.rfind(pathSep);

    if (lastPos == std::string::npos)
    {
        return str.substr(0, str.length());
    }
    else
    {
        ++lastPos;
        return str.substr(lastPos, str.length() - lastPos);
    }
}// end path_base

std::wstring    to_wstr(const std::string& str)
{
    return std::wstring(str.cbegin(), str.cend());
}// end to_wstr

std::string     from_wstr(const std::wstring& wstr)
{
    return std::string(wstr.cbegin(), wstr.cend());
}// end from_wstr

std::string     percent_encode(const std::string& str)
{
    std::string     out    = "";

    for (const char& ch : str)
    {
        if (std::isalnum(ch))
        {
            out.push_back(ch);
        }
        else
        {
            char    buffer[0x08]    = {};

            std::sprintf(buffer, "%%%02x", ch);
            out += buffer;
        }
    }// end for

    return out;
}// end percent_encode

};// end namespace utils
