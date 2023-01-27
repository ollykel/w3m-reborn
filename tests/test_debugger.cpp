#include <time.h>

#include "../deps.hpp"
#include "../debugger.hpp"

#define     BUFFER_LEN      0x100

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
    cout << "Debugger initialized, with limit level "
        << config.limitDefault << endl;

    try
    {
        time_t          currTime                = time(nullptr);
        char            timeBuf[BUFFER_LEN]     = "";

        strftime(
            timeBuf,
            BUFFER_LEN,
            "%a, %d %b %Y %T %z",
            localtime(&currTime)
        );
        debug.printf(0, "Beginning output at %s...", timeBuf);

        for (int i = 0; i < 12; ++i)
        {
            cout << "\tPrinting debug at level " << i << "..." << endl;
            debug.printf(i, "Debug #%d", i);
        }// end for i
    }
    catch (const Debugger::FileIOException& e)
    {
        cerr << "ERROR: " << e << endl;
        return EXIT_FAILURE;
    }

    cout << "Read debugger output at \"" << fname << "\"" << endl;

    return EXIT_SUCCESS;
}// end int main
