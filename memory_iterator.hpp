#ifndef __MEMORY_ITERATOR_HPP__
#define __MEMORY_ITERATOR_HPP__

#include <cstddef>

template    <typename T>
class       MemoryIterator
{
    public:
        // === public member class(es) ====================================
        class       range;

        // === public constructor(s) ======================================
        MemoryIterator(void);// default
        MemoryIterator(T *start, T *limit, size_t increm = 1);// type
        MemoryIterator(T *ptr, size_t len, size_t increm = 1);// type

        // === public operator(s) =========================================
        operator                bool(void) const;
        operator                T*(void) const;
        T&                      operator*(void) const;
        T&                      operator[](size_t index) const;
        MemoryIterator<T>&      operator++(int _);
        MemoryIterator<T>       operator++(void);
        MemoryIterator<T>&      operator--(int _);
        MemoryIterator<T>       operator--(void);
        MemoryIterator<T>&      operator+=(int increm);
        MemoryIterator<T>&      operator-=(int increm);
    private:
        // === private member variable(s) =================================
        T       *m_curr     = nullptr;
        T       *m_start    = nullptr;
        T       *m_limit    = nullptr;
        size_t  m_increm    = 1;
};// end class MemoryIterator

template    <typename T>
class       MemoryIterator<T>::range
{
    public:
        // === public constructor(s) ======================================
        range(void);// default
        range(T *start, T *limit, size_t increm = 1);// type
        range(T *ptr, size_t len, size_t increm = 1);// type

        // === public accessor(s) =========================================
        MemoryIterator<T>        begin(void) const;
        const MemoryIterator<T>& end(void) const;
    private:
        // === private member variable(s) =================================
        T                   *m_start        = nullptr;
        T                   *m_limit        = nullptr;
        size_t              m_increm        = 1;
        MemoryIterator<T>   m_sentinel;
};// end class MemoryIterator<T>::range

template    <typename T>
bool        operator==(
    const MemoryIterator<T>& a,
    const MemoryIterator<T>& b
);

template    <typename T>
bool        operator!=(
    const MemoryIterator<T>& a,
    const MemoryIterator<T>& b
);

template    <typename T>
class       ReverseMemoryIterator
{
    public:
        // === public member class(es) ====================================
        class       range;

        // === public constructor(s) ======================================
        ReverseMemoryIterator(void);// default
        ReverseMemoryIterator(T *start, T *limit, size_t increm = 1);// type
        ReverseMemoryIterator(T *ptr, size_t len, size_t increm = 1);// type

        // === public operator(s) =========================================
        operator                    bool(void) const;
        operator                    T*(void) const;
        T&                          operator*(void) const;
        T&                          operator[](size_t index) const;
        ReverseMemoryIterator<T>&   operator++(int _);
        ReverseMemoryIterator<T>    operator++(void);
        ReverseMemoryIterator<T>&   operator--(int _);
        ReverseMemoryIterator<T>    operator--(void);
        ReverseMemoryIterator<T>&   operator+=(int increm);
        ReverseMemoryIterator<T>&   operator-=(int increm);
    private:
        // === private member variable(s) =================================
        T       *m_curr     = nullptr;
        T       *m_start    = nullptr;
        T       *m_limit    = nullptr;
        size_t  m_increm    = 1;
};// end class ReverseMemoryIterator

template    <typename T>
class       ReverseMemoryIterator<T>::range
{
    public:
        // === public constructor(s) ======================================
        range(void);// default
        range(T *start, T *limit, size_t increm = 1);// type
        range(T *ptr, size_t len, size_t increm = 1);// type

        // === public accessor(s) =========================================
        ReverseMemoryIterator<T>        begin(void) const;
        const ReverseMemoryIterator<T>& end(void) const;
    private:
        // === private member variable(s) =================================
        T                           *m_start        = nullptr;
        T                           *m_limit        = nullptr;
        size_t                      m_increm        = 1;
        ReverseMemoryIterator<T>    m_sentinel;
};// end class ReverseMemoryIterator<T>::range

template    <typename T>
bool        operator==(const MemoryIterator<T>& a, const MemoryIterator<T>& b);

template    <typename T>
bool        operator!=(const MemoryIterator<T>& a, const MemoryIterator<T>& b);

// === abbreviation(s) ====================================================
template    <typename T>
using       MemIter         = MemoryIterator<T>;

template    <typename T>
using       RMemIter        = ReverseMemoryIterator<T>;

#include "memory_iterator.tpp"

#endif
