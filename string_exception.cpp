#include "string_exception.hpp"

// === StringException::operator const std::string&(void) const ===========
//
// ========================================================================
StringException::operator const std::string&(void) const
{
    return m_text;
}// end StringException::operator const std::string&(void) const

// === StringException::set_text(const std::string& text) =================
//
// ========================================================================
void        StringException::set_text(const std::string& text)
{
    m_text = text;
}// end StringException::set_text(const std::string& text)
