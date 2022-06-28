#ifndef __CONTAINER_HPP__
#define __CONTAINER_HPP__

#include <cstddef>

#define     REF_T               Ref<T>
#define     REF_TEMPLATE_DEC    template <typename T>
#define     REF_T2              Ref<T,CONT_T>
#define     REF_TEMPLATE_DEC2   template <typename T, typename CONT_T>
#define     CONT_TEMPLATE_DEC   template <typename CONT_T>

namespace   cont
{

// === class Ref<T> =======================================================
//
// Contains a "conditional reference" to an object.
// May be either active or inactive.
//
// ========================================================================
REF_TEMPLATE_DEC
class           Ref
{
    public:
        // === public constructor(s) ======================================
        Ref(void);// default
        Ref(const size_t index);// type 1

        // === public accessor(s) =========================================
        bool        is_active(void) const;
        size_t      index(void) const;

        CONT_TEMPLATE_DEC
        const T&    in(const CONT_T& ctr) const;

        CONT_TEMPLATE_DEC
        T&          in(CONT_T& ctr) const;
        operator    bool(void) const;
        operator    size_t(void) const;

        // === public mutator(s) ==========================================
        void        clear(void);
        REF_T&      operator=(const size_t index);
    protected:
        // === protected member variable(s) ===============================
        bool        m_isActive      = false;
        size_t      m_index         = 0;
};// end class Ref<T>

};// end namespace   container

#include "container.tpp"

#undef REF_T
#undef REF_TEMPLATE_DEC
#undef REF_T2
#undef REF_TEMPLATE_DEC2
#undef CONT_TEMPLATE_DEC2

#endif
