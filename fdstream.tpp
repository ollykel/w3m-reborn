// === class ifdstream Implementation =====================================
//
// ========================================================================

template <typename T>
auto ifdstream::operator>>(T& out)
    -> ifdstream&
{
    using namespace std;

    string      buf     = "";

    while (not eof() and isspace(peek()))
    {
        ignore(1);
    }// end while

    while (not eof() and not isspace(peek()))
    {
        buf += get();
    }// end while

    istringstream       stream(buf);

    stream >> out;

    return *this;
}// end ifdstream::operator>>

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
