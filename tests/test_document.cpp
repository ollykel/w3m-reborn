#include "../deps.hpp"
#include "../document.hpp"

// === Unit Test Declaration(s) ===========================================
bool        test_construct(std::ostream& outs, const Document::uri_t uri);

// === main ===============================================================
//
// ========================================================================
int main(const int argc, const char **argv)
{
    using namespace std;

    // --- Test type constructor ------------------------------------------
    return EXIT_SUCCESS;
}// end main(const int argc, const char **argv)

// === Unit Test Definition(s) ============================================
bool        test_construct(std::ostream& outs, const Document::uri_t uri)
{
    outs << "Constructing a Document with uri " << uri << "..."
        << std::endl;
    try
    {
        Document        document(uri);
    }
    catch (...)
    {
        outs << "\tinitialization threw exception" << std::endl;
        return false;
    }
    return true;
}// end test_construct(const Document::uri_t uri)
