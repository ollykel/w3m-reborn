#include <map>

#include "deps.hpp"
#include "uri.hpp"
#include "viewer.hpp"

#include "tab.hpp"

#define     TAB_MOVE_FROM(ORIG) \
{ \
    m_fetcher = (ORIG).m_fetcher; \
    m_pages = (ORIG).m_pages; \
    m_currPageIdx = (ORIG).m_currPageIdx; \
}

#define     PAGE_MOVE_FROM(ORIG) \
{ \
    m_documentPtr.reset((ORIG).m_documentPtr.get()); \
    m_viewer = (ORIG).m_viewer; \
    m_uri = (ORIG).m_uri; \
}

// === class Tab Implementation ===========================================
//
// ========================================================================

// --- public constructors ----------------------------------------
Tab::Tab(const Config& cfg, const DocumentFetcher& fetcher)
{
    m_cfg = cfg;
    m_fetcher = &fetcher;
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
    if (m_currPageIdx >= m_pages.size())
    {
        return nullptr;
    }

    return &m_pages.at(m_currPageIdx);
}// end Tab::curr_page

// --- public mutators --------------------------------------------
auto Tab::curr_page(void)
    -> Page*
{
    if (m_currPageIdx >= m_pages.size())
    {
        return nullptr;
    }

    return &m_pages.at(m_currPageIdx);
}// end Tab::curr_page

auto Tab::goto_uri(const Uri& uri)
    -> Page*
{
    if (not uri.empty())
    {
        if (curr_page() and uri.is_fragment())
        {
            const string&   section = uri.fragment;
            auto    idx
                = curr_page()->document().get_section_index(section);

            if (idx)
            {
                curr_page()->viewer().goto_point(idx.line, 0);
            }
            else
            {
                curr_page()->viewer().disp_status(
                    "ERROR: could not find #" +
                    uri.fragment
                );
            }
        }
        else
        {
            std::map<string, string>    headers;
            Uri                         targetUri;
            s_ptr<Document>             doc;

            if (curr_page())
            {
                targetUri = Uri::from_relative(curr_page()->uri(), uri);
            }
            else
            {
                targetUri = Uri::from_relative({ "https" }, uri);
            }

            // emplace new page
            doc = m_fetcher->fetch_url(targetUri.str(), headers);
            m_pages.emplace_back(Page(*doc.get(), targetUri, m_cfg.viewer));
            m_currPageIdx = m_pages.size() - 1;
        }
    }

    return curr_page();
}// end Tab::goto_uri

auto Tab::goto_pagenum(size_t index)
    -> Page*
{
    if (index >= m_pages.size())
    {
        return curr_page();
    }

    m_currPageIdx = index;
    return curr_page();
}// end Tab::goto_pagenum

auto Tab::prev_page(void)
    -> Page*
{
    if (m_currPageIdx)
    {
        --m_currPageIdx;
    }

    return curr_page();
}// end Tab::prev_page

auto Tab::next_page(void)
    -> Page*
{
    if (m_currPageIdx + 1 < m_pages.size())
    {
        ++m_currPageIdx;
    }

    return curr_page();
}// end Tab::next_page

void Tab::destruct(void)
{
    m_fetcher = nullptr;
    m_pages.clear();
}// end Tab::destruct

void Tab::copy_from(const type& orig)
{
    m_fetcher = orig.m_fetcher;
    m_pages = orig.m_pages;
    m_currPageIdx = orig.m_currPageIdx;
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

// --- public mutators ----------------------------------------------------
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
Tab::Page::Page(Document& doc, const Uri& uri, const Viewer::Config& cfg)
{
    m_uri = uri;
    m_documentPtr.reset(&doc);
    m_viewer = Viewer(cfg, m_documentPtr.get());
}// end type constructor

// --- private mutators ---------------------------------------------------
void Tab::Page::destruct(void)
{
    m_documentPtr.reset();
}// end Tab::Page::destruct

void Tab::Page::copy_from(const type& orig)
{
    m_uri = orig.m_uri;
    m_documentPtr.reset(orig.m_documentPtr.get());
    m_viewer = orig.m_viewer;
}// end Tab::Page::copy_from
