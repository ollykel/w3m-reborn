#include "deps.hpp"
#include "document.hpp"

// === class Document implementation ======================================

Document::Document(void)
{
    // do nothing
}// end Document::Document(void)

// === public accessor(s) =================================================

size_t  Document::num_lines(void) const
{
    return m_buffer.size();
}// end Document::num_lines(void) const

auto Document::get_link_at(size_t index) const -> const Reference&
{
    return m_links[index];
}// end get_link_at(size_t index) const

auto Document::get_image_at(size_t index) const -> const Reference&
{
    return m_images[index];
}// end get_image_at(size_t index) const

auto Document::cbegin_lines(size_t idx) const
    -> const_line_iterator
{
    auto        iter        = m_buffer.cbegin();

    while (idx--)
        ++iter;

    return iter;
}// end cbegin_lines(size_t idx) const

auto Document::cend_lines(void) const
    -> const const_line_iterator
{
    return m_buffer.cend();
}// end cend_lines(size_t idx = 0) const    -> const_line_iterator

auto Document::crbegin_lines(size_t idx) const
    -> const_reverse_line_iterator
{
    auto    iter    = m_buffer.crbegin();

    while (idx--)
        ++iter;

    return iter;
}// end Document::crbegin_lines(size_t idx = 0) const

void    Document::clear(void)
{
    m_buffer.clear();
    m_links.clear();
    m_images.clear();
}// end Document::clear(void)

auto Document::crend_lines(void) const
    -> const const_reverse_line_iterator
{
    return m_buffer.crend();
}// end Document::crend_lines(size_t idx = 0) const

// --- link iterator(s) ---------------------------------------------------

auto Document::cbegin_links(size_t idx) const -> const_ref_iterator
{
    auto    iter    = m_links.cbegin();

    while (idx--)
    {
        ++iter;
    }

    return iter;
}// end Document::cbegin_links(size_t idx = 0) const -> const_ref_iterator

auto Document::cend_links(void) const -> const const_ref_iterator
{
    return m_links.cend();
}// end Document::cend_links(void) const -> const const_ref_iterator

// --- image iterator(s) --------------------------------------------------

auto Document::cbegin_images(size_t idx) const -> const_ref_iterator
{
    auto    iter    = m_images.cbegin();

    while (idx--)
    {
        ++iter;
    }

    return iter;
}// end Document::cbegin_images(size_t idx = 0) const -> const_ref_iterator

auto Document::cend_images(void) const -> const const_ref_iterator
{
    return m_images.cend();
}// end Document::cend_images(void) const -> const const_ref_iterator

// === public mutator(s) ==================================================
auto Document::begin_lines(size_t idx) -> line_iterator
{
    auto    iter    = m_buffer.begin();

    while(idx--)
        ++iter;

    return iter;
}// end Document::begin_lines(size_t idx = 0)

auto Document::end_lines(void) -> const line_iterator
{
    return m_buffer.end();
}// end Document::end_lines(void)

auto Document::rbegin_lines(size_t idx) -> reverse_line_iterator
{
    auto    iter    = m_buffer.rbegin();

    while (idx--)
        ++iter;

    return iter;
}// end Document::rbegin_lines(size_t idx = 0) -> reverse_line_iterator

auto Document::rend_lines(void) -> const reverse_line_iterator
{
    return m_buffer.rend();
}// end Document::rend_lines(void) -> const reverse_line_iterator

// === class Document::BufferNode Implementation ==========================

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

// === public accessor(s) =================================================
const string&       Document::BufferNode::get_text(void) const
{
    return m_text;
}

bool                Document::BufferNode::reserved(void) const
{
    return m_isReserved;
}// end Document::BufferNode::reserved(void) const

const cont::Ref& Document::BufferNode::get_link_ref(void) const
{
    return m_linkRef;
}

const cont::Ref& Document::BufferNode::get_image_ref(void) const
{
    return m_imageRef;
}

// === public mutator(s) ==================================================
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


// === class Document::Reference Implementation ===========================

// === public constructor(s) ==============================================
Document::Reference::Reference(const string& url)
{
    m_url = url;
}// end Document::Reference::Reference(const string& url)

// === public accessor(s) =================================================
const string&   Document::Reference::get_url(void) const
{
    return m_url;
}// end Document::Reference::get_url(void) const

auto Document::Reference::cbegin_referers(void)
    -> referer_container::const_iterator
{
    return m_referers.cbegin();
}// end Document::Reference::cbegin_referers(void)

auto Document::Reference::cend_referers(void)
    -> const referer_container::const_iterator
{
    return m_referers.cend();
}// end Document::Reference::cend_referers(void)

// === public mutator(s) ==================================================
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

auto Document::Reference::begin_referers(void)
    -> referer_container::iterator
{
    return m_referers.begin();
}// end Document::Reference::begin_referers(void)

auto Document::Reference::end_referers(void)
    -> const referer_container::iterator
{
    return m_referers.end();
}// end Document::Reference::end_referers(void)

// === Document::BufferIndex Implementation ===============================

// === public constructor(s) ==============================================
Document::BufferIndex::BufferIndex(size_t lnum, size_t cnum)
{
    line = lnum;
    col = cnum;
}// end Document::BufferIndex::BufferIndex(size_t lnum, size_t cnum)
