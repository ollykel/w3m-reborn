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
        cout << "Testing command \"cat\" (piping to stdin)..." << endl;

        auto    sproc   = Command("cat").set_stdin_piped(true).spawn();

        sproc.stdin() << "Allo Welt!" << "\n";
        sproc.stdin().close();

        cout << "Process exited with status " << sproc.wait() << endl;
    }
    return EXIT_SUCCESS;
}// end main
