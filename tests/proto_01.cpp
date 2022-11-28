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

#define     COLOR_DEFAULT               0x00

#define     COLOR_PAIR_STANDARD         0x01
#define     COLOR_PAIR_INPUT            0x02
#define     COLOR_PAIR_IMAGE            0x03
#define     COLOR_PAIR_LINK             0x04
#define     COLOR_PAIR_LINK_CURRENT     0x05
#define     COLOR_PAIR_LINK_VISITED     0x06

struct  Attrib
{
    short   fg;
    short   bg;
    int     attr;
};// end struct Attrib

struct  Config
{
    string                  fetchCommand;
    string                  initUrl;
    struct
    {
        Attrib      standard;
        Attrib      input;
        Attrib      image;
        Attrib      link;
        Attrib      linkCurrent;
        Attrib      linkVisited;
    }                       attribs;
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
        // fetchCommand
        "curl --include ${W3M_URL}",
        // initUrl
        "",
        // attribs
        {
            { COLOR_WHITE, COLOR_DEFAULT, A_NORMAL },// standard
            { COLOR_RED, COLOR_DEFAULT, A_UNDERLINE },// input
            { COLOR_GREEN, COLOR_DEFAULT, A_BOLD },// image
            { COLOR_BLUE, COLOR_DEFAULT, A_NORMAL },// link
            { COLOR_CYAN, COLOR_DEFAULT, A_NORMAL },// linkCurrent
            { COLOR_MAGENTA, COLOR_DEFAULT, A_NORMAL },// linkVisited
        },
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

    // init colors
    init_pair(
        COLOR_PAIR_STANDARD,
        cfg.attribs.standard.fg,
        cfg.attribs.standard.bg
    );
    init_pair(
        COLOR_PAIR_INPUT,
        cfg.attribs.input.fg,
        cfg.attribs.input.bg
    );
    init_pair(
        COLOR_PAIR_IMAGE,
        cfg.attribs.image.fg,
        cfg.attribs.image.bg
    );
    init_pair(
        COLOR_PAIR_LINK,
        cfg.attribs.link.fg,
        cfg.attribs.link.bg
    );
    init_pair(
        COLOR_PAIR_LINK_CURRENT,
        cfg.attribs.linkCurrent.fg,
        cfg.attribs.linkCurrent.bg
    );
    init_pair(
        COLOR_PAIR_LINK_VISITED,
        cfg.attribs.linkVisited.fg,
        cfg.attribs.linkVisited.bg
    );

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
            // choose attribs
            if (node.input_ref())
            {
                wattrset(page, cfg.attribs.input.attr);
                wcolor_set(page, COLOR_PAIR_INPUT, NULL);
            }
            else if (node.image_ref())
            {
                wattrset(page, cfg.attribs.image.attr);
                wcolor_set(page, COLOR_PAIR_IMAGE, NULL);
            }
            else if (node.link_ref())
            {
                // TODO: differentiate types of links
                wattrset(page, cfg.attribs.link.attr);
                wcolor_set(page, COLOR_PAIR_LINK, NULL);
            }
            else
            {
                // standard
                wattrset(page, A_NORMAL);
                wcolor_set(page, COLOR_PAIR_STANDARD, NULL);
            }
            mvwaddnstr(page, i, j, node.text().c_str(), remCols);
            j += node.text().size();
            remCols -= node.text().size();
        }// end for node
    }// end for i

    wcolor_set(page, 0, NULL);
    wattrset(page, A_NORMAL);
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
