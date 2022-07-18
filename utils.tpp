namespace utils
{

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
