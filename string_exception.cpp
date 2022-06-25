#include "string_exception.hpp"

#include <iostream>

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

// === operator<<(std::ostream& outs, const StringException& e) ===========
//
// ========================================================================
std::ostream&   operator<<(std::ostream& outs, const StringException& e)
{
    return outs << e.m_text;
}// end operator<<(std::ostream& outs, const StringException& e)
