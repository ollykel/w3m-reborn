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
                ++iter;
                if (iter == end)
                    return;
                if (*iter == '{')
                {
                    string      param       = "";

                    dest.emplace_back(token);
                    token.clear();
                    ++iter;
                    param = utils::copy_token_until(iter, end, "}", true);
                    dest.emplace_back("%{" + param + "}");
                    if (iter != end)
                    {
                        ++iter;
                    }
                }
                else if (*iter == '%')
                {
                    token += '%';
                    ++iter;
                }
                else
                {
                    dest.emplace_back(token);
                    token.clear();
                    dest.emplace_back(string({ '%', *iter }));
                    ++iter;
                }
                break;
            default:
                throw std::logic_error("unhandled char");
        }// end switch
    }// end while

    if (not token.empty())
    {
        dest.emplace_back(token);
    }
}// end Mailcap::Entry::tokenize_fmt_string
