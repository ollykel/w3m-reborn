#include <cstddef>
#include <stdexcept>

// === class Ref<T, CONT_T> ===============================================
//
// ========================================================================

// === public constructor(s) ==============================================
REF_TEMPLATE_DEC
cont::REF_T::Ref(void)
{
    // already initialized
}// end cont::REF_T::Ref(void)

REF_TEMPLATE_DEC
cont::REF_T::Ref(const size_t index)
{
    m_isActive = true;
    m_index = index;
}// end cont::REF_T::Ref(const size_t index)

// === public accessor(s) =================================================
REF_TEMPLATE_DEC
bool        cont::REF_T::is_active(void) const
{
    return m_isActive;
}// end cont::REF_T::is_active(void) const

REF_TEMPLATE_DEC
size_t      cont::REF_T::index(void) const
{
    return m_index;
}// end cont::REF_T::index(void) const

REF_TEMPLATE_DEC
CONT_TEMPLATE_DEC
const T&    cont::REF_T::in(const CONT_T& ctr) const
{
    if (not is_active())
    {
        throw std::out_of_range("Ref inactive");
    }
    return ctr.at(index());
}// end cont::REF_T::in(const CONT_T& ctr) const

REF_TEMPLATE_DEC
CONT_TEMPLATE_DEC
T&          cont::REF_T::in(CONT_T& ctr) const
{
    if (not is_active())
    {
        throw std::out_of_range("Ref inactive");
    }
    return ctr.at(index());
}// end cont::REF_T::in(CONT_T& ctr) const

REF_TEMPLATE_DEC
cont::REF_T::operator    bool(void) const
{
    return is_active();
}// end cont::REF_T::operator    bool(void) const

REF_TEMPLATE_DEC
cont::REF_T::operator   size_t(void) const
{
    if (not is_active())
    {
        throw std::out_of_range("Ref inactive");
    }
    return index();
}// end cont::REF_T::operator   size_t(void) const

// === public mutator(s) ==================================================
REF_TEMPLATE_DEC
void        cont::REF_T::clear(void)
{
    m_isActive = false;
    m_index = 0;
}// end cont::REF_T::clear(void)

REF_TEMPLATE_DEC
cont::REF_T&    cont::REF_T::operator=(const size_t index)
{
    m_isActive = true;
    m_index = index;
}// end cont::REF_T::operator=(const size_t index)
