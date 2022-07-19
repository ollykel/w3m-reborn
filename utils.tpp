#include <cctype>
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

};// end namespace utils
