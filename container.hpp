#ifndef __CONTAINER_HPP__
#define __CONTAINER_HPP__

#include <cstddef>

namespace   cont
{

// === class Ref<T> =======================================================
//
// Contains a "conditional reference" to an object.
// May be either active or inactive.
//
// ========================================================================
class           Ref
{
    public:
        // === public constructor(s) ======================================
        Ref(void);// default
        Ref(const size_t index);// type 1

        // === public accessor(s) =========================================
        bool        is_active(void) const;
        size_t      index(void) const;
        operator    bool(void) const;
        operator    size_t(void) const;

        // === public mutator(s) ==========================================
        void        clear(void);
        Ref&        operator=(const size_t index);
    protected:
        // === protected member variable(s) ===============================
        bool        m_isActive      = false;
        size_t      m_index         = 0;
};// end class Ref<T>

};// end namespace   container

#endif
