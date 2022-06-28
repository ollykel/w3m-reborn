#include "../deps.hpp"
#include "../container.hpp"

// === forward declarations ===============================================
template    <typename T>
bool        test_refs(std::vector<T>& values);

// === main ===============================================================
//
// ========================================================================
int main(const int argc, const char **argv)
{
    using namespace std;
    
    vector<const char*>     values(argv + 1, argv + argc);

    test_refs(values);

    return EXIT_SUCCESS;
}// end main

// === test definitions ===================================================
template    <typename T>
bool        test_refs(std::vector<T>& values)
{
    using namespace std;

    cont::Ref<T>    a = 1, b = 2, c = 4, d, e = 0;

    #define     RUN_TEST(REF)   \
    try   \
    {   \
        cout << "Element at index " << REF.index() << ": ";   \
        cout << values.at(REF) << endl;   \
    }   \
    catch (out_of_range _)   \
    {   \
        cout << "Caught out_of_range: ";   \
        if (not REF.is_active())   \
        {   \
            cout << "expected; reference not active" << endl;   \
        }   \
        else if (REF.index() >= values.size())   \
        {   \
            cout << "expected; index out of bounds" << endl;   \
        }   \
        else   \
        {   \
            cout << "unexpected" << endl;   \
            return false;   \
        }   \
    }

    RUN_TEST(a);
    RUN_TEST(b);
    RUN_TEST(c);
    RUN_TEST(d);
    RUN_TEST(e);

    #undef  RUN_TEST
    return true;
}// end test_refs(std::vector<T>& values)
