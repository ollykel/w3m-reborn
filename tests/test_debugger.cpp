#include "../deps.hpp"
#include "../debugger.hpp"

int main(const int argc, const char **argv)
{
    using namespace std;

    if (argc != 3)
    {
        cerr << "usage: " << argv[0] << " DEBUG_FILENAME THRESHOLD" << endl;
        return EXIT_FAILURE;
    }

    const string        fname       = argv[1];
    Debugger::Config    config      = {};
    Debugger            debug;

    // settings
    config.filename = fname;
    config.prefix = "TEST";
    sscanf(argv[2], " %d", &config.limitDefault);

    debug = Debugger(config);

    try
    {
        for (int i = 0; i < 12; ++i)
        {
            debug.printf(i, "Debug #%d", i);
        }// end for i
    }
    catch (const Debugger::FileIOException& e)
    {
        cerr << "ERROR: " << e << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}// end int main
