// === class ofdstream Implementation =====================================
//
// ========================================================================

template <typename T>
auto ofdstream::operator<<(const T& in)
    -> ofdstream&
{
    m_formatter << in;
    write(m_formatter.str());
    m_formatter.str("");
    return *this;
}// end ofdstream::operator<<
