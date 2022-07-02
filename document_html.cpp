#include <cstdio>
#include <cctype>
#include <sstream>
#include <map>

#include "deps.hpp"
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
    m_dispatcher["form"] = &DocumentHtml::append_form;
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

    redraw(cols);
}// end DocumentHtml::from_string(const string& text, const size_t cols)

// ------ override(s) ---------------------------------------------
void        DocumentHtml::redraw(size_t cols)
{
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
                    append_node(child, cols);
                }
            }// end for child
        }
        else
        {
            append_node(nd, cols);
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
void    DocumentHtml::append_node(const DomTree::node& nd, const size_t cols)
{
    if (nd.is_text())
    {
        append_text(nd, cols);
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
    else if (is_node_header(nd))
    {
        append_hn(nd, cols);
    }
    else if (m_dispatcher.count(nd.identifier()))
    {
        auto func = m_dispatcher.at(nd.identifier());

        (this->*func)(nd, cols);
    }
    else
    {
        append_other(nd, cols);
    }
}// end DocumentHtml::append_node(const DomTree::node& nd, const size_t cols)

void    DocumentHtml::append_children(const DomTree::node& nd, const size_t cols)
{
    for (auto iter = nd.cbegin(); iter != nd.cend(); ++iter)
    {
        const auto& child = *iter;

        append_node(child, cols);
    }// end for (const auto& child : nd)
}// end DocumentHtml::append_children(const DomTree::node& nd, const size_t cols)

// === DocumentHtml::append_str(const string& str, const size_t cols) =====
//
// ========================================================================
void    DocumentHtml::append_str(const string& str, const size_t cols)
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

    if (currLen and inBuf and isspace(inBuf.peek()))
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
                if (not currLen)
                {
                    currLine = token.substr(0, colsLeft);
                    token.erase(0, colsLeft);
                }
                m_buffer.back().emplace_back(currLine);
                m_buffer.emplace_back();
                currLen = 0;
                currLine.clear();
            }
        }// end while (not token.empty())
    }// end while (inBuf)

    m_buffer.back().emplace_back(currLine);
}// end DocumentHtml::append_str(const string& str, const size_t cols)

// === DocumentHtml::append_text(const DomTree::node& text) ===============
//
// Draws a text node at the end of the buffer, broken over several lines if
// necessary. Starts at the end of the last line, if space is available.
//
// ========================================================================
void    DocumentHtml::append_text(const DomTree::node& text, const size_t cols)
{
    append_str(text.text(), cols);
}// end DocumentHtml::append_text(const DomTree::node& text)

// === DocumentHtml::append_a =============================================
//
// Append an anchor. If has an href, add it to list of links and add
// reference to newly added buffer nodes.
//
// ========================================================================
void    DocumentHtml::append_a(const DomTree::node& a, const size_t cols)
{
    if (m_buffer.empty())
    {
        m_buffer.emplace_back();
    }

    // store last line, column indices before appending children
    const size_t    startLineIdx = m_buffer.size() - 1;
    const size_t    startNodeIdx = m_buffer.back().size();

    append_children(a, cols);

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

            if (not currNode.get_link_ref())
            {
                currNode.set_link_ref(linkIdx);
                currLink.append_referer(i, j);
            }
        }// end for j

        j = 0;
    }// end for i
}// end DocumentHtml::append_a(const DomTree::node& a)

// === DocumentHtml::append_br(const DomTree::node& br, const size_t cols)
//
// ========================================================================
void    DocumentHtml::append_br(const DomTree::node& br, const size_t cols)
{
    m_buffer.emplace_back();
}// end DocumentHtml::append_br(const DomTree::node& br, const size_t cols)

// === DocumentHtml::append_form(const DomTree::node& form, const size_t cols)
//
// TODO: actually implement
//
// ========================================================================
void    DocumentHtml::append_form(const DomTree::node& form, const size_t cols)
{
    m_buffer.emplace_back();
    m_buffer.back().emplace_back("<FORM>");
    m_buffer.emplace_back();
}// end DocumentHtml::append_form(const DomTree::node& form, const size_t cols)

// === DocumentHtml::append_hn(const DomTree::node& hn) ===================
//
// Append a header (i.e. h1, h2, h3, etc.)
//
// TODO: actually implement; need to update BufferNode struct to account
// for styling (i.e. "style" enum)
//
// ========================================================================
void    DocumentHtml::append_hn(const DomTree::node& hn, const size_t cols)
{
    m_buffer.emplace_back();
    append_children(hn, cols);
    m_buffer.emplace_back();
}// end DocumentHtml::append_hn(const DomTree::node& hn)

// === DocumentHtml::append_hr(const DomTree::node& hr, const size_t cols)
//
// ========================================================================
void    DocumentHtml::append_hr(const DomTree::node& hr, const size_t cols)
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
}// end DocumentHtml::append_hr(const DomTree::node& hr, const size_t cols)

// === DocumentHtml::append_img(const DomTree::node& img) =================
//
// ========================================================================
void    DocumentHtml::append_img(const DomTree::node& img, const size_t cols)
{
    if (m_buffer.empty())
    {
        m_buffer.emplace_back();
    }

    // store last line, column indices before appending children
    const size_t    startLineIdx = m_buffer.size() - 1;
    const size_t    startNodeIdx = m_buffer.back().size();

    append_children(img, cols);

    if (not img.attributes.count("src"))
    {
        return;
    }

    const size_t        linkIdx     = m_images.size();

    m_links.emplace_back(img.attributes.at("src"));
    auto&               currImg     = m_images.back();

    for (size_t i = startLineIdx, j = startNodeIdx; i < m_buffer.size(); ++i)
    {
        auto&   currLine    = m_buffer.at(i);

        for (; j < currLine.size(); ++j)
        {
            auto&   currNode    = currLine.at(j);

            if (not currNode.get_image_ref())
            {
                currNode.set_image_ref(linkIdx);
                currImg.append_referer(i, j);
            }
        }// end for j

        j = 0;
    }// end for i
}// end DocumentHtml::append_img(const DomTree::node& img)

// === DocumentHtml::append_ul(const DomTree::node& img, const size_t cols)
//
// ========================================================================
void    DocumentHtml::append_ul(const DomTree::node& ul, const size_t cols)
{
    size_t      listDepth       = 0;
    for (const auto *node = &ul; node; node = node->parent())
    {
        if (node->identifier() == "ol" or node->identifier() == "ul")
        {
            ++listDepth;
        }
    }// end for *node
    const size_t    indentSize = std::min(listDepth * m_tabWidth, cols / 2);
    const string    indent(indentSize, ' ');
    for (auto iter = ul.cbegin(); iter != ul.cend(); ++iter)
    {
        const auto& child = *iter;

        m_buffer.emplace_back();
        m_buffer.back().emplace_back(indent);
        if (child.identifier() == "li")
        {
            append_li_ul(child, cols);
        }
        else
        {
            append_node(child, cols);
        }
    }// end for child
    m_buffer.emplace_back();
}// end DocumentHtml::append_ul(const DomTree::node& ul, const size_t cols)

// === DocumentHtml::append_ol(const DomTree::node& ol, const size_t cols)
//
// ========================================================================
void    DocumentHtml::append_ol(const DomTree::node& ol, const size_t cols)
{
    size_t      listDepth       = 0;
    for (const auto *node = &ol; node; node = node->parent())
    {
        if (node->identifier() == "ol" or node->identifier() == "ul")
        {
            ++listDepth;
        }
    }// end for *node
    const size_t    indentSize = std::min(listDepth * m_tabWidth, cols / 2);
    const string    indent(indentSize, ' ');
    size_t          index       = 1;

    for (auto iter = ol.cbegin(); iter != ol.cend(); ++iter)
    {
        const auto& child = *iter;

        m_buffer.emplace_back();
        m_buffer.back().emplace_back(indent);
        if (child.identifier() == "li")
        {
            append_li_ol(child, cols, index++);
        }
        else
        {
            append_node(child, cols);
        }
    }// end for child
    m_buffer.emplace_back();
}// end DocumentHtml::append_ol(const DomTree::node& ol, const size_t cols)

// === DocumentHtml::append_li_ul =========================================
//
// ========================================================================
void    DocumentHtml::append_li_ul(const DomTree::node& li, const size_t cols)
{
    m_buffer.back().emplace_back("* ");
    append_children(li, cols);
}// end DocumentHtml::append_li_ul

// === DocumentHtml::append_li_ol =========================================
//
// ========================================================================
void    DocumentHtml::append_li_ol(
    const DomTree::node& li,
    const size_t cols,
    const size_t index
)
{
    std::stringstream       format;

    format << index << ". ";
    m_buffer.back().emplace_back(format.str());
    append_children(li, cols);
}// end DocumentHtml::append_li_ol

// === DocumentHtml::append_p(const DomTree::node& p) =====================
//
// TODO: implement, with styling (see append_hn)
//
// ========================================================================
void    DocumentHtml::append_p(const DomTree::node& p, const size_t cols)
{
    m_buffer.emplace_back();
    append_children(p, cols);
    m_buffer.emplace_back();
}// end DocumentHtml::append_p(const DomTree::node& p)

// === DocumentHtml::append_table(const DomTree::node& table, const size_t cols)
//
// TODO: actually implement
//
// ========================================================================
void    DocumentHtml::append_table(const DomTree::node& table, const size_t cols)
{
    append_hr(table, cols);
    append_tbody(table, cols);// behavior is mostly the same
    m_buffer.emplace_back();
}// end DocumentHtml::append_table(const DomTree::node& table, const size_t cols)

// === DocumentHtml::append_tbody(const DomTree::node& tbody, const size_t cols)
//
// TODO: actually implement
//
// ========================================================================
void    DocumentHtml::append_tbody(const DomTree::node& tbody, const size_t cols)
{
    for (auto iter = tbody.cbegin(); iter != tbody.cend(); ++iter)
    {
        const auto&     elem        = *iter;

        append_node(elem, cols);
        if (elem.identifier() == "tr")
        {
            append_hr(tbody, cols);
        }
    }// end for iter
}// end DocumentHtml::append_tbody(const DomTree::node& tbody, const size_t cols)

// === DocumentHtml::append_other(const DomTree::node& nd) ================
//
// TODO: actually implement in a meaningful way; current implementation
// just appends all children of the node.
//
// ========================================================================
void    DocumentHtml::append_other(const DomTree::node& nd, const size_t cols)
{
    append_children(nd, cols);
}// end DocumentHtml::append_other(const DomTree::node& nd)

// === protected static function(s) =======================================
size_t  DocumentHtml::line_length(BufferLine& line)
{
    size_t      count       = 0;

    for (auto& node : line)
    {
        count += node.get_text().length();
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
        {"copy",   169},
        {"reg",    174},
        {"commat",  64},
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
        {"nbsp",    160},
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
        {"quot",   34},
        {"apos",   39},
        {"laquo",  171},
        {"raquo",  187},
        {"frac14", 188},
        {"frac12", 189},
        {"frac34", 190},
        {"plus",      43},
        {"times",     215},
        {"divide",    247},
        {"equals",    61},
        {"plusmn",    177},
        {"not",       172},
        {"lt",        60},
        {"gt",        62},
        {"deg",       176},
        {"sup1",      185},
        {"sup2",      178},
        {"sup3",      179},
        {"percnt",    37},
        {"permil",    137}
    };

    unsigned    out     = 0;

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
            out = ' ';
        }
    }
    else
    {
        out = ' ';
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
