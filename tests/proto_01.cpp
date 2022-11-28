#include <cstdio>
#include <climits>

#include <curses.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../deps.hpp"
#include "../command.hpp"
#include "../html_parser.hpp"
#include "../dom_tree.hpp"
#include "../document_html.hpp"

struct Config
{
    string                  fetchCommand;
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
        "curl --include ${W3M_URL}",
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

    Command         fetch           = Command(cfg.fetchCommand)
                                    .set_env("W3M_URL", cfg.initUrl)
                                    .set_stdout_piped(true);
    string          contentType     = "";
    int             maxCols         = 0;
    int             currLine        = 0;

    WINDOW          *page;
    DocumentHtml    doc(cfg.document);
    char            key;

    // TODO: meaningfully implement

    if (cfg.initUrl.empty())
    {
        curs_set(0);
        waddnstr(stdscr, "ERROR: no url given", COLS);
        wrefresh(stdscr);

        sleep(3);
        curs_set(1);

        return EXIT_FAILURE;
    }

    auto        sproc       = fetch.spawn();

    while (sproc.stdout())
    {
        string      line;
        string      key;
        string      val;
        size_t      colonIdx;

        getline(sproc.stdout(), line);

        if (line.empty() or (line.size() == 1 and line[0] == '\r'))
        {
            break;
        }

        colonIdx = line.find(':');

        if (string::npos == colonIdx)
        {
            continue;
        }

        key = line.substr(0, line.find(':'));

        for (auto& ch : key)
        {
            key = tolower(ch);
        }// end for

        for (auto iter = line.begin() + colonIdx; iter != line.end(); ++iter)
        {
            if (' ' != *iter and '\t' != *iter)
            {
                for (auto iterB = iter; iter != line.end(); ++iter)
                {
                    if (';' == *iterB)
                    {
                        val = string(iter, iterB);
                        break;
                    }
                }// end for
                if (val.empty())
                {
                    val = string(iter, line.end());
                }
                break;
            }
        }// end for

        if ("content-type" == key)
        {
            contentType = val;
        }
    }// end while

    doc.from_stream(sproc.stdout(), COLS);

    sproc.stdout().close();
    sproc.wait();

    // create and draw page
    page = newpad(doc.buffer().size(), COLS);

    for (int i = 0; i < doc.buffer().size(); ++i)
    {
        const auto&     line        = doc.buffer().at(i);
        int             j           = 0;
        int             remCols     = COLS;

        for (const auto& node : line)
        {
            mvwaddnstr(page, i, j, node.text().c_str(), remCols);
            j += node.text().size();
            remCols -= node.text().size();
        }// end for node
    }// end for i

    prefresh(page, currLine, 0, 0, 0, LINES - 1, COLS - 1);
    wnoutrefresh(stdscr);

    // wait for keypress
    while (true)
    {
        switch ((key = wgetch(stdscr)))
        {
            case 'j':
                if (currLine < doc.buffer().size() - LINES)
                {
                    ++currLine;
                }
                break;
            case 'k':
                if (currLine)
                {
                    --currLine;
                }
                break;
            case 'b':
                currLine -= LINES;
                if (currLine < 0)
                {
                    currLine = 0;
                }
                break;
            case ' ':
                currLine += LINES;
                if (currLine >= doc.buffer().size() - LINES)
                {
                    currLine = doc.buffer().size() - LINES;
                }
                break;
            case 'g':
                currLine = 0;
                break;
            case 'G':
                currLine = max(doc.buffer().size() - LINES, 0LU);
                break;
            case 'q':
            case 'Q':
                // exit
                return EXIT_SUCCESS;
        }// end switch
        prefresh(
            page,
            currLine, 0,
            0, 0,
            LINES - 1, COLS - 1
        );
    }// end while

    return EXIT_SUCCESS;
}// end runtime
