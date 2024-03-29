#ifndef __DOCUMENT_HPP__
#define __DOCUMENT_HPP__

#include <list>
#include <map>
#include <unordered_set>

#include "deps.hpp"
#include "debugger.hpp"
#include "dom_tree.hpp"

class   Document
{
    public:
        // --- public member type(s) --------------------------------------
        struct  Config
        {
            struct
            {
                size_t      def;
                size_t      min;
                size_t      max;
            } inputWidth;
        };// end struct Document::Config
        class       BufferNode;
        class       Reference;
        class       Form;
        class       FormInput
        {
            friend class Document;

            public:
                // --- public member type(s) ------------------------------

                // enum class Type
                #include "document_FormInput_Type_enum.gen.hpp"

                // --- public static method(s) ----------------------------
                static auto type(const string& str)
                    -> Type;

                // --- public accessor(s) ---------------------------------
                auto    form(void) const
                    -> Document::Form&;
                auto    type(void) const
                    -> Type;
                auto    name(void) const
                    -> const string&;
                auto    value(void) const
                    -> const string&;
                auto    is_active(void) const
                    -> bool;
                
                // --- public mutator(s) ----------------------------------
                void    set_type(Type t);
                void    set_name(const string& name);
                void    set_value(const string& value);
                void    set_is_active(bool state);
                void    push_buffer_node(Document::BufferNode* bufNode);
                void    clear_buffer_nodes(void);
            private:
                // --- private member variable(s) -------------------------
                Document                *m_parent       = nullptr;
                size_t                  m_index         = SIZE_MAX;
                size_t                  m_formIndex     = 0;
                Type                    m_type          = Type::text;
                DomTree::node           *m_domNode      = nullptr;
                std::vector<Document::BufferNode*>
                                        m_bufNodes      = {};
                string                  m_name          = "";
                string                  m_value         = "";
                bool                    m_isActive      = false;

                // --- private constructors -------------------------------
                FormInput(
                    Document&               parent,
                    size_t                  index,
                    size_t                  formIndex,
                    Type                    type        = Type::text,
                    DomTree::node           *domNode    = nullptr,
                    string                  name        = "",
                    string                  value       = ""
                );
        };// end class Document::FormInput
        struct      BufferIndex;
        struct      buffer_index_type
        {
            // --- member variables ---------------------------------------
            size_t      line;
            size_t      node;

            // --- operator overloaders -----------------------------------
            operator bool(void) const;
        };// end buffer_index_type
        class       buffer_node_iterator;
        class       buffer_node_const_iterator;
        enum class  BufPos
        {
            begin   = 0,
            end     = 1,
        };

        typedef     BufferIndex                     BufIdx;
        typedef     std::vector<BufferNode>         BufferLine;
        typedef     std::vector<BufferLine>         buffer_type;
        typedef     std::vector<Reference>          link_container;
        typedef     std::vector<Reference>          image_container;
        typedef     Form                            form_type;
        typedef     std::vector<form_type>          form_container;
        typedef     std::vector<FormInput>          form_input_container;

        // --- public static functions ------------------------------------
        static void set_debugger_filename(const string& fname);
        static void set_debugger_limit(int limit);

        // --- public constructor(s) --------------------------------------
        Document(const Config& cfg);// default

        // --- public accessor(s) -----------------------------------------
        auto title(void) const
            -> const string&;
        auto buffer(void) const
            -> const buffer_type&;
        auto buffer_string(void) const
            -> string;
        auto links(void) const
            -> const link_container&;
        auto images(void) const
            -> const image_container&;
        auto forms(void) const
            -> const form_container&;
        auto form_inputs(void) const
            -> const form_input_container&;
        auto get_section_index(const string& id) const
            -> buffer_index_type;
        auto buffer_const_iter(BufPos pos) const
            -> buffer_node_const_iterator;
        auto buffer_const_iter(size_t lineIdx, size_t nodeIdx) const
            -> buffer_node_const_iterator;

        // --- public mutator(s) ------------------------------------------
        void            clear(void);
        virtual void    redraw(size_t cols)
        {
            // do nothing
        }
        void set_title(const string& title);
        auto forms(void)
            -> form_container::iterator;
        auto form_inputs(void)
            -> form_input_container::iterator;
    protected:
        // --- protected member types -------------------------------------
        typedef     std::map<string,buffer_index_type>      section_map;
        
        // --- protected member variable(s) -------------------------------
        Config                  m_config        = {};
        string                  m_title         = "";
        buffer_type             m_buffer        = {};
        link_container          m_links         = {};
        image_container         m_images        = {};
        form_container          m_forms         = {};
        form_input_container    m_form_inputs   = {};
        section_map             m_sections      = {};

        // --- protected static functions ---------------------------------
        static auto debugger(void)
            -> Debugger&;

        // --- protected mutator(s) ---------------------------------------
        auto    emplace_form(string action = "", string method = "")
            -> Form&;
        auto    emplace_form_input(
                size_t              formIndex,
                FormInput::Type     type        = FormInput::Type::text,
                DomTree::node       *domNode    = nullptr,
                string              name        = "",
                string              value       = ""
            ) -> FormInput&;
        auto buffer_iter(BufPos pos)
            -> buffer_node_iterator;
        auto buffer_iter(size_t lineIdx, size_t nodeIdx)
            -> buffer_node_iterator;
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
            const wstring& text = wstring(),
            const bool isReserved = false,
            const cont::Ref& link = {},
            const cont::Ref& image = {},
            const cont::Ref& input = {}
        );// type/default

        // --- public accessor(s) -----------------------------------------
        auto text(void) const
            -> const wstring&;
        auto reserved(void) const
            -> bool;
        auto link_ref(void) const
            -> const cont::Ref&;
        auto image_ref(void) const
            -> const cont::Ref&;
        auto input_ref(void) const
            -> const cont::Ref&;
        auto stylers(void) const
            -> const std::vector<string>&;

        // --- public mutator(s) ------------------------------------------
        void set_text(const wstring& text);
        void set_reserved(const bool state);
        void set_link_ref(const size_t index);
        void set_image_ref(const size_t index);
        void set_input_ref(const size_t index);
        void append_styler(const string& styler);
        void clear_text(void);
        void clear_link_ref(void);
        void clear_image_ref(void);
        void clear_stylers(void);
    protected:
        // --- protected member variable(s) -------------------------------
        wstring                 m_text          = wstring();
        bool                    m_isReserved    = false;
        styler_container        m_stylers       = {};
        cont::Ref               m_linkRef       = {};
        cont::Ref               m_imageRef      = {};
        cont::Ref               m_inputRef      = {};
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
    friend class FormInput;

    public:
        // --- public member type(s) --------------------------------------
        typedef     std::unordered_set<size_t>      input_idx_container;
        typedef     std::map<string,input_idx_container>
                                                    input_idx_container_map;
        typedef     std::unordered_set<size_t>      input_index_container;

        // --- public constructor(s) --------------------------------------
        Form(
            Document&   parent,
            string      action      = "",
            string      method      = ""
        );

        // --- public accessor(s) -----------------------------------------
        auto    parent(void) const
            -> const Document*;
        auto    action(void) const
            -> const string&;
        auto    method(void) const
            -> const string&;
        auto    active_inputs(void) const
            -> const input_idx_container_map&;
        auto    value(const string& key) const
            -> string;
        auto    values(void) const
            -> std::map<string,string>;

        // --- public mutator(s) ------------------------------------------
        void    set_action(const string& action);
        void    set_method(const string& method);

    private:
        // --- private member variable(s) ---------------------------------
        Document                    *m_parent           = nullptr;
        string                      m_action            = "";
        string                      m_method            = "";
        input_idx_container_map     m_activeInputs      = {};
        input_index_container       m_input_indices     = {};

        // --- private mutators -------------------------------------------
        void    insert_input(size_t index);
        void    erase_inputs(const string& key);
        void    remove_input(size_t index);
        void    clear_active_inputs(void);
};// end class Document::Form

struct      Document::BufferIndex
{
    // --- public member variable(s) --------------------------------------
    size_t      line;
    size_t      col;

    // --- public constructor(s) ------------------------------------------
    BufferIndex(size_t lnum, size_t cnum);
};// end struct Document::BufferIndex

class       Document::buffer_node_iterator
{
    friend class Document;

    public:
        // --- public member types ----------------------------------------
        typedef     buffer_node_iterator        type;

        // --- public constructors ----------------------------------------
        buffer_node_iterator(void); // void

        // --- public accessors -------------------------------------------
        operator bool(void) const;
        auto operator*(void) const
            -> Document::BufferNode&;
        auto line_index(void) const
            -> size_t;
        auto line(void) const
            -> Document::BufferLine&;
        auto node_index(void) const
            -> size_t;
        auto node(void) const
            -> Document::BufferNode&;
        auto column(void) const
            -> size_t;
        auto at_line_end(void) const
            -> bool;

        // --- public mutators --------------------------------------------
        auto operator++(void)
            -> type&;
        auto operator++(int _)
            -> type;

        // --- friend functions -------------------------------------------
        friend auto operator==(const type& a, const type& b)
            -> bool;
        friend auto operator!=(const type& a, const type& b)
            -> bool;
    private:
        // --- private member variables -----------------------------------
        Document::buffer_type   *m_buffer       = nullptr;
        size_t                  m_lineIdx       = 0;
        size_t                  m_nodeIdx       = 0;
        size_t                  m_column        = 0;

        // --- private constructors ---------------------------------------
        buffer_node_iterator(
            Document::buffer_type& buffer,
            size_t lineIdx,
            size_t nodeIdx,
            size_t column
        );
};// end class Document::buffer_node_iterator

class       Document::buffer_node_const_iterator
{
    friend class Document;

    public:
        // --- public member types ----------------------------------------
        typedef     buffer_node_const_iterator      type;
        typedef     const Document::BufferNode&     reference_type;

        // --- public constructors ----------------------------------------
        buffer_node_const_iterator(void); // void

        // --- public accessors -------------------------------------------
        operator bool(void) const;
        auto operator*(void) const
            -> reference_type;
        auto line_index(void) const
            -> size_t;
        auto line(void) const
            -> const Document::BufferLine&;
        auto node_index(void) const
            -> size_t;
        auto node(void) const
            -> reference_type;
        auto column(void) const
            -> size_t;
        auto at_line_end(void) const
            -> bool;

        // --- public mutators --------------------------------------------
        auto operator++(void)
            -> type&;
        auto operator++(int _)
            -> type;

        // --- friend functions -------------------------------------------
        friend auto operator==(const type& a, const type& b)
            -> bool;
        friend auto operator!=(const type& a, const type& b)
            -> bool;
    private:
        // --- private member variables -----------------------------------
        const Document::buffer_type     *m_buffer       = nullptr;
        size_t                          m_lineIdx       = 0;
        size_t                          m_nodeIdx       = 0;
        size_t                          m_column        = 0;

        // --- private constructors ---------------------------------------
        buffer_node_const_iterator(
            const Document::buffer_type& buffer,
            size_t lineIdx,
            size_t nodeIdx,
            size_t column
        );
};// end class Document::buffer_node_const_iterator

#endif
