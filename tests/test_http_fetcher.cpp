#include <map>

#include "../deps.hpp"
#include "../command.hpp"
#include "../uri.hpp"
#include "../http_fetcher.hpp"

// === main ===============================================================
//
// ========================================================================
int main(const int argc, const char **argv)
{
    using namespace std;

    HttpFetcher                 fetcher("cat", "W3M_URL");
    HttpFetcher::header_type    headers;
    vector<char>                body;

    body = fetcher.fetch_url(headers, Uri("file:///dev/stdin"));

    cout << "HTTP Headers:" << endl;
    cout << "=====" << endl;
    for (const auto& kv : headers)
    {
        const auto&     key     = kv.first;
        const auto&     values  = kv.second;

        cout << key << ": ";

        for (const auto& val : values)
        {
            cout << val << "; ";
        }// end for val

        cout << endl;
    }// end for kv

    cout << "HTTP Body:" << endl;
    cout << "=====" << endl;
    cout << string(body.cbegin(), body.cend()) << endl;

    return EXIT_SUCCESS;
}// end int main
