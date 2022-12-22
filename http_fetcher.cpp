#include <cctype>
#include <map>

#include "deps.hpp"
#include "command.hpp"
#include "uri.hpp"

#include "http_fetcher.hpp"

#define     READ_LEN    0x1000

// === class HttpFetcher Implementation ===================================
//
// ========================================================================

// --- public constructors ------------------------------------------------

HttpFetcher::HttpFetcher(
    const string& shellCommand,
    const string& urlEnv
)
{
    m_cmd = Command(shellCommand).set_stdout_piped(true);
    m_urlEnv = urlEnv;
}// end type constructor

// --- public accessors ---------------------------------------------------

auto HttpFetcher::fetch_url(
    header_type& headers,
    const Uri& url
) const -> std::vector<char>
{
    using namespace std;

    std::vector<char>       body            = {};
    Command                 cmd             = m_cmd;
    string                  currLine        = {};
    size_t                  idx             = 0;

    // set up command
    cmd.set_env(m_urlEnv, url.str());

    auto        sproc       = cmd.spawn();

    // first, read headers
    while (getline(sproc.stdout(), currLine))
    {
        header_key_type             key         = {};
        header_value_type           value       = {};
        size_t                      idx         = 0;
        string::const_iterator      iter;


        // remove terminal carriage return
        if ((not currLine.empty()) and (currLine.back() == '\r'))
        {
            currLine.pop_back();
        }

        // if line is empty, break
        if (currLine.empty())
        {
            break;
        }

        if (currLine.find(':') == string::npos)
        {
            continue;
        }

        #define     BREAK_IF_NPOS(VAR) \
                    if ((VAR) >= string::npos) { break; }
        #define     WS      " \t"

        // parse key
        {
            size_t  beg     = 0;
            size_t  end;

            beg = currLine.find_first_not_of(WS);
            idx = currLine.find(':', beg);
            BREAK_IF_NPOS(idx);
            end = currLine.find_last_not_of(WS, idx - 1);

            if (string::npos == end)
            {
                end = beg;
            }
            else
            {
                ++end;
            }

            ++idx;
            key = currLine.substr(beg, end - beg);
        }

        // parse values
        while (idx < currLine.size())
        {
            size_t      beg     = idx;
            size_t      end;

            // find first non-whitespace character
            idx = currLine.find_first_not_of(WS, beg);
            BREAK_IF_NPOS(idx);
            beg = idx;

            // find terminating ';'
            // if none, end set to end of currLine
            idx = currLine.find(';', beg);
            if (string::npos == idx)
            {
                end = currLine.size();
            }
            else
            {
                end = currLine.find_last_not_of(WS, idx - 1);
                ++idx;
            }

            if (string::npos == end)
            {
                end = beg;
            }
            else
            {
                ++end;
            }

            value.push_back(currLine.substr(beg, end - beg));
        }// end while

        #undef      WS
        #undef      BREAK_IF_NPOS
        headers[key] = value;
    }// end while

    // next, read body
    while (sproc.stdout())
    {
        char    *buf;

        body.resize(body.size() + READ_LEN);
        buf = &body.back() + 1 - READ_LEN;

        sproc.stdout().read(buf, READ_LEN);
    }// end while

    if (not body.empty())
    {
        body.resize(body.size() + sproc.stdout().gcount() - READ_LEN);
    }

    sproc.stdout().close();
    sproc.wait();
    return body;
}// end HttpFetcher::fetch_url
