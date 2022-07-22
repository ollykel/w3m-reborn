#include <cstdio>
#include <cctype>
#include <sstream>
#include <map>

#include "deps.hpp"
#include "utils.hpp"
#include "dom_tree.hpp"
#include "html_parser_basic.hpp"
#include "document.hpp"
#include "document_html.hpp"

// === public constructor(s) ======================================
DocumentHtml::DocumentHtml(void) : Document()
{
    // initialize dispatcher
    m_dispatcher["a"] = &DocumentHtml::append_a;
    m_dispatcher["br"] = &DocumentHtml::append_br;
    m_dispatcher["div"] = &DocumentHtml::append_div;
    m_dispatcher["form"] = &DocumentHtml::append_form;
    m_dispatcher["h1"] = &DocumentHtml::append_hn;
    m_dispatcher["h2"] = &DocumentHtml::append_hn;
    m_dispatcher["h3"] = &DocumentHtml::append_hn;
    m_dispatcher["h4"] = &DocumentHtml::append_hn;
    m_dispatcher["h5"] = &DocumentHtml::append_hn;
    m_dispatcher["h6"] = &DocumentHtml::append_hn;
    m_dispatcher["hr"] = &DocumentHtml::append_hr;
    m_dispatcher["img"] = &DocumentHtml::append_img;
    m_dispatcher["ul"] = &DocumentHtml::append_ul;
    m_dispatcher["ol"] = &DocumentHtml::append_ol;
    m_dispatcher["p"] = &DocumentHtml::append_p;
    m_dispatcher["table"] = &DocumentHtml::append_table;
    m_dispatcher["tbody"] = &DocumentHtml::append_tbody;
}// end DocumentHtml(void)

DocumentHtml::DocumentHtml(std::istream& ins, const size_t cols)
    : DocumentHtml()
{
    from_stream(ins, cols);
}// end DocumentHtml(std::istream& ins, const size_t cols)

DocumentHtml::DocumentHtml(const string& text, const size_t cols)
    : DocumentHtml()
{
    from_string(text, cols);
}// end DocumentHtml(const string& text, const size_t cols)

// === public mutator(s) ==========================================
void        DocumentHtml::from_stream(std::istream& ins, const size_t cols)
{
    std::getline(ins, m_data, static_cast<char>(EOF));
    from_string(m_data, cols);
}// end DocumentHtml::from_stream(std::istream& ins, const size_t cols)

void        DocumentHtml::from_string(const string& text, const size_t cols)
{
    using namespace std;

    HtmlParserBasic     parser;
    istringstream       inBuf(text);

    m_dom.reset_root("window");
    m_data = text;
    parser.parse_html(*m_dom.root(), inBuf);

    parse_title_from_data();
    redraw(cols);
}// end DocumentHtml::from_string(const string& text, const size_t cols)

void        DocumentHtml::parse_title_from_data(void)
{
    for (auto& nd : *m_dom.root())
    {
        if (nd.identifier() == "document" or nd.identifier() == "html")
        {
            for (auto& child : nd)
            {
                if (child.identifier() == "head")
                {
                    if (title().empty())
                    {
                        for (auto& item : child)
                        {
                            if (item.identifier() == "title")
                            {
                                for (auto& nd : item)
                                {
                                    if (nd.is_text())
                                    {
                                        set_title(nd.text());
                                        break;
                                    }
                                }// end for nd
                            }
                        }// end for item
                    }
                }
            }// end for child
        }
    }// end for (auto& nd : *m_dom->root())
}// end DocumentHtml::parse_title_from_data(void)

// ------ override(s) ---------------------------------------------
void        DocumentHtml::redraw(size_t cols)
{
    std::vector<string>     styleStack;

    m_buffer.clear();
    m_links.clear();
    m_images.clear();

    for (auto& nd : *m_dom.root())
    {
        if (nd.identifier() == "document" or nd.identifier() == "html")
        {
            for (auto& child : nd)
            {
                // skip <head>
                if (child.identifier() != "head")
                {
                    append_node(child, cols, {}, styleStack);
                }
            }// end for child
        }
        else
        {
            append_node(nd, cols, {}, styleStack);
        }
    }// end for (auto& nd : *m_dom->root())

    // remove extra spaces from ends of lines
    for (auto& line : m_buffer)
    {
        if (not line.empty())
        {
            string&     text    = line.back().m_text;

            if (text.empty())
            {
                line.pop_back();// NOTE: text now points to deallocated memory
            }
            else if (std::isspace(text.back()))
            {
                text.pop_back();
            }
        }
    }// end for (auto& line : m_buffer)
}// end DocumentHtml::redraw(size_t cols)

// === protected mutator(s) ===============================================

// === DocumentHtml::append_node ==========================================
//
// ========================================================================
void    DocumentHtml::append_node(
    const DomTree::node& nd,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    if (nd.is_text())
    {
        append_text(nd, cols, fmt, styleStack);
    }
    // ignore scripts
    else if (nd.identifier() == "script")
    {
        // do nothing
    }
    // ignore styles
    else if (nd.identifier() == "style")
    {
        // do nothing
    }
    else if (m_dispatcher.count(nd.identifier()))
    {
        auto func = m_dispatcher.at(nd.identifier());

        (this->*func)(nd, cols, fmt, styleStack);
    }
    else
    {
        append_other(nd, cols, fmt, styleStack);
    }
}// end DocumentHtml::append_node(const DomTree::node& nd, const size_t cols, Format fmt, std::vector<string>& styleStack)

void    DocumentHtml::append_children(const DomTree::node& nd, const size_t cols, Format fmt, std::vector<string>& styleStack)
{
    for (auto iter = nd.cbegin(); iter != nd.cend(); ++iter)
    {
        const auto& child = *iter;

        append_node(child, cols, fmt, styleStack);
    }// end for (const auto& child : nd)
}// end DocumentHtml::append_children(const DomTree::node& nd, const size_t cols, Format fmt, std::vector<string>& styleStack)

// === DocumentHtml::append_str(const string& str, const size_t cols, Format fmt, std::vector<string>& styleStack) =====
//
// ========================================================================
void    DocumentHtml::append_str(
    const string& str,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    using namespace std;

    size_t          currLen     = m_buffer.empty() ?
                                    0 :
                                    line_length(m_buffer.back());
    string          currLine    = "";
    istringstream   inBuf(str);

    if (m_buffer.empty())
    {
        m_buffer.emplace_back();
    }

    if (m_buffer.back().empty())
    {
        m_buffer.back().emplace_back(string(fmt.indent, ' '), true);
        currLen = fmt.indent;
    }

    if (currLen > fmt.indent and inBuf and isspace(inBuf.peek()))
    {
        currLine += ' ';
        ++currLen;
    }

    while (inBuf)
    {
        string      token       = "";

        inBuf >> token;
        token = decode_text(token);

        while (not token.empty())
        {
            size_t      colsLeft        = cols > currLen ?
                                            cols - currLen :
                                            0;

            if (token.length() <= colsLeft)
            {
                currLine += token;
                currLen += token.length();
                if (inBuf and inBuf.peek() != EOF
                    and colsLeft != token.length())
                {
                    currLine += ' ';
                    ++currLen;
                    --colsLeft;
                }
                token.clear();
            }
            else
            {
                if (currLen <= fmt.indent)
                {
                    currLine = token.substr(0, colsLeft);
                    token.erase(0, colsLeft);
                }
                m_buffer.back().emplace_back(currLine);
                for (const auto& styler : styleStack)
                {
                    m_buffer.back().back().append_styler(styler);
                }// end for styler
                m_buffer.emplace_back();
                if (fmt.indent)
                {
                    currLen = fmt.indent;
                    m_buffer.back().emplace_back(string(fmt.indent, ' '), true);
                }
                else
                {
                    currLen = 0;
                }
                currLine.clear();
            }
        }// end while (not token.empty())
    }// end while (inBuf)

    m_buffer.back().emplace_back(currLine);
    for (const auto& styler : styleStack)
    {
        m_buffer.back().back().append_styler(styler);
    }// end for styler
}// end DocumentHtml::append_str(const string& str, const size_t cols, Format fmt, std::vector<string>& styleStack)

// === DocumentHtml::append_text(const DomTree::node& text) ===============
//
// Draws a text node at the end of the buffer, broken over several lines if
// necessary. Starts at the end of the last line, if space is available.
//
// ========================================================================
void    DocumentHtml::append_text(const DomTree::node& text, const size_t cols, Format fmt, std::vector<string>& styleStack)
{
    append_str(text.text(), cols, fmt, styleStack);
}// end DocumentHtml::append_text(const DomTree::node& text)

// === DocumentHtml::append_a =============================================
//
// Append an anchor. If has an href, add it to list of links and add
// reference to newly added buffer nodes.
//
// ========================================================================
void    DocumentHtml::append_a(
    const DomTree::node& a,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    if (m_buffer.empty())
    {
        m_buffer.emplace_back();
    }

    // store last line, column indices before appending children
    const size_t    startLineIdx = m_buffer.size() - 1;
    const size_t    startNodeIdx = m_buffer.back().size();

    append_children(a, cols, fmt, styleStack);

    if (not a.attributes.count("href"))
    {
        return;
    }

    const size_t        linkIdx     = m_links.size();

    m_links.emplace_back(a.attributes.at("href"));
    auto&               currLink    = m_links.back();

    for (size_t i = startLineIdx, j = startNodeIdx; i < m_buffer.size(); ++i)
    {
        auto&   currLine    = m_buffer.at(i);

        for (; j < currLine.size(); ++j)
        {
            auto&   currNode    = currLine.at(j);

            if (not currNode.reserved() and not currNode.link_ref())
            {
                currNode.set_link_ref(linkIdx);
                currLink.append_referer(i, j);
            }
        }// end for j

        j = 0;
    }// end for i
}// end DocumentHtml::append_a(const DomTree::node& a)

// === DocumentHtml::append_br(const DomTree::node& br, const size_t cols, Format fmt, std::vector<string>& styleStack)
//
// ========================================================================
void    DocumentHtml::append_br(const DomTree::node& br, const size_t cols, Format fmt, std::vector<string>& styleStack)
{
    m_buffer.emplace_back();
}// end DocumentHtml::append_br(const DomTree::node& br, const size_t cols, Format fmt, std::vector<string>& styleStack)

void    DocumentHtml::append_div(
    const DomTree::node& div,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    begin_block(cols, fmt);
    append_children(div, cols, fmt, styleStack);
}// end DocumentHtml::append_div(const DomTree::node& br, const size_t cols, Format fmt, std::vector<string>& styleStack)

// === DocumentHtml::append_form(const DomTree::node& form, const size_t cols, Format fmt, std::vector<string>& styleStack)
//
// TODO: actually implement
//
// ========================================================================
void    DocumentHtml::append_form(const DomTree::node& form, const size_t cols, Format fmt, std::vector<string>& styleStack)
{
    m_buffer.emplace_back();
    m_buffer.back().emplace_back("<FORM>");
    m_buffer.emplace_back();
}// end DocumentHtml::append_form(const DomTree::node& form, const size_t cols, Format fmt, std::vector<string>& styleStack)

// === DocumentHtml::append_hn(const DomTree::node& hn) ===================
//
// Append a header (i.e. h1, h2, h3, etc.)
//
// TODO: actually implement; need to update BufferNode struct to account
// for styling (i.e. "style" enum)
//
// ========================================================================
void    DocumentHtml::append_hn(
    const DomTree::node& hn,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    styleStack.emplace_back(hn.identifier());

    begin_block(cols, fmt);
    append_children(hn, cols, fmt, styleStack);
    m_buffer.emplace_back();

    styleStack.pop_back();
}// end DocumentHtml::append_hn(const DomTree::node& hn)

// === DocumentHtml::append_hr(const DomTree::node& hr, const size_t cols, Format fmt, std::vector<string>& styleStack)
//
// ========================================================================
void    DocumentHtml::append_hr(
    const DomTree::node& hr,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    static size_t       nCols       = 0;
    static string       rule        = "";

    if (cols != nCols)
    {
        rule = string(cols, '-');
        nCols = cols;
    }

    m_buffer.emplace_back();
    m_buffer.back().emplace_back(rule);
    m_buffer.emplace_back();
}// end DocumentHtml::append_hr(const DomTree::node& hr, const size_t cols, Format fmt, std::vector<string>& styleStack)

// === DocumentHtml::append_img(const DomTree::node& img) =================
//
// ========================================================================
void    DocumentHtml::append_img(
    const DomTree::node& img,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    using namespace std;

    if (m_buffer.empty())
    {
        m_buffer.emplace_back();
    }

    if (not img.attributes.count("src"))
    {
        return;
    }

    // store last line, column indices before appending children
    const size_t    startLineIdx = m_buffer.size() - 1;
    const size_t    startNodeIdx = m_buffer.back().size();

    // set representative string; ideally alt
    // if alt not provided, use truncated url from src
    string      imgText     = "[";

    imgText += img.attributes.count("alt") ?
                img.attributes.at("alt") :
                utils::path_base(img.attributes.at("src"));

    if (imgText.length() == 1)
    {
        imgText += utils::path_base(img.attributes.at("src"));
    }

    imgText += ']';
    append_str(imgText, cols, fmt, styleStack);

    const size_t        linkIdx     = m_images.size();

    m_images.emplace_back(img.attributes.at("src"));

    auto&               currImg     = m_images.back();

    for (size_t i = startLineIdx, j = startNodeIdx; i < m_buffer.size(); ++i)
    {
        auto&   currLine    = m_buffer.at(i);

        for (; j < currLine.size(); ++j)
        {
            auto&   currNode    = currLine.at(j);

            if (not currNode.reserved())
            {
                currNode.set_image_ref(linkIdx);
                currImg.append_referer(i, j);
            }
        }// end for j

        j = 0;
    }// end for i
}// end DocumentHtml::append_img(const DomTree::node& img)

// === DocumentHtml::append_ul(const DomTree::node& img, const size_t cols, Format fmt, std::vector<string>& styleStack)
//
// ========================================================================
void    DocumentHtml::append_ul(
    const DomTree::node& ul,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    begin_block(cols, fmt);
    fmt.set_in_list(true);
    fmt.set_ordered_list(false);
    if (fmt.listLevel)
        fmt.indent += 2;

    for (auto iter = ul.cbegin(); iter != ul.cend(); ++iter)
    {
        const auto& child = *iter;

        m_buffer.emplace_back();
        if (child.identifier() == "li")
        {
            append_li_ul(child, cols, fmt, styleStack);
        }
        else
        {
            append_node(child, cols, fmt, styleStack);
        }
    }// end for child

    m_buffer.emplace_back();
}// end DocumentHtml::append_ul(const DomTree::node& ul, const size_t cols, Format fmt, std::vector<string>& styleStack)

// === DocumentHtml::append_ol(const DomTree::node& ol, const size_t cols, Format fmt, std::vector<string>& styleStack)
//
// ========================================================================
void    DocumentHtml::append_ol(
    const DomTree::node& ol,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    begin_block(cols, fmt);
    fmt.set_in_list(true);
    fmt.set_ordered_list(true);
    fmt.listIndex = 1;

    for (auto iter = ol.cbegin(); iter != ol.cend(); ++iter)
    {
        const auto& child = *iter;

        m_buffer.emplace_back();
        if (child.identifier() == "li")
        {
            append_li_ol(child, cols, fmt, styleStack);
            ++fmt.listIndex;
        }
        else
        {
            append_node(child, cols, fmt, styleStack);
        }
    }// end for child
    m_buffer.emplace_back();
}// end DocumentHtml::append_ol(const DomTree::node& ol, const size_t cols, Format fmt, std::vector<string>& styleStack)

// === DocumentHtml::append_li_ul =========================================
//
// ========================================================================
void    DocumentHtml::append_li_ul(
    const DomTree::node& li,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    if (fmt.in_list())
    {
        fmt.set_block_ignore(true);
        m_buffer.back().emplace_back(string(fmt.indent, ' '), true);
        if (fmt.listLevel & 1)// odd list levels
            m_buffer.back().emplace_back("+ ");
        else
            m_buffer.back().emplace_back("* ");
        ++fmt.listLevel;
        if (fmt.indent < cols / 2)
            fmt.indent += 2;
    }
    fmt.set_in_list(false);
    fmt.set_ordered_list(false);
    append_children(li, cols, fmt, styleStack);
}// end DocumentHtml::append_li_ul

// === DocumentHtml::append_li_ol =========================================
//
// ========================================================================
void    DocumentHtml::append_li_ol(
    const DomTree::node& li,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    if (fmt.in_list())
    {
        std::stringstream       format;

        fmt.set_block_ignore(true);
        m_buffer.back().emplace_back(string(fmt.indent, ' '), true);
        format << fmt.listIndex << ". ";
        m_buffer.back().emplace_back(format.str());
        if (fmt.indent < cols / 2)
            fmt.indent += format.str().length();
    }
    append_children(li, cols, fmt, styleStack);
}// end DocumentHtml::append_li_ol

// === DocumentHtml::append_p(const DomTree::node& p) =====================
//
// TODO: implement, with styling (see append_hn)
//
// ========================================================================
void    DocumentHtml::append_p(
    const DomTree::node& p,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    begin_block(cols, fmt);
    fmt.set_block_ignore(false);
    append_children(p, cols, fmt, styleStack);
    // append an extra newline to denote new paragraph
    m_buffer.emplace_back();
    m_buffer.emplace_back();
}// end DocumentHtml::append_p(const DomTree::node& p)

// === DocumentHtml::append_table(const DomTree::node& table, const size_t cols, Format fmt, std::vector<string>& styleStack)
//
// TODO: actually implement
//
// ========================================================================
void    DocumentHtml::append_table(
    const DomTree::node& table,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    begin_block(cols, fmt);
    append_hr(table, cols, fmt, styleStack);
    append_tbody(table, cols, fmt, styleStack);// behavior is mostly the same
    m_buffer.emplace_back();
}// end DocumentHtml::append_table(const DomTree::node& table, const size_t cols, Format fmt, std::vector<string>& styleStack)

// === DocumentHtml::append_tbody(const DomTree::node& tbody, const size_t cols, Format fmt, std::vector<string>& styleStack)
//
// TODO: actually implement
//
// ========================================================================
void    DocumentHtml::append_tbody(
    const DomTree::node& tbody,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    for (auto iter = tbody.cbegin(); iter != tbody.cend(); ++iter)
    {
        const auto&     elem        = *iter;

        append_node(elem, cols, fmt, styleStack);
        if (elem.identifier() == "tr")
        {
            append_hr(tbody, cols, fmt, styleStack);
        }
    }// end for iter
}// end DocumentHtml::append_tbody(const DomTree::node& tbody, const size_t cols, Format fmt, std::vector<string>& styleStack)

// === DocumentHtml::append_other(const DomTree::node& nd) ================
//
// TODO: actually implement in a meaningful way; current implementation
// just appends all children of the node.
//
// ========================================================================
void    DocumentHtml::append_other(
    const DomTree::node& nd,
    const size_t cols,
    Format fmt,
    std::vector<string>& styleStack
)
{
    append_children(nd, cols, fmt, styleStack);
}// end DocumentHtml::append_other(const DomTree::node& nd)

void    DocumentHtml::begin_block(const size_t cols, Format& fmt)
{
    fmt.set_block_ignore(false);

    if (m_buffer.empty())
    {
        m_buffer.emplace_back();
        return;
    }

    const size_t    currLen     = line_length(m_buffer.back());

    if (currLen > fmt.indent)
    {
        m_buffer.emplace_back();
    }
}// end DocumentHtml::begin_block(const size_t cols, Format fmt)

// === protected static function(s) =======================================
size_t  DocumentHtml::line_length(BufferLine& line)
{
    size_t      count       = 0;

    for (auto& node : line)
    {
        count += node.text().length();
    }// end for (auto& node : line)

    return count;
}// end DocumentHtml::line_length(std::vector<BufferNode>& line)

bool    DocumentHtml::is_node_header(const DomTree::node& nd)
{
    const string&   id      = nd.identifier();

    if (id.length() < 2 or id[0] != 'h')
    {
        return false;
    }
    for (size_t i = 1; i < id.length(); ++i)
    {
        if (not isdigit(id[i]))
        {
            return false;
        }
    }// end for (size_t i = 1; i < id.length(); ++i)
    return true;
}// end DocumentHtml::is_node_header(const DomTree::node& nd)

unsigned    DocumentHtml::parse_html_entity(const string& id)
{
    // TODO: find way to generate from text/csv file
    const static std::map<string, unsigned>   entMap  = {
        {"dollar", 36},
        {"cent",   162},
        {"pound",  163},
        {"curren", 164},
        {"yen",    165},
        {"copy",              169},
        {"reg",               174},
        {"trade",             8482},
        {"commat",            64},
        {"Copf",              8450},
        {"incare",            8453},
        {"gscr",              8458},
        {"hamilt",            8459},
        {"Hfr",               8460},
        {"Hopf",              8461},
        {"planckh",           8462},
        {"planck",            8463},
        {"Iscr",              8464},
        {"image",             8465},
        {"Lscr",              8466},
        {"ell",               8467},
        {"Nopf",              8469},
        {"numero",            8470},
        {"copysr",            8471},
        {"weierp",            8472},
        {"Popf",              8473},
        {"Qopf",              8474},
        {"Rscr",              8475},
        {"real",              8476},
        {"Ropf",              8477},
        {"rx",                8478},
        {"Zopf",              8484},
        {"mho",               8487},
        {"Zfr",               8488},
        {"iiota",             8489},
        {"bernou",            8492},
        {"Cfr",               8493},
        {"escr",              8495},
        {"Escr",              8496},
        {"Fscr",              8497},
        {"Mscr",              8499},
        {"oscr",              8500},
        {"alefsym",           8501},
        {"beth",              8502},
        {"gimel",             8503},
        {"daleth",            8504},
        {"DD",                8517},
        {"dd",                8518},
        {"ee",                8519},
        {"ii",                8520},
        {"starf",             9733},
        {"star",              9734},
        {"phone",             9742},
        {"female",            9792},
        {"male",              9794},
        {"spades",            9824},
        {"clubs",             9827},
        {"hearts",            9829},
        {"diams",             9830},
        {"loz",               9674},
        {"sung",              9834},
        {"flat",              9837},
        {"natural",           9838},
        {"sharp",             9839},
        {"check",             10003},
        {"cross",             10007},
        {"malt",              10016},
        {"sext",              10038},
        {"VerticalSeparator", 10072},
        {"lbbrk",             10098},
        {"rbbrk",             10099},
        {"excl",    33},
        {"num",     35},
        {"percnt",  37},
        {"amp",     38},
        {"lpar",    40},
        {"rpar",    41},
        {"ast",     42},
        {"comma",   44},
        {"period",  46},
        {"sol",     47},
        {"colon",   58},
        {"semi",    59},
        {"quest",   63},
        {"lbrack",  91},
        {"bsol",    92},
        {"rbrack",  93},
        {"Hat",     94},
        {"lowbar",  95},
        {"grave",   96},
        {"lbrace",  123},
        {"vert",    124},
        {"rbrace",  125},
        {"tilde",   126},
        {"circ",    710},
        {"nbsp",    160},
        {"ensp",    8194},
        {"emsp",    8195},
        {"thinsp",  8201},
        {"zwnj",    8204},
        {"zwj",     8205},
        {"lrm",     8206},
        {"rlm",     8207},
        {"iexcl",   161},
        {"brvbar",  166},
        {"sect",    167},
        {"uml",     168},
        {"ordf",    170},
        {"not",     172},
        {"shy",     173},
        {"macr",    175},
        {"sup2",    178},
        {"sup3",    179},
        {"acute",   180},
        {"micro",   181},
        {"para",    182},
        {"middot",  183},
        {"cedil",   184},
        {"sup1",    185},
        {"ordm",    186},
        {"iquest",  191},
        {"hyphen",  8208},
        {"ndash",   8211},
        {"mdash",   8212},
        {"horbar",  8213},
        {"Vert",    8214},
        {"dagger",  8224},
        {"Dagger",  8225},
        {"bull",    8226},
        {"nldr",    8229},
        {"hellip",  8230},
        {"pertenk", 8241},
        {"prime",   8242},
        {"Prime",   8243},
        {"tprime",  8244},
        {"bprime",  8245},
        {"oline",   8254},
        {"caret",   8257},
        {"hybull",  8259},
        {"frasl",   8260},
        {"bsemi",   8271},
        {"qprime",  8279},
        {"quot",   34},
        {"apos",   39},
        {"laquo",  171},
        {"raquo",  187},
        {"lsquo",  8216},
        {"rsquo",  8217},
        {"sbquo",  8218},
        {"ldquo",  8220},
        {"rdquo",  8221},
        {"bdquo",  8222},
        {"lsaquo", 8249},
        {"rsaquo", 8250},
        {"frac14", 188},
        {"frac12", 189},
        {"frac34", 190},
        {"frac13", 8531},
        {"frac23", 8532},
        {"frac15", 8533},
        {"frac25", 8534},
        {"frac35", 8535},
        {"frac45", 8536},
        {"frac16", 8537},
        {"frac56", 8538},
        {"frac18", 8539},
        {"frac38", 8540},
        {"frac58", 8541},
        {"frac78", 8542},
        {"plus",      43},
        {"minus",     8722},
        {"times",     215},
        {"divide",    247},
        {"equals",    61},
        {"ne",        8800},
        {"plusmn",    177},
        {"not",       172},
        {"lt",        60},
        {"gt",        62},
        {"deg",       176},
        {"sup1",      185},
        {"sup2",      178},
        {"sup3",      179},
        {"fnof",      402},
        {"percnt",    37},
        {"permil",    137},
        {"pertenk",   8241},
        {"forall",    8704},
        {"comp",      8705},
        {"part",      8706},
        {"exist",     8707},
        {"nexist",    8708},
        {"empty",     8709},
        {"nabla",     8711},
        {"isin",      8712},
        {"notin",     8713},
        {"ni",        8715},
        {"notni",     8716},
        {"prod",      8719},
        {"coprod",    8720},
        {"sum",       8721},
        {"mnplus",    8723},
        {"plusdo",    8724},
        {"setminus",  8726},
        {"lowast",    8727},
        {"compfn",    8728},
        {"radic",     8730},
        {"prop",      8733},
        {"infin",     8734},
        {"angrt",     8735},
        {"ang",       8736},
        {"angmsd",    8737},
        {"angsph",    8738},
        {"mid",       8739},
        {"nmid",      8740},
        {"parallel",  8741},
        {"npar",      8742},
        {"and",       8743},
        {"or",        8744},
        {"cap",       8745},
        {"cup",       8746},
        {"int",       8747},
        {"Int",       8748},
        {"iiint",     8749},
        {"conint",    8750},
        {"Conint",    8751},
        {"Cconint",   8752},
        {"cwint",     8753},
        {"cwconint",  8754},
        {"awconint",  8755},
        {"there4",    8756},
        {"because",   8757},
        {"ratio",     8758},
        {"Colon",     8759},
        {"minusd",    8760},
        {"mDDot",     8762},
        {"homtht",    8763},
        {"sim",       8764},
        {"bsim",      8765},
        {"ac",        8766},
        {"acd",       8767},
        {"wreath",    8768},
        {"nsim",      8769},
        {"esim",      8770},
        {"sime",      8771},
        {"nsime",     8772},
        {"cong",      8773},
        {"simne",     8774},
        {"ncong",     8775},
        {"asymp",     8776},
        {"nap",       8777},
        {"approxeq",  8778},
        {"apid",      8779},
        {"bcong",     8780},
        {"asympeq",   8781},
        {"bump",      8782},
        {"bumpe",     8783},
        {"esdot",     8784},
        {"eDot",      8785},
        {"efDot",     8786},
        {"erDot",     8787},
        {"colone",    8788},
        {"ecolon",    8789},
        {"ecir",      8790},
        {"cire",      8791},
        {"wedgeq",    8793},
        {"veeeq",     8794},
        {"trie",      8796},
        {"equest",    8799},
        {"equiv",     8801},
        {"nequiv",    8802},
        {"le",        8804},
        {"ge",        8805},
        {"lE",        8806},
        {"gE",        8807},
        {"lnE",       8808},
        {"gnE",       8809},
        {"Lt",        8810},
        {"Gt",        8811},
        {"between",   8812},
        {"NotCupCap", 8813},
        {"nlt",       8814},
        {"ngt",       8815},
        {"nle",       8816},
        {"nge",       8817},
        {"lsim",      8818},
        {"gsim",      8819},
        {"nlsim",     8820},
        {"ngsim",     8821},
        {"lg",        8822},
        {"gl",        8823},
        {"ntlg",      8824},
        {"ntgl",      8825},
        {"pr",        8826},
        {"sc",        8827},
        {"prcue",     8828},
        {"sccue",     8829},
        {"prsim",     8830},
        {"scsim",     8831},
        {"npr",       8832},
        {"nsc",       8833},
        {"sub",       8834},
        {"sup",       8835},
        {"nsub",      8836},
        {"nsup",      8837},
        {"sube",      8838},
        {"supe",      8839},
        {"nsube",     8840},
        {"nsupe",     8841},
        {"subne",     8842},
        {"supne",     8843},
        {"cupdot",    8845},
        {"uplus",     8846},
        {"sqsub",     8847},
        {"sqsup",     8848},
        {"sqsube",    8849},
        {"sqsupe",    8850},
        {"sqcap",     8851},
        {"sqcup",     8852},
        {"oplus",     8853},
        {"ominus",    8854},
        {"otimes",    8855},
        {"osol",      8856},
        {"odot",      8857},
        {"ocir",      8858},
        {"oast",      8859},
        {"odash",     8861},
        {"plusb",     8862},
        {"minusb",    8863},
        {"timesb",    8864},
        {"sdotb",     8865},
        {"vdash",     8866},
        {"dashv",     8867},
        {"top",       8868},
        {"perp",      8869},
        {"models",    8871},
        {"vDash",     8872},
        {"Vdash",     8873},
        {"Vvdash",    8874},
        {"VDash",     8875},
        {"nvdash",    8876},
        {"nvDash",    8877},
        {"nVdash",    8878},
        {"nVDash",    8879},
        {"prurel",    8880},
        {"vltri",     8882},
        {"vrtri",     8883},
        {"ltrie",     8884},
        {"rtrie",     8885},
        {"origof",    8886},
        {"imof",      8887},
        {"mumap",     8888},
        {"hercon",    8889},
        {"intcal",    8890},
        {"veebar",    8891},
        {"barvee",    8893},
        {"angrtvb",   8894},
        {"lrtri",     8895},
        {"xwedge",    8896},
        {"xvee",      8897},
        {"xcap",      8898},
        {"xcup",      8899},
        {"diamond",   8900},
        {"sdot",      8901},
        {"Star",      8902},
        {"divonx",    8903},
        {"bowtie",    8904},
        {"ltimes",    8905},
        {"rtimes",    8906},
        {"lthree",    8907},
        {"rthree",    8908},
        {"bsime",     8909},
        {"cuvee",     8910},
        {"cuwed",     8911},
        {"Sub",       8912},
        {"Sup",       8913},
        {"Cap",       8914},
        {"Cup",       8915},
        {"fork",      8916},
        {"epar",      8917},
        {"ltdot",     8918},
        {"gtdot",     8919},
        {"Ll",        8920},
        {"Gg",        8921},
        {"leg",       8922},
        {"gel",       8923},
        {"cuepr",     8926},
        {"cuesc",     8927},
        {"nprcue",    8928},
        {"nsccue",    8929},
        {"nsqsube",   8930},
        {"nsqsupe",   8931},
        {"lnsim",     8934},
        {"gnsim",     8935},
        {"prnsim",    8936},
        {"scnsim",    8937},
        {"nltri",     8938},
        {"nrtri",     8939},
        {"nltrie",    8940},
        {"nrtrie",    8941},
        {"vellip",    8942},
        {"ctdot",     8943},
        {"utdot",     8944},
        {"dtdot",     8945},
        {"disin",     8946},
        {"isinsv",    8947},
        {"isins",     8948},
        {"isindot",   8949},
        {"notinvc",   8950},
        {"notinvb",   8951},
        {"isinE",     8953},
        {"nisd",      8954},
        {"xnis",      8955},
        {"nis",       8956},
        {"notnivc",   8957},
        {"notnivb",   8958},
        {"lceil",     8968},
        {"rceil",     8969},
        {"lfloor",    8970},
        {"rfloor",    8971},
        {"lang",      9001},
        {"rang",      9002},
        {"Alpha",     913},
        {"Beta",      914},
        {"Gamma",     915},
        {"Delta",     916},
        {"Epsilon",   917},
        {"Zeta",      918},
        {"Eta",       919},
        {"Theta",     920},
        {"Iota",      921},
        {"Kappa",     922},
        {"Lambda",    923},
        {"Mu",        924},
        {"Nu",        925},
        {"Xi",        926},
        {"Omicron",   927},
        {"Pi",        928},
        {"Rho",       929},
        {"Sigma",     931},
        {"Tau",       932},
        {"Upsilon",   933},
        {"Phi",       934},
        {"Chi",       935},
        {"Psi",       936},
        {"Omega",     937},
        {"alpha",     945},
        {"beta",      946},
        {"gamma",     947},
        {"delta",     948},
        {"epsilon",   949},
        {"zeta",      950},
        {"eta",       951},
        {"theta",     952},
        {"iota",      953},
        {"kappa",     954},
        {"lambda",    955},
        {"mu",        956},
        {"nu",        957},
        {"xi",        958},
        {"omicron",   959},
        {"pi",        960},
        {"rho",       961},
        {"sigmaf",    962},
        {"sigma",     963},
        {"tau",       964},
        {"upsilon",   965},
        {"phi",       966},
        {"chi",       967},
        {"psi",       968},
        {"omega",     969},
        {"thetasym",  977},
        {"upsih",     978},
        {"piv",       982},
        {"Agrave",     192},
        {"Aacute",     193},
        {"Acirc",      194},
        {"Atilde",     195},
        {"Auml",       196},
        {"Aring",      197},
        {"AElig",      198},
        {"Ccedil",     199},
        {"Egrave",     200},
        {"Eacute",     201},
        {"Ecirc",      202},
        {"Euml",       203},
        {"Lgrave",     204},
        {"Lacute",     313},
        {"Lcirc",      206},
        {"Luml",       207},
        {"ETH",        208},
        {"Ntilde",     209},
        {"Ograve",     210},
        {"Oacute",     211},
        {"Ocirc",      212},
        {"Otilde",     213},
        {"Ouml",       214},
        {"Oslash",     216},
        {"Ugrave",     217},
        {"Uacute",     218},
        {"Ucirc",      219},
        {"Uuml",       220},
        {"Yacute",     221},
        {"THORN",      222},
        {"szlig",      223},
        {"agrave",     224},
        {"aacute",     225},
        {"acirc",      226},
        {"atilde",     227},
        {"auml",       228},
        {"aring",      229},
        {"aelig",      230},
        {"ccedil",     231},
        {"egrave",     232},
        {"eacute",     233},
        {"ecirc",      234},
        {"euml",       235},
        {"igrave",     236},
        {"iacute",     237},
        {"icirc",      238},
        {"iuml",       239},
        {"eth",        240},
        {"ntilde",     241},
        {"ograve",     242},
        {"oacute",     243},
        {"ocirc",      244},
        {"otilde",     245},
        {"ouml",       246},
        {"oslash",     248},
        {"ugrave",     249},
        {"uacute",     250},
        {"ucirc",      251},
        {"uuml",       252},
        {"yacute",     253},
        {"thorn",      254},
        {"yuml",       255},
        {"Amacr",      256},
        {"amacr",      257},
        {"Abreve",     258},
        {"abreve",     259},
        {"Aogon",      260},
        {"aogon",      261},
        {"Cacute",     262},
        {"cacute",     263},
        {"Ccirc",      264},
        {"ccirc",      265},
        {"Cdot",       266},
        {"cdot",       267},
        {"Ccaron",     268},
        {"ccaron",     269},
        {"Dcaron",     270},
        {"dcaron",     271},
        {"Dstrok",     272},
        {"dstrok",     273},
        {"Emacr",      274},
        {"emacr",      275},
        {"Edot",       278},
        {"edot",       279},
        {"Eogon",      280},
        {"eogon",      281},
        {"Ecaron",     282},
        {"ecaron",     283},
        {"Gcirc",      284},
        {"gcirc",      285},
        {"Gbreve",     286},
        {"gbreve",     287},
        {"Gdot",       288},
        {"gdot",       289},
        {"Gcedil",     290},
        {"Hcirc",      292},
        {"hcirc",      293},
        {"Hstrok",     294},
        {"hstrok",     295},
        {"Itilde",     296},
        {"itilde",     297},
        {"Imacr",      298},
        {"imacr",      299},
        {"Iogon",      302},
        {"iogon",      303},
        {"Idot",       304},
        {"imath",      305},
        {"IJlig",      306},
        {"ijlig",      307},
        {"Jcirc",      308},
        {"jcirc",      309},
        {"Kcedil",     310},
        {"kcedil",     311},
        {"kgreen",     312},
        {"lacute",     314},
        {"Lcedil",     315},
        {"lcedil",     316},
        {"Lcaron",     317},
        {"lcaron",     318},
        {"Lmidot",     319},
        {"lmidot",     320},
        {"Lstrok",     321},
        {"lstrok",     322},
        {"Nacute",     323},
        {"nacute",     324},
        {"Ncedil",     325},
        {"ncedil",     326},
        {"Ncaron",     327},
        {"ncaron",     328},
        {"napos",      329},
        {"ENG",        330},
        {"eng",        331},
        {"Omacr",      332},
        {"omacr",      333},
        {"Odblac",     336},
        {"odblac",     337},
        {"OElig",      338},
        {"oelig",      339},
        {"Racute",     340},
        {"racute",     341},
        {"Rcedil",     342},
        {"rcedil",     343},
        {"Rcaron",     344},
        {"rcaron",     345},
        {"Sacute",     346},
        {"sacute",     347},
        {"Scirc",      348},
        {"scirc",      349},
        {"Scedil",     350},
        {"scedil",     351},
        {"Scaron",     352},
        {"scaron",     353},
        {"Tcedil",     354},
        {"tcedil",     355},
        {"Tcaron",     356},
        {"tcaron",     357},
        {"Tstrok",     358},
        {"tstrok",     359},
        {"Utilde",     360},
        {"utilde",     361},
        {"Umacr",      362},
        {"umacr",      363},
        {"Ubreve",     364},
        {"ubreve",     365},
        {"Uring",      366},
        {"uring",      367},
        {"Udblac",     368},
        {"udblac",     369},
        {"Uogon",      370},
        {"uogon",      371},
        {"Wcirc",      372},
        {"wcirc",      373},
        {"Ycirc",      374},
        {"ycirc",      375},
        {"Yuml",       376},
        {"Zacute",     377},
        {"zacute",     378},
        {"Zdot",       379},
        {"zdot",       380},
        {"Zcaron",     381},
        {"zcaron",     382},
        {"DownBreve",  785},
        {"olarr",               8634},
        {"orarr",               8635},
        {"lharu",               8636},
        {"lhard",               8637},
        {"uharr",               8638},
        {"uharl",               8639},
        {"rharu",               8640},
        {"rhard",               8641},
        {"dharr",               8642},
        {"dharl",               8643},
        {"rlarr",               8644},
        {"udarr",               8645},
        {"lrarr",               8646},
        {"llarr",               8647},
        {"uuarr",               8648},
        {"rrarr",               8649},
        {"ddarr",               8650},
        {"lrhar",               8651},
        {"rlhar",               8652},
        {"nlArr",               8653},
        {"nhArr",               8654},
        {"nrArr",               8655},
        {"lArr",                8656},
        {"uArr",                8657},
        {"rArr",                8658},
        {"dArr",                8659},
        {"hArr",                8660},
        {"vArr",                8661},
        {"nwArr",               8662},
        {"neArr",               8663},
        {"seArr",               8664},
        {"swArr",               8665},
        {"lAarr",               8666},
        {"rAarr",               8667},
        {"ziglarr",             8668},
        {"zigrarr",             8669},
        {"larrb",               8676},
        {"rarrb",               8677},
        {"duarr",               8693},
        {"hoarr",               8703},
        {"loarr",               8701},
        {"roarr",               8702},
        {"xlarr",               10229},
        {"xrarr",               10230},
        {"xharr",               10231},
        {"xlArr",               10232},
        {"xrArr",               10233},
        {"xhArr",               10234},
        {"dzigrarr",            10239},
        {"xmap",                10236},
        {"nvlArr",              10498},
        {"nvrArr",              10499},
        {"nvHarr",              10500},
        {"Map",                 10501},
        {"lbarr",               10508},
        {"rbarr",               10509},
        {"lBarr",               10510},
        {"rBarr",               10511},
        {"RBarr",               10512},
        {"DDotrahd",            10513},
        {"UpArrowBar",          10514},
        {"DownArrowBar",        10515},
        {"Rarrtl",              10518},
        {"latail",              10521},
        {"ratail",              10522},
        {"lAtail",              10523},
        {"rAtail",              10524},
        {"larrfs",              10525},
        {"rarrfs",              10526},
        {"larrbfs",             10527},
        {"rarrbfs",             10528},
        {"nwarhk",              10531},
        {"nearhk",              10532},
        {"searhk",              10533},
        {"swarhk",              10534},
        {"nwnear",              10535},
        {"nesear",              10536},
        {"seswar",              10537},
        {"swnwar",              10538},
        {"cudarrr",             10549},
        {"ldca",                10550},
        {"rdca",                10551},
        {"cudarrl",             10552},
        {"larrpl",              10553},
        {"curarrm",             10556},
        {"cularrp",             10557},
        {"rarrpl",              10565},
        {"harrcir",             10568},
        {"Uarrocir",            10569},
        {"lurdshar",            10570},
        {"ldrushar",            10571},
        {"RightUpDownVector",   10575},
        {"DownLeftRightVector", 10576},
        {"LeftUpDownVector",    10577},
        {"LeftVectorBar",       10578},
        {"RightVectorBar",      10579},
        {"RightUpVectorBar",    10580},
        {"RightDownVectorBar",  10581},
        {"DownLeftVectorBar",   10582},
        {"DownRightVectorBar",  10583},
        {"LeftUpVectorBar",     10584},
        {"LeftDownVectorBar",   10585},
        {"LeftTeeVector",       10586},
        {"RightTeeVector",      10587},
        {"RightUpTeeVector",    10588},
        {"RightDownTeeVector",  10589},
        {"DownLeftTeeVector",   10590},
        {"DownRightTeeVector",  10591},
        {"LeftUpTeeVector",     10592},
        {"LeftDownTeeVector",   10593},
        {"lHar",                10594},
        {"uHar",                10595},
        {"rHar",                10596},
        {"dHar",                10597},
        {"luruhar",             10598},
        {"ldrdhar",             10599},
        {"ruluhar",             10600},
        {"rdldhar",             10601},
        {"lharul",              10602},
        {"llhard",              10603},
        {"rharul",              10604},
        {"lrhard",              10605},
        {"udhar",               10606},
        {"duhar",               10607},
        {"RoundImplies",        10608},
        {"erarr",               10609},
        {"simrarr",             10610},
        {"larrsim",             10611},
        {"rarrsim",             10612},
        {"rarrap",              10613},
        {"ltlarr",              10614},
        {"gtrarr",              10616},
        {"subrarr",             10617},
        {"suplarr",             10619},
        {"lfisht",              10620},
        {"rfisht",              10621},
        {"ufisht",              10622},
        {"dfisht",              10623},
    };

    unsigned    out     = '&';

    if (entMap.count(id))
    {
        out = entMap.at(id);
    }
    else if (id.length() > 2 and id[0] == '#')
    {
        try
        {
            out = std::stoi(id.substr(1, id.length() - 1));
        }
        catch (std::invalid_argument& _)
        {
            out = '&';
        }
    }
    else
    {
        out = '&';
    }

    if (out == 160)// nbsp
    {
        out = ' ';
    }

    return out;
}// end DocumentHtml::parse_html_entity(const string& id)

string   DocumentHtml::decode_text(const string& text)
{
    using namespace std;

    istringstream       inBuf(text);
    string              output      = "";

    while (inBuf)
    {
        output += utils::read_token_until(inBuf, "&");
        if (inBuf and inBuf.peek() == '&')
        {
            inBuf.ignore(1);
            string      entId       = utils::read_token_until(inBuf, ";");

            output += parse_html_entity(entId);
            if (inBuf and inBuf.peek() == ';')
            {
                inBuf.ignore(1);
            }
        }
    }// end while (inBuf)

    return output;
}// end DocumentHtml::decode_text(const string& text)

// XXX class DocumentHtml::Format Implementation XXXXXXXXXXXXXXXXXXXXXXXXXX

// === public accessor(s) =================================================
bool        DocumentHtml::Format::in_list(void) const
{
    return m_flags & F_IN_LIST;
}// end DocumentHtml::Format::in_list(void) const

bool        DocumentHtml::Format::ordered_list(void) const
{
    return m_flags & F_ORDERED_LIST;
}// end DocumentHtml::Format::ordered_list(void) const

bool        DocumentHtml::Format::preformatted(void) const
{
    return m_flags & F_PREFORMATTED;
}// end DocumentHtml::Format::preformatted(void) const

bool        DocumentHtml::Format::block_ignore(void) const
{
    return m_flags & F_BLOCK_IGNORE;
}// end DocumentHtml::Format::block_ignore(void) const

// === public mutator(s) ==================================================
void        DocumentHtml::Format::set_in_list(bool state)
{
    if (state)
        m_flags |= F_IN_LIST;
    else
        m_flags &= ~F_IN_LIST;
}// end DocumentHtml::Format::set_in_list(bool state)

void        DocumentHtml::Format::set_ordered_list(bool state)
{
    if (state)
        m_flags |= F_ORDERED_LIST;
    else
        m_flags &= ~F_ORDERED_LIST;
}// end DocumentHtml::Format::set_ordered_list(bool state)

void        DocumentHtml::Format::set_preformatted(bool state)
{
    if (state)
        m_flags |= F_PREFORMATTED;
    else
        m_flags &= ~F_PREFORMATTED;
}// end DocumentHtml::Format::set_preformatted(bool state)

void        DocumentHtml::Format::set_block_ignore(bool state)
{
    if (state)
        m_flags |= F_BLOCK_IGNORE;
    else
        m_flags &= ~F_BLOCK_IGNORE;
}// end DocumentHtml::Format::set_block_ignore(bool state)
