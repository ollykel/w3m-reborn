#include "deps.hpp"
#include "uri.hpp"

// === public constructor(s) ==============================================
Uri::Uri(void)
{
    // do nothing
}// end Uri::Uri(void)

Uri::Uri(const string& str)
{
    if (str.empty())
        return;
    
    auto            iter    = str.cbegin();
    const auto      end     = str.cend();

    if (str.front() == '/')
    {
        if (str.length() > 1 and str.at(1) == '/')
        {
            iter += 2;
            goto parse_authority;
        }
        else
        {
            goto parse_path;
        }
    }
    else if (str.front() == '#' or str.front() == '?')
    {
        goto parse_path;
    }
parse_scheme:
    for (; iter != end and *iter != ':'; ++iter)
    {
        scheme += *iter;
    }// end for iter
    iter += 3;
parse_authority:
    {
        string      accum       = "";
        string      *dest       = &host;

        for (; iter != end and *iter != '/'; ++iter)
        {
            switch (*iter)
            {
                case '@':
                    if (userInfo.empty() and dest != &port)
                    {
                        userInfo = accum;
                        accum.clear();
                    }
                    else
                    {
                        accum += '@';
                    }
                    break;
                case ':':
                    if (dest != &port)
                    {
                        *dest = accum;
                        accum.clear();
                        dest = &port;
                    }
                    else
                    {
                        accum += ':';
                    }
                    break;
                default:
                    accum += *iter;
            }// end switch (*iter)
        }// end for (; iter != end and *iter != '/'; ++iter)
        *dest = accum;
    }// end parse_authority
parse_path:
    {
        string      accum       = "";
        string      *dest       = &path;

        for (; iter != end; ++iter)
        {
            switch (*iter)
            {
                case '?':
                    if (dest == &path)
                    {
                        *dest = accum;
                        accum.clear();
                        dest = &query;
                    }
                    else
                    {
                        accum += '?';
                    }
                    break;
                case '#':
                    if (dest != &fragment)
                    {
                        *dest = accum;
                        accum.clear();
                        dest = &fragment;
                    }
                    else
                    {
                        accum += '#';
                    }
                    break;
                default:
                    accum += *iter;
            }// end switch (*iter)
        }// end for (; iter != end; ++iter)

        *dest = accum;
    }
}// end Uri::Uri(const string& str)

// === public accessor(s) =================================================
auto Uri::str(void) const
    -> string
{
    string      out     = "";

    if (not scheme.empty())
    {
        out += scheme + "://";
    }

    if (not host.empty())
    {
        if (not userInfo.empty())
        {
            out += userInfo + "@";
        }
        out += host;
        if (not port.empty())
        {
            out += port;
        }
    }

    if (not path.empty())
    {
        out += path;
    }
    if (not query.empty())
    {
        out += "?" + query;
    }
    if (not fragment.empty())
    {
        out += "#" + fragment;
    }

    return out;
}// end Uri::str(void) const -> string

// --- public static function(s) ------------------------------------------
auto Uri::from_relative(const type& base, const type& rel)
    -> type
{
    Uri     out     = rel;

    if (out.scheme.empty())
    {
        out.scheme = base.scheme;
    }
    if (out.host.empty())
    {
        out.userInfo = base.userInfo;
        out.host = base.host;
        out.port = base.port;

        if (out.path.empty())
        {
            out.path = base.path;

            if (out.query.empty())
            {
                out.query = base.query;
            }
        }
    }

finally:
    return out;
}// end Uri::from_relative

std::ostream&   operator<<(std::ostream& outs, const Uri& uri)
{
    using namespace std;

    outs << '{' << endl;

    outs << "\tscheme: "     << uri.scheme       << endl;
    outs << "\tuserInfo: "   << uri.userInfo       << endl;
    outs << "\thost: "       << uri.host       << endl;
    outs << "\tport: "       << uri.port       << endl;
    outs << "\tpath: "       << uri.path       << endl;
    outs << "\tquery: "      << uri.query       << endl;
    outs << "\tfragment: "   << uri.fragment       << endl;

    outs << '}' << endl;

    return outs;
}// end operator<<(std::ostream& outs, const Uri& uri)
