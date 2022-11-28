#include <cstdio>
#include <climits>

#include <curses.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../deps.hpp"
#include "../html_parser.hpp"
#include "../dom_tree.hpp"
#include "../document_html.hpp"

struct Config
{
    string                  initUrl;
    Document::Config        document;
};// end struct Config

// === Function Prototypes ================================================
int runtime(const Config& cfg);

// === main ===============================================================
//
// ========================================================================
int main(const int argc, const char **argv, const char **envp)
{
    using namespace std;

    int         ret         = EXIT_FAILURE;
    int         wstatus;
    pid_t       child;
    Config      config      = {
        // initUrl
        "",
        {
            // inputWidth
            {
                40,         // def
                0,          // min
                SIZE_MAX,   // max
            },
        },
    };

    // get url
    if (argc > 1)
    {
        config.initUrl = argv[1];
    }
    else
    {
        const char  *url    = getenv("WWW_HOME");

        if (url)
        {
            config.initUrl = url;
        }
    }

    // initialize screen
    initscr();
    cbreak();
    noecho();

    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    wtimeout(stdscr, 100);
    curs_set(0);
    start_color();

    switch ((child = fork()))
    {
        case 0:
            // we are child process
            return runtime(config);
        case -1:
            // could not fork
            cerr << "ERROR: could not spawn child process" << endl;
            ret = EXIT_FAILURE;
        default:
            // we are parent
            waitpid(child, &wstatus, 0x00);

            ret = WEXITSTATUS(wstatus);
    }// end switch

finally:
    // reset screen
    use_default_colors();
    curs_set(1);
    endwin();

    return ret;
}// end main

// === Function Definitions ===============================================

int runtime(const Config& cfg)
{
    using namespace std;

    const string    msg01   = "Hello World!";
    const string    msg02   = "Press 'q' to exit.";

    WINDOW  *page;
    char    key;

    // TODO: meaningfully implement

    werase(stdscr);

    if (not cfg.initUrl.size())
    {
        waddnstr(stdscr, "ERROR: no url given", COLS);
        wrefresh(stdscr);

        sleep(3);
        return EXIT_FAILURE;
    }

    // print messages
    for (int i = 0; i < msg01.size(); ++i)
    {
        mvwaddch(stdscr, 0, i, msg01[i]);
    }// end for
    for (int i = 0; i < msg02.size(); ++i)
    {
        mvwaddch(stdscr, 1, i, msg02[i]);
    }// end for
    wrefresh(stdscr);

    // wait for keypress
    while (true)
    {
        switch ((key = wgetch(stdscr)))
        {
            case 'q':
            case 'Q':
                return EXIT_SUCCESS;
        }// end switch
    }// end while

    return EXIT_SUCCESS;
}// end runtime
