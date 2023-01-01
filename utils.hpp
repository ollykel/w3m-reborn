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
    
    std::istream&       ignore_chars(
                            std::istream& ins,
                            const std::string& ignoreStr
                        );

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

    std::wstring        to_wstr(const std::string& str);
    std::string         from_wstr(const std::wstring& wstr);

    std::string         percent_encode(const std::string& str);
    std::string         percent_decode(const std::string& str);

    template <typename ITER_T>
    void ignore_whitespace(
        ITER_T& iter,
        const ITER_T& end
    );

    template <typename ITER_T>
    void ignore_chars(
        ITER_T& iter,
        const ITER_T& end,
        const std::string& ignoreStr
    );

    template <typename ITER_T>
    auto copy_token_until(
            ITER_T& iter,
            const ITER_T& end,
            const std::string& avoid,
            const bool escapeChars = false
        )
        -> std::string;

    template <typename ITER_T>
    auto copy_token_squoted(ITER_T& iter, const ITER_T& end)
        -> std::string;

    template <typename ITER_T>
    auto copy_token_dquoted(ITER_T& iter, const ITER_T& end)
        -> std::string;

    template <typename ITERABLE_T>
    auto join_str(const ITERABLE_T& cont, const std::string& joiner = "")
        -> std::string;

    template <typename CONTAINER_T>
    auto splitn(
            CONTAINER_T& dest,
            const std::string& str,
            const std::string& delims = " \t\r\n",
            const char escape = '\0',
            const size_t nSplits = 0
        )
        -> size_t;

    template <class DEST_CONT, class SRC_CONT, typename MAP_FUNC>
    void map(DEST_CONT& dest, const SRC_CONT& src, MAP_FUNC mapFunc);

    template <class CONT, typename OPERATION>
    void for_each(CONT& cont, OPERATION op);

    template <class STR_T>
    void to_lower(STR_T& str);

    template <class STR_T>
    void to_upper(STR_T& str);
};// end namespace utils

#include "utils.tpp"

#endif
