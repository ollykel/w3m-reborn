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
};// end class DocumentHtml : public Document

#endif
