#ifndef __DOCUMENT_FETCHER_HPP__
#define __DOCUMENT_FETCHER_HPP__

#include <map>

#include "deps.hpp"
#include "command.hpp"
#include "document.hpp"

class DocumentFetcher
{
    public:
        // --- public constructors ----------------------------------------
        DocumentFetcher(
            const string& shellCmd,
            const Document::Config& documentConfig
        );

        // --- public accessors -------------------------------------------
        auto fetch_url(
            const string& url,
            std::map<string, string>& headers
        ) const -> s_ptr<Document>;

        // --- public mutators --------------------------------------------
        void set_property(const string& key, const string& val);
        auto document_config(void)
            -> Document::Config&;
    private:
        // --- private member variables -----------------------------------
        Command             m_cmd;
        Document::Config    m_documentConfig;
};// end class DocumentFetcher

#endif
