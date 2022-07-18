#ifndef __PAGE_HPP__
#define __PAGE_HPP__

#include "deps.hpp"
#include "document.hpp"
#include "document_text.hpp"
#include "document_html.hpp"
#include "uri.hpp"

class Page
{
    public:
        // --- public member type(s) --------------------------------------
        typedef     std::vector<Uri>        link_container;
        typedef     std::vector<Uri>        image_container;
        // --- public member class(es) ------------------------------------
        class UriAccessor : public std::vector<Uri>
        {
            // --- friend class(es) -----------------------------------------------
            friend class Page;

            protected:
                // --- protected constructor(s) -----------------------------------
                UriAccessor(void);

                template <typename ITER_T>
                UriAccessor(
                    ITER_T iter,
                    const ITER_T& end,
                    const Uri& base,
                    const size_t res = 0
                );
        };// end class Page:UriAccessor

        // --- public accessor(s) -----------------------------------------
        auto document(void) const
            -> const Document&;
        auto links_relative(void) const
            -> const link_container&;
        auto links_full(void) const
            -> const link_container&;
        auto images_relative(void) const
            -> const image_container&;
        auto images_full(void) const
            -> const image_container&;

        // --- public mutator(s) ------------------------------------------
        void redraw(const size_t cols);

        // --- public static function(s) ----------------------------------
        static auto from_text_stream(
                std::istream& ins,
                const Uri& uri,
                const size_t cols
            ) -> Page;
        static auto from_text_string(
                const string& inStr,
                const Uri& uri,
                const size_t cols
            ) -> Page;
        static auto from_html_stream(
                std::istream& ins,
                const Uri& uri,
                const size_t cols
            ) -> Page;
        static auto from_html_string(
                const string& inStr,
                const Uri& uri,
                const size_t cols
            ) -> Page;
    protected:
        // --- protected member variable(s) -------------------------------
        Uri                 m_uri                   = {};
        u_ptr<Document>     m_document              = {};
        link_container      m_linkUrisRel           = {};
        link_container      m_linkUrisFull          = {};
        image_container     m_imageUrisRel          = {};
        image_container     m_imageUrisFull         = {};
};// end class Page

#endif
