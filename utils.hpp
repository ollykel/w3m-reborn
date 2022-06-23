#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <iostream>
#include <string>

namespace utils
{
    std::istream&       ignore_whitespace(std::istream& ins);
    std::string         read_token_until(
                            std::istream& ins,
                            const char *avoid,
                            const bool inclusive = false
                        );
    std::string         read_token_snake_case(std::istream& ins);
    std::string         read_token_squoted(std::istream& ins);
    std::string         read_token_dquoted(std::istream& ins);
};

#endif
