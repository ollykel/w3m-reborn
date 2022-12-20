#ifndef __DOCUMENT_HTML_HPP__
#define __DOCUMENT_HTML_HPP__

#include "deps.hpp"
#include "dom_tree.hpp"
#include "document.hpp"

// === class DocumentHtml =================================================
//
// ========================================================================
class   DocumentHtml : public Document
{
    public:
        // === public constructor(s) ======================================
        DocumentHtml(const Document::Config& cfg);// default
        DocumentHtml(
            const Document::Config& cfg,
            std::istream& ins,
            const size_t cols
        );// type 1
        DocumentHtml(
            const Document::Config& cfg,
            const string& text,
            const size_t cols
        );// type 2

        // === public mutator(s) ==========================================
        void        from_stream(std::istream& ins, const size_t cols);
        void        from_string(const string& text, const size_t cols);
        void        parse_title_from_data(void);
        // ------ override(s) ---------------------------------------------
        void        redraw(size_t cols) override;
    protected:
        // === protected member type(s) ===================================
        class   Format;
        struct  Stacks
        {
            std::vector<size_t>     formIndices;
            std::vector<string>     styles;
        };

        // === protected member variable(s) ===============================
        string      m_data      = "";
        DomTree     m_dom       = {};
        size_t      m_tabWidth  = 4;// TODO: read from config
        std::map<
            string,
            void (DocumentHtml::*)(
                DomTree::node&,
                const size_t,
                Format,
                Stacks&)
        >           m_dispatcher;

        // === protected mutator(s) =======================================
        void    append_node(
            DomTree::node& nd,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_children(
            DomTree::node& nd,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_str(
            const string& str,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_text(
            DomTree::node& text,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_a(
            DomTree::node& a,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_br(
            DomTree::node& br,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_div(
            DomTree::node& div,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_form(
            DomTree::node& form,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_hn(
            DomTree::node& hn,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_hr(
            DomTree::node& hr,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_img(
            DomTree::node& img,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_input(
            DomTree::node& input,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_ul(
            DomTree::node& ul,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_ol(
            DomTree::node& ol,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_li_ul(
            DomTree::node& li,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_li_ol(
            DomTree::node& li,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_p(
            DomTree::node& p,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_table(
            DomTree::node& table,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_tbody(
            DomTree::node& tbody,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_other(
            DomTree::node& nd,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    begin_block(const size_t cols, Format& fmt);

        // === protected static function(s) ===============================
        static size_t       line_length(BufferLine& line);
        static bool         is_node_header(const DomTree::node& nd);
        static wchar_t      parse_html_entity(const string& id);
        static wstring      decode_text(const string& text);
};// end class DocumentHtml : public Document

// === class DocumentHtml::Format =========================================
//
// Keeps track of formatting information for parsing html elements.
// Should be passed by value in each recursive call, to avoid side effects
// in subsequent function calls.
//
// ========================================================================
class   DocumentHtml::Format
{
    public:
        // === public member type(s) ======================================
        enum class  Align
        {
            left = 0,
            center,
            right
        };// end enum class Align

        // === public member variable(s) ==================================
        unsigned    indent      = 0;
        unsigned    listIndex   = 0;
        unsigned    listLevel   = 0;
        Align       align       = Align::left;

        // === public accessor(s) =========================================
        bool        in_list(void) const;
        bool        ordered_list(void) const;
        bool        preformatted(void) const;
        bool        block_ignore(void) const;

        // === public mutator(s) ==========================================
        void        set_in_list(bool state);
        void        set_ordered_list(bool state);
        void        set_preformatted(bool state);
        void        set_block_ignore(bool state);
    protected:
        // === protected member variable(s) ===============================
        unsigned    m_flags     = 0;

        // === protected static variable(s) ===============================
        const static unsigned   F_IN_LIST       = 1;
        const static unsigned   F_ORDERED_LIST  = 2;
        const static unsigned   F_PREFORMATTED  = 4;
        const static unsigned   F_BLOCK_IGNORE  = 8;
};// end class DocumentHtml::Format

#endif
