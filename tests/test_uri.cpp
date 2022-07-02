#include "../deps.hpp"
#include "../uri.hpp"

// === main ===============================================================
//
// ========================================================================
int main(const int argc, const char **argv)
{
    using namespace std;

    const string    str     = argc > 1 ?
                    argv[1] :
                    "https://www.google.com/search?q=Hello+World#content";
    const Uri   uri(str);
    
    cout << "Original uri string: " << str << endl;
    cout << "Parsed uri: " << uri << endl;
    cout << "Re-encoded uri: " << uri.str() << endl;

    return EXIT_SUCCESS;
}// end main
