#include <map>

#include "deps.hpp"
#include "page.hpp"
#include "document.hpp"
#include "document_text.hpp"
#include "document_html.hpp"
#include "uri.hpp"

// === class Page Implementation ==========================================
//
// ========================================================================

// --- public accessor(s) -------------------------------------------------
auto Page::document(void) const
    -> const Document&
{
    return *m_document;
}// end Page::document

auto Page::links(void) const
    -> const UriAccessor&
{
    return m_linkAccessor;
}// end Page::links

auto Page::images(void) const
    -> const UriAccessor&
{
    return m_imageAccessor;
}// end Page::images

// --- public static function(s) ------------------------------------------
auto Page::from_text_stream(
    std::istream& ins,
    const Uri& uri,
    const size_t cols
) -> Page
{
    Page    out     = {};

    out.m_uri = uri;
    out.m_document = std::make_unique<DocumentText>(ins, cols);
    out.m_linkAccessor = UriAccessor(
                            out.m_document.links().cbegin(),
                            out.m_document.links().cend(),
                            uri,
                            out.m_document.links().size()
                        );
    out.m_imageAccessor = UriAccessor(
                            out.m_document.images().cbegin(),
                            out.m_document.images().cend(),
                            uri,
                            out.m_document.images().size()
                        );

    return out;
}// end Page::from_text_stream

auto Page::from_text_string(
    const string& inStr,
    const Uri& uri,
    const size_t cols
) -> Page
{
    Page    out     = {};

    out.m_uri = uri;
    out.m_document = std::make_unique<DocumentText>(inStr, cols);
    out.m_linkAccessor = UriAccessor(
                            out.m_document.links().cbegin(),
                            out.m_document.links().cend(),
                            uri,
                            out.m_document.links().size()
                        );
    out.m_imageAccessor = UriAccessor(
                            out.m_document.images().cbegin(),
                            out.m_document.images().cend(),
                            uri,
                            out.m_document.images().size()
                        );

    return out;
}// end Page::from_text_string

auto Page::from_html_stream(
    std::istream& ins,
    const Uri& uri,
    const size_t cols
) -> Page
{
    Page    out     = {};

    out.m_uri = uri;
    out.m_document = std::make_unique<DocumentHtml>(ins, cols);
    out.m_linkAccessor = UriAccessor(
                            out.m_document.links().cbegin(),
                            out.m_document.links().cend(),
                            uri,
                            out.m_document.links().size()
                        );
    out.m_imageAccessor = UriAccessor(
                            out.m_document.images().cbegin(),
                            out.m_document.images().cend(),
                            uri,
                            out.m_document.images().size()
                        );

    return out;
}// end Page::from_html_stream

auto Page::from_html_string(
    const string& inStr,
    const Uri& uri,
    const size_t cols
) -> Page
{
    Page    out     = {};

    out.m_uri = uri;
    out.m_document = std::make_unique<DocumentHtml>(inStr, cols);
    out.m_linkAccessor = UriAccessor(
                            out.m_document.links().cbegin(),
                            out.m_document.links().cend(),
                            uri,
                            out.m_document.links().size()
                        );
    out.m_imageAccessor = UriAccessor(
                            out.m_document.images().cbegin(),
                            out.m_document.images().cend(),
                            uri,
                            out.m_document.images().size()
                        );

    return out;
}// end Page::from_html_string

// === class Page::UriAccessor ============================================
//
// ========================================================================

template <typename ITER_T>
UriAccessor::UriAccessor(
    ITER_T iter,
    const ITER_T& end,
    const Uri& base,
    const size_t res
)
{
    reserve(res);

    for (; iter != end; ++iter)
    {
        auto&   uri     = *iter;

        push_back(Uri::relative(base, uri));
    }// end 
}// end UriAccessor::UriAccessor
