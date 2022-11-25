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
        DocumentHtml(void);// default
        DocumentHtml(std::istream& ins, const size_t cols);// type 1
        DocumentHtml(const string& text, const size_t cols);// type 2

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
                const DomTree::node&,
                const size_t,
                Format,
                Stacks&)
        >           m_dispatcher;

        // === protected mutator(s) =======================================
        void    append_node(
            const DomTree::node& nd,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_children(
            const DomTree::node& nd,
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
            const DomTree::node& text,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_a(
            const DomTree::node& a,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_br(
            const DomTree::node& br,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_div(
            const DomTree::node& div,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_form(
            const DomTree::node& form,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_hn(
            const DomTree::node& hn,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_hr(
            const DomTree::node& hr,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_img(
            const DomTree::node& img,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_input(
            const DomTree::node& input,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_ul(
            const DomTree::node& ul,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_ol(
            const DomTree::node& ol,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_li_ul(
            const DomTree::node& li,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_li_ol(
            const DomTree::node& li,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_p(
            const DomTree::node& p,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_table(
            const DomTree::node& table,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_tbody(
            const DomTree::node& tbody,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    append_other(
            const DomTree::node& nd,
            const size_t cols,
            Format fmt,
            Stacks& stacks
        );
        void    begin_block(const size_t cols, Format& fmt);

        // === protected static function(s) ===============================
        static size_t       line_length(BufferLine& line);
        static bool         is_node_header(const DomTree::node& nd);
        static unsigned     parse_html_entity(const string& id);
        static string       decode_text(const string& text);
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
