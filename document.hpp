#ifndef __DOCUMENT_HPP__
#define __DOCUMENT_HPP__

#include <list>
#include <map>
#include <unordered_set>

#include "deps.hpp"

class   Document
{
    public:
        // --- public member type(s) --------------------------------------
        class       BufferNode;
        class       Reference;
        class       Form;
        class       FormInput;
        struct      BufferIndex;

        typedef     BufferIndex                     BufIdx;
        typedef     std::vector<BufferNode>         BufferLine;
        typedef     std::vector<BufferLine>         buffer_type;
        typedef     std::vector<Reference>          link_container;
        typedef     std::vector<Reference>          image_container;
        typedef     std::map<string, string>        form_type;
        typedef     std::vector<form_type>          form_container;

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
        auto forms(void) const
            -> const form_container&;

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
        form_container      m_forms         = {};
};// end class Document

class   Document::BufferNode
{
    friend class Document;
    friend class DocumentText;
    friend class DocumentHtml;

    public:
        // --- public member type(s) --------------------------------------
        typedef     string                          styler_type;
        typedef     std::vector<styler_type>        styler_container;

        // --- public constructor(s) --------------------------------------
        BufferNode(
            const string& text = "",
            const bool isReserved = false,
            const cont::Ref& link = {},
            const cont::Ref& image = {}
        );// type/default

        // --- public accessor(s) -----------------------------------------
        auto text(void) const
            -> const string&;
        auto reserved(void) const
            -> bool;
        auto link_ref(void) const
            -> const cont::Ref&;
        auto image_ref(void) const
            -> const cont::Ref&;
        auto stylers(void) const
            -> const std::vector<string>&;

        // --- public mutator(s) ------------------------------------------
        void set_text(const string& text);
        void set_reserved(const bool state);
        void set_link_ref(const size_t index);
        void set_image_ref(const size_t index);
        void append_styler(const string& styler);
        void clear_text(void);
        void clear_link_ref(void);
        void clear_image_ref(void);
        void clear_stylers(void);
    protected:
        // --- protected member variable(s) -------------------------------
        string                  m_text          = "";
        bool                    m_isReserved    = false;
        styler_container        m_stylers       = {};
        cont::Ref               m_linkRef       = {};
        cont::Ref               m_imageRef      = {};
};// end class Document::BufferNode

class   Document::Reference
{
    public:
        // === public member class(es) ====================================
        typedef     std::list<Document::BufferIndex>    referer_container;

        // === public constructor(s) ======================================
        Reference(const string& url = "");

        // === public accessor(s) =========================================
        auto get_url(void) const
            -> const string&;
        auto referers(void) const
            -> const referer_container&;

        // === public mutator(s) ==========================================
        void set_url(const string& url);
        void append_referer(size_t line, size_t col);
        void clear_referers(void);
    private:
        // === private member variable(s) =================================
        string                              m_url       = "";
        std::list<Document::BufferIndex>    m_referers  = {};
};// end class Document::Reference

class       Document::Form
{
    public:
        // === public member type(s) ======================================
        typedef     std::vector<FormInput*>         input_ptr_container;
        typedef     std::unordered_set<size_t>      input_index_container;

        // === public accessor(s) =========================================
        auto    parent(void) const
            -> const Document*;
        auto    action(void) const
            -> const string&;
        auto    method(void) const
            -> const string&;
        auto    inputs(void) const
            -> input_ptr_container;

        // === public mutator(s) ==========================================
        auto    input_indices(void)
            -> input_index_container&;
        void    set_action(const string& action);
        void    set_method(const string& method);

    private:
        // === private member variable(s) =================================
        Document                    *m_parent           = nullptr;
        string                      m_action            = "";
        string                      m_method            = "";
        input_index_container       m_input_indices     = {};

        // === private constructor(s) =====================================
        Form(
            Document *parent,
            string  action       = "",
            string  method      = ""
        );
};// end class Document::Form

class       Document::FormInput
{
    public:
        // === public member type(s) ======================================
        enum class  Type
        {
            text = 0,
            button,
            checkbox,
            color,
            date,
            datetime_local,
            email,
            file,
            hidden,
            image,
            month,
            number,
            password,
            radio,
            range,
            reset,
            search,
            submit,
            tel,
            time,
            url,
            week,
        };// end enum class Type
    private:
        // === private member variable(s) =================================
        Type        m_type      = Type::text;
        string      m_name      = "";
        string      m_value     = "";
};// end class Document::FormInput

struct      Document::BufferIndex
{
    // === public member variable(s) ======================================
    size_t      line;
    size_t      col;

    // === public constructor(s) ==========================================
    BufferIndex(size_t lnum, size_t cnum);
};// end struct Document::BufferIndex

#endif
