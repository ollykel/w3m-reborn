#include "deps.hpp"
#include "document.hpp"

// === class Document implementation ======================================
//
// ========================================================================

Document::Document(void)
{
    // do nothing
}// end Document::Document(void)

// --- public accessor(s) -------------------------------------------------

auto Document::title(void) const
    -> const string&
{
    return m_title;
}// end Document::title

auto Document::buffer(void) const
    -> const buffer_type&
{
    return m_buffer;
}// end Document::buffer

auto Document::links(void) const
    -> const link_container&
{
    return m_links;
}// end Document::links

auto Document::images(void) const
    -> const image_container&
{
    return m_images;
}// end Document::images

auto Document::forms(void) const
    -> const form_container&
{
    return m_forms;
}// end Document::forms

// --- public mutator(s) --------------------------------------------------
void    Document::clear(void)
{
    m_buffer.clear();
    m_links.clear();
    m_images.clear();
}// end Document::clear(void)

void    Document::set_title(const string& title)
{
    m_title = title;
}// end Document::set_title

// === class Document::BufferNode Implementation ==========================
//
// ========================================================================

Document::BufferNode::BufferNode(
    const string& text,
    const bool isReserved,
    const cont::Ref& link,
    const cont::Ref& image
)
{
    m_text = text;
    m_isReserved = isReserved;
    m_linkRef = link;
    m_imageRef = image;
}// end Document::BufferNode::BufferNode(string text, linkIdx, imageIdx)

// --- public accessor(s) -------------------------------------------------
auto Document::BufferNode::text(void) const
    -> const string&
{
    return m_text;
}

auto Document::BufferNode::reserved(void) const
    -> bool
{
    return m_isReserved;
}// end Document::BufferNode::reserved(void) const

auto Document::BufferNode::link_ref(void) const
    -> const cont::Ref&
{
    return m_linkRef;
}

auto Document::BufferNode::image_ref(void) const
    -> const cont::Ref&
{
    return m_imageRef;
}

auto Document::BufferNode::stylers(void) const
    -> const std::vector<string>&
{
    return m_stylers;
}// end Document::BufferNode::stylers(void) const

// --- public mutator(s) --------------------------------------------------
void    Document::BufferNode::set_text(const string& text)
{
    m_text = text;
}// end Document::BufferNode::set_text(const string& text)

void    Document::BufferNode::set_reserved(const bool state)
{
    m_isReserved = state;
}// end Document::BufferNode::set_reserved(const bool state)

void    Document::BufferNode::set_link_ref(const size_t index)
{
    m_linkRef = index;
}// end Document::BufferNode::set_link_ref(const size_t index)

void    Document::BufferNode::set_image_ref(const size_t index)
{
    m_imageRef = index;
}// end Document::BufferNode::set_image_ref(const size_t index)

void    Document::BufferNode::append_styler(const string& styler)
{
    m_stylers.emplace_back(styler);
}// end Document::BufferNode::append_styler(const string& styler)

void    Document::BufferNode::clear_text(void)
{
    m_text.clear();
}// end Document::BufferNode::clear_text(void)

void    Document::BufferNode::clear_link_ref(void)
{
    m_linkRef.clear();
}// end Document::BufferNode::clear_link_ref(void)

void    Document::BufferNode::clear_image_ref(void)
{
    m_imageRef.clear();
}// end Document::BufferNode::clear_image_ref(void)

void    Document::BufferNode::clear_stylers(void)
{
}// end Document::BufferNode::clear_stylers(void)

// === class Document::Reference Implementation ===========================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
Document::Reference::Reference(const string& url)
{
    m_url = url;
}// end Document::Reference::Reference(const string& url)

// --- public accessor(s) -------------------------------------------------
auto Document::Reference::get_url(void) const
    -> const string&
{
    return m_url;
}// end Document::Reference::get_url(void) const

auto Document::Reference::referers(void) const
    -> const referer_container&
{
    return m_referers;
}// end Document::Reference::referers

// --- public mutator(s) --------------------------------------------------

void Document::Reference::set_url(const string& url)
{
    m_url = url;
}// end Document::Reference::set_url(const string& url)

void Document::Reference::append_referer(size_t line, size_t col)
{
    m_referers.emplace_back(line, col);
}// end Document::Reference::append_referer(size_t line, size_t col)

void Document::Reference::clear_referers(void)
{
    m_referers.clear();
}// end Document::Reference::clear_referers(void)

// === Document::BufferIndex Implementation ===============================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
Document::BufferIndex::BufferIndex(size_t lnum, size_t cnum)
{
    line = lnum;
    col = cnum;
}// end Document::BufferIndex::BufferIndex(size_t lnum, size_t cnum)
