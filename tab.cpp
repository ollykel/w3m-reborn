#include <map>

#include "deps.hpp"
#include "uri.hpp"
#include "viewer.hpp"

#include "tab.hpp"

#define     TAB_MOVE_FROM(ORIG) \
{ \
    m_pages = (ORIG).m_pages; \
    m_currPageIdx = (ORIG).m_currPageIdx; \
}

#define     PAGE_MOVE_FROM(ORIG) \
{ \
    m_documentPtr = (ORIG).m_documentPtr; \
    m_viewer = (ORIG).m_viewer; \
    m_uri = (ORIG).m_uri; \
}

// === class Tab Implementation ===========================================
//
// ========================================================================

// --- public constructors ----------------------------------------
Tab::Tab(const Config& cfg)
{
    m_cfg = cfg;
    m_pageIter = m_pages.begin();
    m_currPageIdx = SIZE_MAX;
}// end type constructor

Tab::Tab(const Tab& orig)
{
    copy_from(orig);
}// end copy constructor

Tab::Tab(Tab&& orig)
{
    TAB_MOVE_FROM(orig);
}// end move constructor

// --- public accessors -------------------------------------------
auto Tab::pages(void) const
    -> const page_container&
{
    return m_pages;
}// end Tab::pages

auto Tab::curr_page(void) const
    -> const Page*
{
    if (m_pages.end() == m_pageIter)
    {
        return nullptr;
    }

    return &(*m_pageIter);
}// end Tab::curr_page

// --- public mutators --------------------------------------------
auto Tab::curr_page(void)
    -> Page*
{
    if (m_pages.end() == m_pageIter)
    {
        return nullptr;
    }

    return &(*m_pageIter);
}// end Tab::curr_page

auto Tab::push_document(const s_ptr<Document>& doc, const Uri& uri)
    -> Page*
{
    if (m_pages.empty())
    {
        m_pages.emplace_back(
            Page(doc, uri, m_cfg.viewer, m_startLine, m_startCol)
        );
        m_pageIter = m_pages.begin();
        m_currPageIdx = 0;
    }
    else
    {
        if (m_pageIter != m_pages.end())
        {
            ++m_pageIter;
        }
        m_pageIter = m_pages.emplace(
            m_pageIter,
            Page(doc, uri, m_cfg.viewer, m_startLine, m_startCol)
        );
        ++m_currPageIdx;
    }

    return curr_page();
}// end Tab::push_document

auto Tab::push_page(const Page& orig)
    -> Page*
{
    if (m_pages.empty())
    {
        m_pages.emplace_back(orig);
        m_pageIter = m_pages.begin();
        m_currPageIdx = 0;
    }
    else
    {
        if (m_pageIter != m_pages.end())
        {
            ++m_pageIter;
        }
        m_pageIter = m_pages.emplace(m_pageIter, orig);
        ++m_currPageIdx;
    }

    return curr_page();
}// end Tab::push_page

auto Tab::goto_pagenum(size_t index)
    -> Page*
{
    if ((m_pages.empty()) or (index >= m_pages.size()))
    {
        return curr_page();
    }

    while (m_currPageIdx < index)
    {
        ++m_currPageIdx;
        ++m_pageIter;
    }// end while

    while (m_currPageIdx > index)
    {
        --m_currPageIdx;
        --m_pageIter;
    }// end while

    return curr_page();
}// end Tab::goto_pagenum

auto Tab::prev_page(void)
    -> Page*
{
    if (m_currPageIdx and (m_currPageIdx != SIZE_MAX))
    {
        --m_currPageIdx;
        --m_pageIter;
    }

    return curr_page();
}// end Tab::prev_page

auto Tab::next_page(void)
    -> Page*
{
    if (m_currPageIdx + 1 < m_pages.size())
    {
        ++m_currPageIdx;
        ++m_pageIter;
    }

    return curr_page();
}// end Tab::next_page

auto Tab::back_page(void)
    -> Page*
{
    page_container::iterator    oldIter     = m_pageIter;

    if (m_pages.begin() == oldIter)
    {
        return curr_page();
    }

    --m_pageIter;
    --m_currPageIdx;
    
    m_pages.erase(oldIter);
    return curr_page();
}// end Tab::back_page

void Tab::set_start_line(size_t lnum)
{
    m_startLine = lnum;
    
    for (auto& pg : m_pages)
    {
        pg.viewer().set_start_line(lnum);
    }// end for
}// end Tab::set_start_line

void Tab::set_start_col(size_t cnum)
{
    m_startCol = cnum;
    
    for (auto& pg : m_pages)
    {
        pg.viewer().set_start_col(cnum);
    }// end for
}// end Tab::set_start_col

void Tab::set_start_point(size_t lnum, size_t cnum)
{
    set_start_line(lnum);
    set_start_col(cnum);
}// end Tab::set_start_point

void Tab::destruct(void)
{
    m_pages.clear();
}// end Tab::destruct

void Tab::copy_from(const type& orig)
{
    m_pages = orig.m_pages;
    m_pageIter = m_pages.begin();
    m_currPageIdx = 0;

    if (m_pages.empty())
    {
        m_currPageIdx = SIZE_MAX;
        return;
    }

    while (m_currPageIdx < orig.m_currPageIdx)
    {
        ++m_currPageIdx;
        ++m_pageIter;
    }
}// end Tab::copy_from

// === class Tab::Page Implementation =====================================
//
// ========================================================================

// --- public constructors ------------------------------------------------
Tab::Page::Page(const type& orig)
{
    copy_from(orig);
}// end copy constructor

Tab::Page::Page(type&& orig)
{
    PAGE_MOVE_FROM(orig);
}// end move constructor

// --- public accessors ---------------------------------------------------
auto Tab::Page::document(void) const
    -> const Document&
{
    return *m_documentPtr.get();
}// end Tab::Page::document

auto Tab::Page::viewer(void) const
    -> const Viewer&
{
    return m_viewer;
}// end Tab::Page::viewer

auto Tab::Page::uri(void) const
    -> const Uri&
{
    return m_uri;
}// end Tab::Page::uri

auto Tab::Page::title(void) const
    -> string
{
    if (not document().title().empty())
    {
        return document().title();
    }
    else
    {
        return uri().str();
    }
}// end Tab::Page::title

// --- public mutators ----------------------------------------------------
auto Tab::Page::document(void)
    -> Document&
{
    return *m_documentPtr.get();
}// end Tab::Page::document

auto Tab::Page::viewer(void)
    -> Viewer&
{
    return m_viewer;
}// end Tab::Page::viewer

auto Tab::Page::operator=(const type& orig)
    -> type&
{
    if (&orig != this)
    {
        destruct();
        copy_from(orig);
    }

    return *this;
}// end Tab::Page::operator=

auto Tab::Page::operator=(type&& orig)
    -> type&
{
    if (&orig != this)
    {
        destruct();
        PAGE_MOVE_FROM(orig);
    }

    return *this;
}// end Tab::Page::operator=

// --- private constructors -----------------------------------------------
Tab::Page::Page(
    const s_ptr<Document>& doc,
    const Uri& uri,
    const Viewer::Config& cfg,
    size_t startLine,
    size_t startCol
)
{
    m_uri = uri;
    m_documentPtr = doc;
    m_viewer = Viewer(cfg, m_documentPtr.get());
    m_viewer.set_start_point(startLine, startCol);
}// end type constructor

// --- private mutators ---------------------------------------------------
void Tab::Page::destruct(void)
{
    m_documentPtr.reset();
}// end Tab::Page::destruct

void Tab::Page::copy_from(const type& orig)
{
    m_uri = orig.m_uri;
    m_documentPtr = orig.m_documentPtr;
    m_viewer = orig.m_viewer;
}// end Tab::Page::copy_from
