#include <vector>

#include "../deps.hpp"
#include "../memory_iterator.hpp"

// === main ===============================================================
//
// ========================================================================
int main(const int argc, const char **argv)
{
    using namespace std;

    vector<const char*>     args;

    args.reserve(argc - 1);
    for (const char **arg = argv + 1; *arg; ++arg)
    {
        args.push_back(*arg);
    }// end for (const char **arg = argv; *arg; ++arg)

    cout << "Printing args..." << endl;
    for (auto x : MemIter<const char*>::range(&args[0], args.size()))
    {
        cout << '\t' << x << endl;
    }// end for x

    cout << "Printing args in reverse..." << endl;
    for (auto x : RMemIter<const char*>::range(&args[0], args.size()))
    {
        cout << '\t' << x << endl;
    }// end for x

    cout << "Printing every other arg..." << endl;
    for (auto x : MemIter<const char*>::range(&args[0], args.size(), 2))
    {
        cout << '\t' << x << endl;
    }// end for x

    cout << "Printing every other args in reverse..." << endl;
    for (auto x : RMemIter<const char*>::range(&args[0], args.size(), 2))
    {
        cout << '\t' << x << endl;
    }// end for x

    return EXIT_SUCCESS;
}// end main(const int argc, const char **argv)
