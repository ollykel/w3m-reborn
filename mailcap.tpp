template <typename CONTAINER_T>
void Mailcap::Entry::tokenize_fmt_string(CONTAINER_T& dest, const string& str)
{
    auto            iter        = str.cbegin();
    const auto      end         = str.cend();
    string          token       = "";

    while (iter != end)
    {
        token += utils::copy_token_until(iter, end, "%", true);
        if (iter == end)
            break;
        switch (*iter)
        {
            case '%':
                dest.push_back(token);
                token.clear();
                ++iter;
                if (iter == end)
                    return;
                if (*iter == '{')
                {
                    string      param       = "";

                    ++iter;
                    param = utils::copy_token_until(iter, end, "}", true);
                    dest.push_back("%{" + param + "}");
                    if (iter != end)
                        ++iter;
                }
                else
                {
                    dest.push_back("%" + *iter);
                    ++iter;
                }
                break;
            default:
                throw std::logic_error("unhandled char");
        }// end switch
    }// end while
}// end Mailcap::Entry::tokenize_fmt_string
