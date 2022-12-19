#ifndef __VIEWER_HPP__
#define __VIEWER_HPP__

#include <curses.h>

#include "deps.hpp"
#include "document.hpp"

// color pair definitions
#define     COLOR_DEFAULT               -1

#define     COLOR_PAIR_STANDARD         0x01
#define     COLOR_PAIR_INPUT            0x02
#define     COLOR_PAIR_IMAGE            0x03
#define     COLOR_PAIR_LINK             0x04
#define     COLOR_PAIR_LINK_CURRENT     0x05
#define     COLOR_PAIR_LINK_VISITED     0x06

// === class Viewer =======================================================
//
// ========================================================================
class   Viewer
{
    public:
        // --- public member types ----------------------------------------
        struct  Attrib
        {
            short   fg;
            short   bg;
            int     attr;
        };// end struct Attrib

        struct  Config
        {
            struct
            {
                Attrib      standard;
                Attrib      input;
                Attrib      image;
                Attrib      link;
                Attrib      linkCurrent;
                Attrib      linkVisited;
            }                       attribs;
        };// end struct Config

        // --- public constructors ----------------------------------------
        Viewer(const Config& cfg = {}, Document *doc = nullptr);
        Viewer(const Viewer& other);
        ~Viewer(void);

        // --- public mutators --------------------------------------------
        auto    operator=(const Viewer& other)
            -> Viewer&;
        void    destruct(void);
        void    copy_from(const Viewer& other);
        void    refresh(bool retouch = false);
        void    redraw(void);
        void    goto_point(size_t line, size_t col);
        void    line_down(size_t nLines = 1);
        void    line_up(size_t nLines = 1);
        void    curs_down(size_t nLines = 1);
        void    curs_up(size_t nLines = 1);
        void    curs_left(size_t nCols = 1);
        void    curs_right(size_t nCols = 1);
        auto    curr_url(void)
            -> const string&;
        void    disp_status(const string& str);
        void    clear_status(void);
        auto    prompt_char(const string& str)
            -> char;
        auto    prompt_string(const string& prompt)
            -> string;
    private:
        // === private member variables ===================================
        Config                                  m_cfg;
        WINDOW                                  *m_pad              = nullptr;
        WINDOW                                  *m_statusWin        = nullptr;
        Document                                *m_doc              = nullptr;
        Document::buffer_type::const_iterator   m_bufLineIter;
        Document::BufferLine::const_iterator    m_bufNodeIter;
        size_t                                  m_currLine          = 0;
        size_t                                  m_currCursLine      = 0;
        size_t                                  m_currCol           = 0;
        bool                                    m_isSinglePage      = false;
};// end class Viewer

#endif