#include "../deps.hpp"
#include "../command_parser.hpp"

// === main ===============================================================
//
// Pipe commands in through stdin, and each parsed command will be printed
// to stdout in a comma-separated, brace-enclosed list.
//
// Should be considered one unit test, to be used within a testing script.
//
// ========================================================================
int main(const int argc, const char **argv)
{
    using namespace std;

    CommandParser       parser;

    while (cin)
    {
        auto    commandTokens       = parser.parse_command(cin);

        cout << "COMMAND:\t{";
        for (auto& token : commandTokens)
        {
            cout << token << ", ";
        }// end for token
        cout << "}" << endl;
    }// end while cin

    return EXIT_SUCCESS;
}// end 
