#include <climits>
#include <iostream>
#include <string>
#include <cctype>

#include "utils.hpp"

std::istream&       utils::ignore_whitespace(std::istream& ins)
{
    while (ins && std::isspace(ins.peek()))
    {
        ins.ignore(1);
    }
    return ins;
}// end utils::ignore_whitespace(std::istream& ins)

std::string         utils::read_token_until(
    std::istream& ins,
    const char *avoid,
    const bool inclusive
)
{
    using namespace std;

    string      output                  = "";
    bool        avoidSet[UCHAR_MAX]     = {};

    for (const char *curr = avoid; *curr; ++curr)
    {
        if (*curr < 0)
            avoidSet[static_cast<int>(*curr) + UCHAR_MAX] = true;
        else
            avoidSet[static_cast<int>(*curr)] = true;
    }// end for (const char *curr = avoid; *curr; ++curr)

    while (ins && !avoidSet[ins.peek()])
    {
        output += ins.get();
    }// end while (ins && !avoidSet[ins.peek()])

    if (ins && inclusive)
        output += ins.get();

    return output;
}// end utils::read_token_until

// === utils::read_token_snake_case(std::istream& ins) ====================
//
// ========================================================================
std::string         utils::read_token_snake_case(std::istream& ins)
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
}// end utils::read_token_snake_case(std::istream& ins)

// === utils::read_token_squoted(std::istream& ins) =======================
//
// ========================================================================
std::string         utils::read_token_squoted(std::istream& ins)
{
    using namespace std;

    string      output      = "";

    // Weird Case: doesn't actually begin with a single quote
    if (!ins || ins.peek() != '\'')
        return output;
    
    ins.ignore(1);

    output = utils::read_token_until(ins, "'");

    if (ins)
        ins.ignore(1);

    return output;
}// end utils::read_token_squoted(std::istream& ins)

// === utils::read_token_dquoted(std::istream& ins) =======================
//
// ========================================================================
std::string         utils::read_token_dquoted(std::istream& ins)
{
    using namespace std;

    string      output      = "";

    // Weird case: initial char not a double quote
    if (!ins || ins.peek() != '"')
        return output;

    ins.ignore(1);

    while (ins && ins.peek() != '"')
    {
        output += utils::read_token_until(ins, "\"\\");
        if (ins && ins.peek() == '\\')
            output += ins.get();
    }// end while (ins && ins.peek() != '"')

    if (ins)
        ins.ignore();// ignore terminal dquote

    return output;
}// end utils::read_token_dquoted(std::istream& ins)

// === utils::read_token_to_pattern =======================================
//
// TODO: fix implementation
//
// ========================================================================
std::string         utils::read_token_to_pattern(
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

        output += utils::read_token_until(ins, sentinel);

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
}// end utils::read_token_to_pattern
