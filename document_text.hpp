#ifndef __DOCUMENT_TEXT_HPP__
#define __DOCUMENT_TEXT_HPP__

#include "deps.hpp"
#include "document.hpp"

// === class DocumentText =================================================
//
// ========================================================================
class   DocumentText : public Document
{
    public:
        // === public constructor(s) ======================================
        DocumentText(const Document::Config& cfg);// default
        DocumentText(
            const Document::Config& cfg,
            std::istream& ins,
            const size_t cols
        );// type 1
        DocumentText(
            const Document::Config& cfg,
            const string& text,
            const size_t cols
        );// type 2

        // === public mutator(s) ==========================================
        void    from_stream(
            std::istream& ins,
            const size_t cols
        );
        void    from_string(
            const string& text,
            const size_t cols
        );
        // ------ override(s) ---------------------------------------------
        void        redraw(size_t cols) override;
    protected:
        // === protected member variable(s) ===============================
        string      m_data      = "";
};// end class DocumentText : public Document

#endif
