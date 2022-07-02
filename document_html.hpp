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
        // ------ override(s) ---------------------------------------------
        void        redraw(size_t cols) override;
    protected:
        // === protected member variable(s) ===============================
        string      m_data      = "";
        DomTree     m_dom       = {};
        size_t      m_tabWidth  = 4;// TODO: read from config
        std::map<
            string,
            void (DocumentHtml::*)(const DomTree::node&, const size_t)
        >           m_dispatcher;

        // === protected mutator(s) =======================================
        void    append_node(const DomTree::node& nd, const size_t cols);
        void    append_children(const DomTree::node& nd, const size_t cols);
        void    append_text(const DomTree::node& text, const size_t cols);
        void    append_a(const DomTree::node& a, const size_t cols);
        void    append_br(const DomTree::node& br, const size_t cols);
        void    append_form(const DomTree::node& form, const size_t cols);
        void    append_hn(const DomTree::node& hn, const size_t cols);
        void    append_hr(const DomTree::node& hr, const size_t cols);
        void    append_img(const DomTree::node& img, const size_t cols);
        void    append_ul(const DomTree::node& ul, const size_t cols);
        void    append_ol(const DomTree::node& ol, const size_t cols);
        void    append_li_ul(const DomTree::node& li, const size_t cols);
        void    append_li_ol(
                    const DomTree::node& li,
                    const size_t cols,
                    const size_t index
                );
        void    append_p(const DomTree::node& p, const size_t cols);
        void    append_table(const DomTree::node& table, const size_t cols);
        void    append_tbody(const DomTree::node& tbody, const size_t cols);
        void    append_other(const DomTree::node& nd, const size_t cols);

        // === protected static function(s) ===============================
        static size_t       line_length(BufferLine& line);
        static bool         is_node_header(const DomTree::node& nd);
        static unsigned     parse_html_entity(const string& id);
        static string       decode_text(const string& text);
};// end class DocumentHtml : public Document

#endif
