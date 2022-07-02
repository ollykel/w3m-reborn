#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <iostream>
#include <string>

namespace utils
{
    // === utils::ignore_whitespace(std::istream& ins) ====================
    //
    // Ignores chars in an istream until either the stream is empty or the
    // stream comes to a non-whitespace character. Does not extract the
    // first non-whitespace character.
    //
    // Input:
    //      ins     [IN]    -- stream to read from
    //
    // Output:
    //      stream that was passed in <in>
    //
    // ====================================================================
    std::istream&       ignore_whitespace(std::istream& ins);

    // === read_token_until ===============================================
    //
    // Reads characters from input stream <ins> until one of the characters
    // in the string <avoid> is encountered. If <inclusive> is true, the
    // encountered character is extracted and appended to the token;
    // otherwise, it is left in the stream.
    //
    // Input:
    //      ins     [IN]        -- input stream to read from
    //      avoid   [IN]        -- string of characters to avoid
    //      inclusive   [IN]    -- whether or not to append delimiter
    //
    // Output:
    //      token extracted from <ins>
    //
    // ====================================================================
    std::string         read_token_until(
                            std::istream& ins,
                            const std::string& avoid,
                            const bool inclusive = false
                        );
    std::string         read_token_snake_case(std::istream& ins);
    std::string         read_token_squoted(std::istream& ins);
    std::string         read_token_dquoted(std::istream& ins);
    std::string         read_token_to_pattern(
                            std::istream& ins,
                            const char *pattern
                        );
    std::string         path_base(
                            const std::string& str,
                            const char pathSep = '/'
                        );
};// end namespace utils

#endif
