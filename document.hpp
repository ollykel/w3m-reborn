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

        typedef     BufferIndex                         BufIdx;
        typedef     std::vector<BufferNode>             BufferLine;
        typedef     std::vector<BufferLine>::iterator
                    line_iterator;
        typedef     std::vector<BufferLine>::reverse_iterator
                    reverse_line_iterator;
        typedef     std::vector<BufferLine>::const_iterator
                    const_line_iterator;
        typedef     std::vector<BufferLine>::const_reverse_iterator
                    const_reverse_line_iterator;
        typedef     BufferLine::iterator            node_iterator;
        typedef     BufferLine::const_iterator      const_node_iterator;
        typedef     std::vector<Reference>::const_iterator
                    const_ref_iterator;
        typedef     std::vector<BufferLine>         buffer_type;
        typedef     std::vector<Reference>          link_container_type;
        typedef     std::vector<Reference>          image_container_type;

        // --- public constructor(s) --------------------------------------
        Document(void);// default

        // --- public accessor(s) -----------------------------------------
        auto num_lines(void) const
            -> size_t;
        auto title(void) const
            -> const string&;
        auto buffer(void) const
            -> const buffer_type&;
        auto links(void) const
            -> const link_container_type&;
        auto images(void) const
            -> const image_container_type&;
        auto get_link_at(size_t index) const
            -> const Reference&;
        auto get_image_at(size_t index) const
            -> const Reference&;
        // --- line iterators ---------------------------------------------
        auto cbegin_lines(size_t idx = 0) const  -> const_line_iterator;
        auto cend_lines(void) const         -> const const_line_iterator;
        auto crbegin_lines(size_t idx = 0) const -> const_reverse_line_iterator;
        auto crend_lines(void) const   -> const const_reverse_line_iterator;
        // --- link iterator(s) -------------------------------------------
        auto cbegin_links(size_t idx = 0) const -> const_ref_iterator;
        auto cend_links(void) const -> const const_ref_iterator;
        // --- image iterator(s) ------------------------------------------
        auto cbegin_images(size_t idx = 0) const -> const_ref_iterator;
        auto cend_images(void) const -> const const_ref_iterator;

        // --- public mutator(s) ------------------------------------------
        void            clear(void);
        virtual void    redraw(size_t cols)
        {
            // do nothing
        }
        void set_title(const string& title);
        auto begin_lines(size_t idx = 0)    -> line_iterator;
        auto end_lines(void)                -> const line_iterator;
        auto rbegin_lines(size_t idx = 0)   -> reverse_line_iterator;
        auto rend_lines(void)               -> const reverse_line_iterator;
    protected:
        // --- protected member variable(s) -------------------------------
        string                          m_title         = "";
        std::vector<BufferLine>         m_buffer        = {};
        link_container_type             m_links         = {};
        image_container_type            m_images        = {};
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
