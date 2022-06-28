#include <cstddef>
#include <stdexcept>

#include "container.hpp"

// === class Ref<T> =======================================================
//
// ========================================================================

// === public constructor(s) ==============================================
cont::Ref::Ref(void)
{
    // already initialized
}// end cont::Ref::Ref(void)

cont::Ref::Ref(const size_t index)
{
    m_isActive = true;
    m_index = index;
}// end cont::Ref::Ref(const size_t index)

// === public accessor(s) =================================================
bool        cont::Ref::is_active(void) const
{
    return m_isActive;
}// end cont::Ref::is_active(void) const

size_t      cont::Ref::index(void) const
{
    return m_index;
}// end cont::Ref::index(void) const

cont::Ref::operator     bool(void) const
{
    return is_active();
}// end cont::Ref::operator    bool(void) const

cont::Ref::operator     size_t(void) const
{
    if (not is_active())
    {
        throw std::out_of_range("Ref inactive");
    }
    return index();
}// end cont::Ref::operator   size_t(void) const

// === public mutator(s) ==================================================
void        cont::Ref::clear(void)
{
    m_isActive = false;
    m_index = 0;
}// end cont::Ref::clear(void)

cont::Ref&    cont::Ref::operator=(const size_t index)
{
    m_isActive = true;
    m_index = index;
    return *this;
}// end cont::Ref::operator=(const size_t index)
