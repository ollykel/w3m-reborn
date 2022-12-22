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
    HttpFetcher::Status         status;
    HttpFetcher::header_type    headers;
    vector<char>                body;

    body = fetcher.fetch_url(status, headers, Uri("file:///dev/stdin"));

    cout << ">== Start HTTP Status ==<" << endl;
    cout << "\tVersion: " << status.version << endl;
    cout << "\tCode: " << status.code << endl;
    cout << "\tReason: " << status.reason << endl;
    cout << ">== End HTTP Status ==<" << endl;

    cout << endl;

    cout << ">== Start HTTP Headers ==<" << endl;
    for (const auto& kv : headers)
    {
        const auto&     key     = kv.first;
        const auto&     values  = kv.second;

        cout << '\t' << key << "=";

        for (const auto& val : values)
        {
            cout << val << "|";
        }// end for val

        cout << endl;
    }// end for kv

    cout << ">== End HTTP Headers ==<" << endl;
    cout << endl;
    cout << ">== Start HTTP Body ==<" << endl;
    cout << string(body.cbegin(), body.cend()) << endl;
    cout << ">== End HTTP Body ==<" << endl;

    return EXIT_SUCCESS;
}// end int main
