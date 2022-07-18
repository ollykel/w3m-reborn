#include <sstream>

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

auto Page::links_relative(void) const
    -> const link_container&
{
    return m_linkUrisRel;
}// end Page::links_relative

auto Page::links_full(void) const
    -> const link_container&
{
    return m_linkUrisFull;
}// end Page::links_full

auto Page::images_relative(void) const
    -> const image_container&
{
    return m_imageUrisRel;
}// end Page::images_relative

auto Page::images_full(void) const
    -> const image_container&
{
    return m_imageUrisFull;
}// end Page::images_full

// --- public mutator(s) --------------------------------------------------
void Page::redraw(const size_t cols)
{
    m_document->redraw(cols);
}// end Page::redraw

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

    out.m_linkUrisRel.reserve(out.m_document->links().size());
    for (const auto& link : out.m_document->links())
    {
        out.m_linkUrisRel.emplace_back(link.get_url());
    }// end for link

    out.m_imageUrisRel.reserve(out.m_document->links().size());
    for (const auto& image : out.m_document->images())
    {
        out.m_imageUrisRel.emplace_back(image.get_url());
    }// end for image

    out.m_linkUrisFull.reserve(out.m_linkUrisRel.size());
    for (const auto& link : out.m_linkUrisRel)
    {
        out.m_linkUrisFull.push_back(Uri::from_relative(uri, link));
    }// end for link

    out.m_imageUrisFull.reserve(out.m_imageUrisRel.size());
    for (const auto& image : out.m_imageUrisRel)
    {
        out.m_imageUrisFull.push_back(Uri::from_relative(uri, image));
    }// end for image

    return out;
}// end Page::from_text_stream

auto Page::from_text_string(
    const string& inStr,
    const Uri& uri,
    const size_t cols
) -> Page
{
    std::istringstream      stream(inStr);

    return from_text_stream(stream, uri, cols);
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

    out.m_linkUrisRel.reserve(out.m_document->links().size());
    for (const auto& link : out.m_document->links())
    {
        out.m_linkUrisRel.emplace_back(link.get_url());
    }// end for link

    out.m_imageUrisRel.reserve(out.m_document->links().size());
    for (const auto& image : out.m_document->images())
    {
        out.m_imageUrisRel.emplace_back(image.get_url());
    }// end for image

    out.m_linkUrisFull.reserve(out.m_linkUrisRel.size());
    for (const auto& link : out.m_linkUrisRel)
    {
        out.m_linkUrisFull.push_back(Uri::from_relative(uri, link));
    }// end for link

    out.m_imageUrisFull.reserve(out.m_imageUrisRel.size());
    for (const auto& image : out.m_imageUrisRel)
    {
        out.m_imageUrisFull.push_back(Uri::from_relative(uri, image));
    }// end for image

    return out;
}// end Page::from_html_stream

auto Page::from_html_string(
    const string& inStr,
    const Uri& uri,
    const size_t cols
) -> Page
{
    std::istringstream      stream(inStr);

    return from_html_stream(stream, uri, cols);
}// end Page::from_html_string
