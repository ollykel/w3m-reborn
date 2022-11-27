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
        struct Config
        {
            Document::Config    document;
        };// end struct Page::Config

        // --- public constructors ----------------------------------------
        Page(const Config& cfg);
        Page(const Page& other);

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
        auto operator=(const Page& other)
            -> Page&;

        // --- public static function(s) ----------------------------------
        static auto from_text_stream(
                const Config& cfg,
                std::istream& ins,
                const Uri& uri,
                const size_t cols
            ) -> Page;
        static auto from_text_string(
                const Config& cfg,
                const string& inStr,
                const Uri& uri,
                const size_t cols
            ) -> Page;
        static auto from_html_stream(
                const Config& cfg,
                std::istream& ins,
                const Uri& uri,
                const size_t cols
            ) -> Page;
        static auto from_html_string(
                const Config& cfg,
                const string& inStr,
                const Uri& uri,
                const size_t cols
            ) -> Page;
    protected:
        // --- protected member classes -----------------------------------
        enum class Kind
        {
            text,
            html
        };// end enum class Kind

        // --- protected member variable(s) -------------------------------
        Config              m_config                = {};
        Kind                m_kind                  = Kind::text;
        Uri                 m_uri                   = {};
        u_ptr<Document>     m_document              = {};
        link_container      m_linkUrisRel           = {};
        link_container      m_linkUrisFull          = {};
        image_container     m_imageUrisRel          = {};
        image_container     m_imageUrisFull         = {};
    private:
        // --- private mutators -------------------------------------------
        void copy_from(const Page& other);
};// end class Page

#endif
