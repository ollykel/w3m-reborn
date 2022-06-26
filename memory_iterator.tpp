// === class MemoryIterator<T> ============================================

// ====== public constructor(s) ===========================================
template    <typename T>
MemoryIterator<T>::MemoryIterator(void)
{
    // do nothing; members already initialized
}// end MemoryIterator::MemoryIterator

template    <typename T>
MemoryIterator<T>::MemoryIterator(T *start, T *limit, size_t increm)
{
    m_start     = start;
    m_limit     = limit;
    m_curr      = start;
    m_increm    = increm;
}// end MemoryIterator<T>::MemoryIterator(T *start, T *limit)

template    <typename T>
MemoryIterator<T>::MemoryIterator(T *ptr, size_t len, size_t increm)
    : MemoryIterator(ptr, ptr + len, increm)
{
    // already done in constructor delegator
}// end MemoryIterator::MemoryIterator(T *ptr, size_t len)

// ====== public operator(s) ==============================================
template    <typename T>
MemoryIterator<T>::operator bool(void) const
{
    return m_curr and m_curr >= m_start and m_curr < m_limit;
}// end MemoryIterator<T>::operator bool(void) const

template    <typename T>
MemoryIterator<T>::operator T*(void) const
{
    return m_curr;
}// end MemoryIterator<T>::operator T*(void) const

template                <typename T>
T&                      MemoryIterator<T>::operator*(void) const
{
    return *m_curr;
}// end MemoryIterator<T>::operator*(void) const

template                <typename T>
T&                      MemoryIterator<T>::operator[](size_t index) const
{
    return m_curr[index];
}// end MemoryIterator<T>::operator[](size_t index) const

template                <typename T>
MemoryIterator<T>&      MemoryIterator<T>::operator++(int _)
{
    m_curr += m_increm;
    return *this;
}// end MemoryIterator<T>::operator++(int _)

template                <typename T>
MemoryIterator<T>       MemoryIterator<T>::operator++(void)
{
    auto        output      = *this;

    m_curr += m_increm;
    return output;
}// end MemoryIterator<T>::operator++(void)

template                <typename T>
MemoryIterator<T>&      MemoryIterator<T>::operator--(int _)
{
    m_curr -= m_increm;
    return *this;
}// end MemoryIterator<T>::operator--(int _)

template                <typename T>
MemoryIterator<T>       MemoryIterator<T>::operator--(void)
{
    auto        output      = *this;

    m_curr -= m_increm;
    return output;
}// end MemoryIterator<T>::operator--(void)

template                <typename T>
MemoryIterator<T>&      MemoryIterator<T>::operator+=(int increm)
{
    m_curr += increm * m_increm;
    return *this;
}// end MemoryIterator<T>::operator+=(int increm)

template                <typename T>
MemoryIterator<T>&      MemoryIterator<T>::operator-=(int increm)
{
    m_curr -= increm * m_increm;
    return *this;
}// end MemoryIterator<T>::operator-=(int increm)

// === class MemoryIterator<T>::range =====================================

// ====== public constructor(s) ===========================================
template    <typename T>
MemoryIterator<T>::range::range(void)
{
    // already initialized
}// end MemoryIterator<T>::range::range(void)

template    <typename T>
MemoryIterator<T>::range::range(T *start, T *limit, size_t increm)
    : m_start(start), m_limit(limit), m_increm(increm),
    m_sentinel(limit, limit)
{
    // already initialized
}// end MemoryIterator<T>::range::range(T *start, T *limit)

template    <typename T>
MemoryIterator<T>::range::range(T *ptr, size_t len, size_t increm)
    : range(ptr, ptr + len, increm)
{
    // already initialized
}// end MemoryIterator<T>::range::range(T *ptr, size_t len)

// ====== public accessor(s) ==============================================
template    <typename T>
MemoryIterator<T>        MemoryIterator<T>::range::begin(void) const
{
    return MemoryIterator<T>(m_start, m_limit, m_increm);
}// end MemoryIterator<T>::range::begin(void) const

template    <typename T>
const MemoryIterator<T>& MemoryIterator<T>::range::end(void) const
{
    return m_sentinel;
}// end MemoryIterator<T>::range::end(void) const

template    <typename T>
bool        operator==(
    const MemoryIterator<T>& a,
    const MemoryIterator<T>& b
)
{
    return !a or static_cast<T*>(a) == static_cast<T*>(b);
}// end operator==(const MemoryIterator<T>& a, const MemoryIterator<T>& b)

template    <typename T>
bool        operator!=(
    const MemoryIterator<T>& a,
    const MemoryIterator<T>& b
)
{
    return !(a == b);
}// end operator!=(const MemoryIterator<T>& a, const MemoryIterator<T>& b)

// === class ReverseMemoryIterator<T> =====================================

// ====== public constructor(s) ===========================================
template    <typename T>
ReverseMemoryIterator<T>::ReverseMemoryIterator(void)
{
    // do nothing; members already initialized
}// end ReverseMemoryIterator::ReverseMemoryIterator

template    <typename T>
ReverseMemoryIterator<T>::ReverseMemoryIterator(T *start, T *limit, size_t increm)
{
    m_start     = limit - 1;
    m_limit     = start - 1;
    m_curr      = m_start;
    m_increm    = increm;
}// end ReverseMemoryIterator<T>::ReverseMemoryIterator(T *start, T *limit)

template    <typename T>
ReverseMemoryIterator<T>::ReverseMemoryIterator(T *ptr, size_t len, size_t increm)
    : ReverseMemoryIterator(ptr, ptr + len, increm)
{
    // already done in constructor delegator
}// end ReverseMemoryIterator::ReverseMemoryIterator(T *ptr, size_t len)

// ====== public operator(s) ==============================================
template    <typename T>
ReverseMemoryIterator<T>::operator bool(void) const
{
    return m_curr and m_curr <= m_start and m_curr > m_limit;
}// end ReverseMemoryIterator<T>::operator bool(void) const

template    <typename T>
ReverseMemoryIterator<T>::operator T*(void) const
{
    return m_curr;
}// end ReverseMemoryIterator<T>::operator T*(void) const

template    <typename T>
T&          ReverseMemoryIterator<T>::operator*(void) const
{
    return *m_curr;
}// end ReverseMemoryIterator<T>::operator*(void) const

template    <typename T>
T&          ReverseMemoryIterator<T>::operator[](size_t index) const
{
    return *(m_curr - index);
}// end ReverseMemoryIterator<T>::operator[](size_t index) const

template                <typename T>
ReverseMemoryIterator<T>&
ReverseMemoryIterator<T>::operator++(int _)
{
    m_curr -= m_increm;
    return *this;
}// end ReverseMemoryIterator<T>::operator++(int _)

template                <typename T>
ReverseMemoryIterator<T>
ReverseMemoryIterator<T>::operator++(void)
{
    auto        output      = *this;

    m_curr -= m_increm;
    return output;
}// end ReverseMemoryIterator<T>::operator++(void)

template                <typename T>
ReverseMemoryIterator<T>&
ReverseMemoryIterator<T>::operator--(int _)
{
    m_curr += m_increm;
    return *this;
}// end ReverseMemoryIterator<T>::operator--(int _)

template                <typename T>
ReverseMemoryIterator<T>
ReverseMemoryIterator<T>::operator--(void)
{
    auto        output      = *this;

    m_curr += m_increm;
    return output;
}// end ReverseMemoryIterator<T>::operator--(void)

template                <typename T>
ReverseMemoryIterator<T>&
ReverseMemoryIterator<T>::operator+=(int increm)
{
    m_curr -= increm * m_increm;
    return *this;
}// end ReverseMemoryIterator<T>::operator+=(int increm)

template                <typename T>
ReverseMemoryIterator<T>&
ReverseMemoryIterator<T>::operator-=(int increm)
{
    m_curr += increm * m_increm;
    return *this;
}// end ReverseMemoryIterator<T>::operator-=(int increm)

// === class ReverseMemoryIterator<T>::range =====================================

// ====== public constructor(s) ===========================================
template    <typename T>
ReverseMemoryIterator<T>::range::range(void)
{
    // already initialized
}// end ReverseMemoryIterator<T>::range::range(void)

template    <typename T>
ReverseMemoryIterator<T>::range::range(T *start, T *limit, size_t increm)
    : m_start(start), m_limit(limit), m_increm(increm),
    m_sentinel(start, start)
{
    // already initialized
}// end ReverseMemoryIterator<T>::range::range(T *start, T *limit)

template    <typename T>
ReverseMemoryIterator<T>::range::range(T *ptr, size_t len, size_t increm)
    : range(ptr, ptr + len, increm)
{
    // already initialized
}// end ReverseMemoryIterator<T>::range::range(T *ptr, size_t len)

// ====== public accessor(s) ==============================================
template    <typename T>
ReverseMemoryIterator<T>
ReverseMemoryIterator<T>::range::begin(void) const
{
    return ReverseMemoryIterator<T>(m_start, m_limit, m_increm);
}// end ReverseMemoryIterator<T>::range::begin(void) const

template    <typename T>
const ReverseMemoryIterator<T>&
ReverseMemoryIterator<T>::range::end(void) const
{
    return m_sentinel;
}// end ReverseMemoryIterator<T>::range::end(void) const

template    <typename T>
bool        operator==(
    const ReverseMemoryIterator<T>& a,
    const ReverseMemoryIterator<T>& b
)
{
    return !a or static_cast<T*>(a) == static_cast<T*>(b);
}// end operator==(const ReverseMemoryIterator<T>& a, const ReverseMemoryIterator<T>& b)

template    <typename T>
bool        operator!=(
    const ReverseMemoryIterator<T>& a,
    const ReverseMemoryIterator<T>& b
)
{
    return !(a == b);
}// end operator!=(...)
