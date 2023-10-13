#include <map>

#include <curses.h>

#include "deps.hpp"
#include "command.hpp"
#include "document.hpp"
#include "document_text.hpp"
#include "document_html.hpp"

#include "document_fetcher.hpp"

// === class DocumentFetcher Implementation ===============================
//
// ========================================================================

// --- public constructors ------------------------------------------------
DocumentFetcher::DocumentFetcher(
    const string& shellCmd,
    const Document::Config& documentConfig
)
{
    m_cmd = Command(shellCmd);
    m_cmd.set_std_out_piped(true);
    m_documentConfig = documentConfig;
}// end constructor

// --- public accessors ---------------------------------------------------

// === auto DocumentFetcher::fetch_url ====================================
//
// Fetches a document identified by a url, returning a smart pointer
// to a Document.
//
// Parameters:
//  url         IN      -- url of document to fetch
//  headers     IN      -- headers with which to make request
//
// Return:
//  A smart pointer pointing to the fetched document.
//
// ========================================================================
auto DocumentFetcher::fetch_url(
    const string& url,
    std::map<string, string>& headers
) const -> s_ptr<Document>
{
    Command             cmd         = m_cmd;
    s_ptr<Document>     docPtr;

    cmd.set_env("W3M_URL", url);

    auto        sproc   = cmd.spawn();

    while (sproc.std_out())
    {
        string      line;
        string      key;
        string      val;
        size_t      colonIdx;

        getline(sproc.std_out(), line);

        if (line.empty() or (line.size() == 1 and line[0] == '\r'))
        {
            break;
        }

        colonIdx = line.find(':');

        if (string::npos == colonIdx)
        {
            continue;
        }

        key = line.substr(0, line.find(':'));

        for (auto& ch : key)
        {
            key = tolower(ch);
        }// end for

        for (auto iter = line.begin() + colonIdx; iter != line.end(); ++iter)
        {
            if (' ' != *iter and '\t' != *iter)
            {
                for (auto iterB = iter; iter != line.end(); ++iter)
                {
                    if (';' == *iterB)
                    {
                        val = string(iter, iterB);
                        break;
                    }
                }// end for
                if (val.empty())
                {
                    val = string(iter, line.end());
                }
                break;
            }
        }// end for

        headers[key] = val;
    }// end while

    // TODO: differentiate document types
    // if (headers.at("content-type") == "document/html")
    {
        DocumentHtml    *docHtml    = new DocumentHtml(m_documentConfig);

        docPtr.reset(docHtml);
        docHtml->from_stream(sproc.std_out(), COLS);

        sproc.std_out().close();
        sproc.wait();
    }

    return docPtr;
}// end fetch_url

// --- public mutators ----------------------------------------------------
void DocumentFetcher::set_property(const string& key, const string& val)
{
    m_cmd.set_env(key, val);
}// end set_property

auto DocumentFetcher::document_config(void)
    -> Document::Config&
{
    return m_documentConfig;
}// end DocumentFetcher::document_config
