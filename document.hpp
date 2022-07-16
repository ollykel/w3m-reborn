#ifndef __DOCUMENT_HPP__
#define __DOCUMENT_HPP__

#include <list>

#include "deps.hpp"

class   Document
{
    public:
        // --- public member type(s) --------------------------------------
        class       BufferNode;
        class       Reference;
        struct      BufferIndex;

        typedef     BufferIndex                     BufIdx;
        typedef     std::vector<BufferNode>         BufferLine;
        typedef     std::vector<BufferLine>         buffer_type;
        typedef     std::vector<Reference>          link_container;
        typedef     std::vector<Reference>          image_container;

        // --- public constructor(s) --------------------------------------
        Document(void);// default

        // --- public accessor(s) -----------------------------------------
        auto title(void) const
            -> const string&;
        auto buffer(void) const
            -> const buffer_type&;
        auto links(void) const
            -> const link_container&;
        auto images(void) const
            -> const image_container&;

        // --- public mutator(s) ------------------------------------------
        void            clear(void);
        virtual void    redraw(size_t cols)
        {
            // do nothing
        }
        void set_title(const string& title);
    protected:
        // --- protected member variable(s) -------------------------------
        string              m_title         = "";
        buffer_type         m_buffer        = {};
        link_container      m_links         = {};
        image_container     m_images        = {};
};// end class Document

class   Document::BufferNode
{
    friend class Document;
    friend class DocumentText;
    friend class DocumentHtml;

    public:
        // === public constructor(s) ======================================
        BufferNode(
            const string& text = "",
            const bool isReserved = false,
            const cont::Ref& link = {},
            const cont::Ref& image = {}
        );// type/default

        // === public accessor(s) =========================================
        const string&                   text(void) const;
        bool                            reserved(void) const;
        const cont::Ref&                link_ref(void) const;
        const cont::Ref&                image_ref(void) const;
        auto    stylers(void) const -> const std::vector<string>&;

        // === public mutator(s) ==========================================
        void    set_text(const string& text);
        void    set_reserved(const bool state);
        void    set_link_ref(const size_t index);
        void    set_image_ref(const size_t index);
        void    append_styler(const string& styler);
        void    clear_text(void);
        void    clear_link_ref(void);
        void    clear_image_ref(void);
        void    clear_stylers(void);
    protected:
        // === protected member variable(s) ===============================
        string                  m_text          = "";
        bool                    m_isReserved    = false;
        std::vector<string>     m_stylers       = {};
        cont::Ref               m_linkRef       = {};
        cont::Ref               m_imageRef      = {};
};// end class Document::BufferNode

class   Document::Reference
{
    public:
        // === public member class(es) ====================================
        typedef     std::list<Document::BufferIndex>    referer_container;
        typedef     referer_container::iterator         referer_iterator;
        typedef     referer_container::const_iterator
                    const_referer_iterator;

        // === public constructor(s) ======================================
        Reference(const string& url = "");

        // === public accessor(s) =========================================
        const string& get_url(void) const;
        auto cbegin_referers(void)  -> referer_container::const_iterator;
        auto cend_referers(void)    -> const referer_container::const_iterator;

        // === public mutator(s) ==========================================
        void                    set_url(const string& url);
        void                    append_referer(size_t line, size_t col);
        void                    clear_referers(void);
        auto begin_referers(void)   -> referer_container::iterator;
        auto end_referers(void)     -> const referer_container::iterator;
    private:
        // === private member variable(s) =================================
        string                              m_url       = "";
        std::list<Document::BufferIndex>    m_referers  = {};
};// end class Document::Reference

struct      Document::BufferIndex
{
    // === public member variable(s) ======================================
    size_t      line;
    size_t      col;

    // === public constructor(s) ==========================================
    BufferIndex(size_t lnum, size_t cnum);
};// end struct Document::BufferIndex

#endif
