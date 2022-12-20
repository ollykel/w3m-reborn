#ifndef __TAB_HPP__
#define __TAB_HPP__

#include <list>

#include "deps.hpp"
#include "viewer.hpp"
#include "uri.hpp"
#include "document_fetcher.hpp"

#include "document.hpp"

class Tab
{
    public:
        // --- public member types ----------------------------------------
        typedef     Tab     type;
        struct Config
        {
            Viewer::Config      viewer;
        };// end struct Config
        class Page
        {
            friend class Tab;

            public:
                // --- public member types --------------------------------
                typedef     Page        type;

                // --- public constructors --------------------------------
                Page(const type& orig);     // copy
                Page(type&& orig);          // move

                // --- public accessors -----------------------------------
                auto document(void) const
                    -> const Document&;
                auto viewer(void) const
                    -> const Viewer&;
                auto uri(void) const
                    -> const Uri&;

                // --- public mutators ------------------------------------
                auto viewer(void)
                    -> Viewer&;
                auto operator=(const type& orig)
                    -> type&;
                auto operator=(type&& orig)
                    -> type&;
            private:
                // --- private member variables ---------------------------
                s_ptr<Document>     m_documentPtr;
                Viewer              m_viewer;
                Uri                 m_uri;

                // --- private constructors -------------------------------
                Page(
                    const s_ptr<Document>& doc,
                    const Uri& uri,
                    const Viewer::Config& cfg
                );// type

                // --- private mutators -----------------------------------
                void destruct(void);
                void copy_from(const type& orig);
        };// end struct Page
        typedef     std::list<Page>     page_container;

        // --- public constructors ----------------------------------------
        Tab(const Config& cfg, const DocumentFetcher& fetcher); // type
        Tab(const type& orig);                                  // copy
        Tab(type&& orig);                                       // move

        // --- public accessors -------------------------------------------
        auto pages(void) const
            -> const page_container&;
        auto curr_page(void) const
            -> const Page*;

        // --- public mutators --------------------------------------------
        auto curr_page(void)
            -> Page*;
        auto goto_uri(const Uri& uri)
            -> Page*;
        auto goto_pagenum(size_t index)
            -> Page*;
        auto prev_page(void)
            -> Page*;
        auto back_page(void)
            -> Page*;
        auto next_page(void)
            -> Page*;
        void destruct(void);
        void copy_from(const type& orig);
    private:
        // --- private member variables -----------------------------------
        Config                      m_cfg               = {};
        const DocumentFetcher       *m_fetcher          = nullptr;
        page_container              m_pages             = {};
        page_container::iterator    m_pageIter;
        size_t                      m_currPageIdx       = SIZE_MAX;
};// end class Tab

#endif
