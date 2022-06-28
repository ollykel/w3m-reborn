#ifndef __DOCUMENT_HPP__
#define __DOCUMENT_HPP__

#include <list>

#include "deps.hpp"

class   Document
{
    public:
        // === public member class(es) ====================================
        class       BufferNode;
        class       Reference;
        struct      BufferIndex;
        typedef     BufferIndex     BufIdx;
        typedef     std::vector<std::vector<BufferNode> >::iterator
                    line_iterator;
        typedef     std::vector<std::vector<BufferNode> >::reverse_iterator
                    reverse_line_iterator;
        typedef     std::vector<std::vector<BufferNode> >::const_iterator
                    const_line_iterator;
        typedef     std::vector<std::vector<BufferNode> >::const_reverse_iterator
                    const_reverse_line_iterator;
        typedef     std::vector<BufferNode>::iterator
                    node_iterator;
        typedef     std::vector<BufferNode>::const_iterator
                    const_node_iterator;

        // === public constructor(s) ======================================
        Document(void);// default

        // === public accessor(s) =========================================
        size_t                              num_lines(void) const;
        auto get_link_at(size_t index) const    -> const Reference&;
        auto get_image_at(size_t index) const   -> const Reference&;
        auto cbegin_lines(size_t idx = 0) const  -> const_line_iterator;
        auto cend_lines(void) const         -> const const_line_iterator;
        auto crbegin_lines(size_t idx = 0) const -> const_reverse_line_iterator;
        auto crend_lines(void) const   -> const const_reverse_line_iterator;

        // === public mutator(s) ==========================================
        void            clear(void);
        virtual void    redraw(size_t cols)
        {
            // do nothing
        }
        auto begin_lines(size_t idx = 0)    -> line_iterator;
        auto end_lines(void)                -> const line_iterator;
        auto rbegin_lines(size_t idx = 0)   -> reverse_line_iterator;
        auto rend_lines(void)               -> const reverse_line_iterator;
    protected:
        // === protected member variable(s) ===============================
        std::vector<std::vector<BufferNode> >       m_buffer        = {};
        std::vector<Reference>                      m_links         = {};
        std::vector<Reference>                      m_images        = {};
};// end class Document

class   Document::BufferNode
{
    public:
        // === public constructor(s) ======================================
        BufferNode(
            const string& text = "",
            const cont::Ref<Document::Reference>& link = {},
            const cont::Ref<Document::Reference>& image = {}
        );// type/default

        // === public accessor(s) =========================================
        const string&                           get_text(void) const;
        const cont::Ref<Document::Reference>&   get_link_ref(void) const;
        const cont::Ref<Document::Reference>&   get_image_ref(void) const;
    private:
        // === private member variable(s) =================================
        string                  m_text          = "";
        cont::Ref<Reference>    m_linkRef       = {};
        cont::Ref<Reference>    m_imageRef      = {};
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
        const string&                               get_url(void) const;
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
