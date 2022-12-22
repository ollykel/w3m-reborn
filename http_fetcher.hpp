#ifndef __HTTP_FETCHER_HPP__
#define __HTTP_FETCHER_HPP__

#include <map>

#include "deps.hpp"
#include "command.hpp"
#include "uri.hpp"

class HttpFetcher
{
    public:
        // --- public member types ----------------------------------------
        typedef     string                  header_key_type;
        typedef     std::vector<string>     header_value_type;
        typedef     std::map<header_key_type, header_value_type>
                                            header_type;
        struct      Status
        {
            string      version;
            int         code;
            string      reason;
        };// end struct Protocol

        // --- public constructors ----------------------------------------
        HttpFetcher(
            const string& shellCommand,
            const string& urlEnv
        );

        // --- public accessors -------------------------------------------
        auto fetch_url(
            Status& status,
            header_type& headers,
            const Uri& url
        ) const -> std::vector<char>;

    private:
        // --- private member variables -----------------------------------
        Command     m_cmd;
        string      m_urlEnv;
};// end class HttpFetcher

#endif
