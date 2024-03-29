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
        typedef     string                      header_key_type;
        typedef     std::vector<string>         header_value_type;
        typedef     std::map<header_key_type, header_value_type>
                                                header_type;
        typedef     std::vector<char>           data_container;
        typedef     std::map<string,string>     env_map;
        struct      Status
        {
            string      version;
            int         code;
            string      reason;
        };// end struct Protocol

        // --- public constructors ----------------------------------------
        HttpFetcher(
            const string& shellCommand,
            const string& urlEnv,
            const env_map& env = {}
        );

        // --- public accessors -------------------------------------------
        auto fetch_url(
            Status& status,
            header_type& headers,
            const Uri& url,
            const data_container& input = {},
            const env_map& env = {}
        ) const -> data_container;

    private:
        // --- private member variables -----------------------------------
        Command     m_cmd;
        string      m_urlEnv;
};// end class HttpFetcher

#endif
