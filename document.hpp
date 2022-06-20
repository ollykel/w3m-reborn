#ifndef __DOCUMENT_HPP__
#define __DOCUMENT_HPP__

#include "deps.hpp"

#include <vector>

// === class Document =====================================================
//
// ========================================================================
class   Document
{
    public:
        // === public member class(es) ====================================
        typedef     string      uri_t;

        // === public constructor(s) ======================================
        Document(const uri_t uri);
    private:
        // === private member variable(s) =================================
        uri_t                   m_uri;
        std::vector<char>       m_data;
};// end class Document

#endif
