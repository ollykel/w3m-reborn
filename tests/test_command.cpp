#include <iomanip>

#include "../deps.hpp"
#include "../command.hpp"

int main(const int argc, const char **argv)
{
    using namespace std;

    {
        const string        cmdName     = "ls";

        Command     cmd(cmdName);

        cout << "Running command \"" << cmdName << "\" (unpiped)..."
            << endl;
        cout << "===" << endl;
        cout << "Process exit: " << cmd.spawn().wait() << endl;

        cout << "===" << endl;
        cout << "Running command \"" << cmdName << "\" (piped)..."
            << endl;

        auto            sproc   = cmd.set_stdout_piped(true).spawn();
        vector<string>  lines   = {};

        while (sproc.stdout())
        {
            lines.emplace_back();
            getline(sproc.stdout(), lines.back());
            cout << "(line: " << lines.back() << ")" << endl;
        }// end while

        sproc.stdout().close();
        sproc.wait();

        if (not lines.empty() and lines.back().empty())
            lines.pop_back();

        for (size_t i = 0; i < lines.size(); ++i)
        {
            cout << "\tLine " << i + 1 << ": " << lines[i] << endl;
        }// end for i
    }

    // test cat (using stdin)
    {
        const string    cmdName
                        = "cat --number";
        auto            sproc
                        = Command(cmdName).set_stdin_piped(true).spawn();

        cout << "Testing command \"" << cmdName << "\" (piping to stdin)..."
            << endl;

        sproc.stdin().setf(ios::left);
        sproc.stdin() << "\tHello World from cat!" << endl;
        for (int i = 1; i < 6; ++i)
        {
            int     val     = i * i;

            sproc.stdin() << '\t';
            for (int j = 0; j < 3; ++j)
            {
                sproc.stdin() << setw(8) << val++ << ' ';
            }// end for j
            sproc.stdin() << endl;
        }// end for i
        sproc.stdin().close();

        cout << "Process exited with status " << sproc.wait() << endl;
    }
    return EXIT_SUCCESS;
}// end main
