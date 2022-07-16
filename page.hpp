#ifndef __PAGE_HPP__
#define __PAGE_HPP__

#include <map>

#include "deps.hpp"
#include "document.hpp"
#include "document_text.hpp"
#include "document_html.hpp"
#include "uri.hpp"

class Page
{
    public:
        // --- public member class(es) ------------------------------------
        class UriAccessor;

        // --- public accessor(s) -----------------------------------------
        auto document(void) const
            -> const Document&;
        auto links(void) const
            -> const UriAccessor&;
        auto images(void) const
            -> const UriAccessor&;

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
        Uri                 m_uri               = {};
        u_ptr<Document>     m_document          = {};
        UriAccessor         m_linkAccessor      = {};
        UriAccessor         m_imageAccessor     = {};
};// end class Page

// === class Page::UriAccessor ============================================
//
// ========================================================================
class Page::UriAccessor : public std::vector<Uri>
{
    // --- friend class(es) -----------------------------------------------
    friend class Page;

    protected:
        // --- protected constructor(s) -----------------------------------
        template <typename ITER_T>
        UriAccessor(
            ITER_T iter,
            const ITER_T& end,
            const Uri& base,
            const size_t res = 0
        );
};// end class Page:UriAccessor

#endif
