#include <cctype>
#include <algorithm>
#include <unordered_set>

namespace utils
{

template <typename ITER_T>
void ignore_whitespace(
    ITER_T& iter,
    const ITER_T& end
)
{
    while (iter != end and std::isspace(*iter))
    {
        ++iter;
    }// end while
}// end ignore_whitespace

template <typename ITER_T>
void ignore_chars(
    ITER_T& iter,
    const ITER_T& end,
    const std::string& ignoreStr
)
{
    using namespace std;

    unordered_set<char>     ignoreSet(ignoreStr.cbegin(), ignoreStr.cend());

    while (iter != end and ignoreSet.count(*iter))
    {
        ++iter;
    }// end while
}// end ignore_chars

template <typename ITER_T>
auto copy_token_until(
        ITER_T& iter,
        const ITER_T& end,
        const std::string& avoid,
        const bool escapeChars
    )
    -> std::string
{
    using namespace std;

    string                  output                  = "";
    unordered_set<char>     avoidSet(avoid.cbegin(), avoid.cend());

    while (iter != end)
    {
        if (escapeChars and *iter == '\\')
        {
            ++iter;
            if (iter == end)
                break;
        }
        else if (avoidSet.count(*iter))
        {
            break;
        }
        output += *iter;
        ++iter;
    }// end while

    return output;
}// end copy_token_until

template <typename ITER_T>
auto copy_token_squoted(ITER_T& iter, const ITER_T& end)
    -> std::string
{
    using namespace std;

    string      output      = "";

    // Weird Case: doesn't actually begin with a single quote
    if (iter == end or *iter != '\'')
    {
        return output;
    }
    
    ++iter;

    output = copy_token_until(iter, end, "'");

    if (iter != end)
    {
        ++iter;
    }

    return output;
}// end copy_token_squoted

template <typename ITER_T>
auto copy_token_dquoted(ITER_T& iter, const ITER_T& end)
    -> std::string
{
    using namespace std;

    string      output      = "";

    // Weird case: initial char not a double quote
    if (iter != end or *iter != '"')
    {
        return output;
    }

    ++iter;

    while (iter != end and *iter != '"')
    {
        output += copy_token_until(iter, end, "\"\\");
        if (iter != end and *iter == '\\')
        {
            output += *iter;
            ++iter;
        }
    }// end while (ins && ins.peek() != '"')

    // ignore terminal quote
    if (iter != end)
    {
        ++iter;
    }

    return output;
}// end copy_token_dquoted

template <typename ITERABLE_T>
auto join_str(const ITERABLE_T& cont, const std::string& joiner)
    -> std::string
{
    std::string     output      = "";
    auto            iter        = cont.cbegin();
    const auto      end         = cont.cend();

    if (iter == end)
    {
        goto finally;
    }

    output += *iter;
    ++iter;

    for (; iter != end; ++iter)
    {
        output += joiner;
        output += *iter;
    }// end for iter
finally:
    return output;
}// end join_str

template <typename CONTAINER_T>
auto splitn(
        CONTAINER_T& dest,
        const std::string& str,
        const std::string& delims,
        const char escape,
        const size_t nSplits
    )
    -> size_t
{
    using namespace std;

    unordered_set<char>     delimSet(delims.begin(), delims.end());
    size_t                  count           = 0;
    string                  token           = "";
    bool                    toEscape        = false;

    for (const char ch : str)
    {
        if (ch == escape)
        {
            toEscape = true;
        }
        else if (delimSet.count(ch) and not toEscape
            and (not nSplits or count < nSplits))
        {
            if (token.length())
            {
                dest.push_back(token);
                token.clear();
                ++count;
            }
        }
        else
        {
            token += ch;
            toEscape = false;
        }
    }

    if (token.length())
    {
        dest.push_back(token);
    }

    return count;
}// end splitn

template <class DEST_CONT, class SRC_CONT, typename MAP_FUNC>
void map(DEST_CONT& dest, const SRC_CONT& src, MAP_FUNC mapFunc)
{
    for (const auto& elem : src)
    {
        dest.push_back(mapFunc(elem));
    }// end for elem
}// end map

template <class CONT, typename OPERATION>
void for_each(CONT& cont, OPERATION op)
{
    std::for_each(cont.begin(), cont.end(), op);
}// end for_each

template <class STR_T>
void to_lower(STR_T& str)
{
    for (auto& ch : str)
    {
        ch = std::tolower(ch);
    }// end for
}// end to_lower

template <class STR_T>
void to_upper(STR_T& str)
{
    for (auto& ch : str)
    {
        ch = std::toupper(ch);
    }// end for
}// end to_lower

};// end namespace utils
