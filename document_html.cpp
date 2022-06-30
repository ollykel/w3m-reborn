#include <cstdio>
#include <sstream>

#include "deps.hpp"
#include "dom_tree.hpp"
#include "html_parser_basic.hpp"
#include "document.hpp"
#include "document_html.hpp"

// === public constructor(s) ======================================
DocumentHtml::DocumentHtml(void) : Document()
{
    // do nothing
}// end DocumentHtml(void)

DocumentHtml::DocumentHtml(std::istream& ins, const size_t cols) : Document()
{
    from_stream(ins, cols);
}// end DocumentHtml(std::istream& ins, const size_t cols)

DocumentHtml::DocumentHtml(const string& text, const size_t cols) : Document()
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
    // TODO: implement
    m_buffer.clear();
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
        else if (nd.identifier() == "a")
        {
            append_a(nd, cols);
        }
        else if (nd.identifier() == "br")
        {
            append_br(nd, cols);
        }
        else if (nd.identifier() == "form")
        {
            append_form(nd, cols);
        }
        else if (nd.identifier() == "hn")
        {
            append_hn(nd, cols);
        }
        else if (nd.identifier() == "hr")
        {
            append_hr(nd, cols);
        }
        else if (nd.identifier() == "img")
        {
            append_img(nd, cols);
        }
        else if (nd.identifier() == "p")
        {
            append_p(nd, cols);
        }
        else if (nd.identifier() == "table")
        {
            append_table(nd, cols);
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

// === DocumentHtml::append_text(const DomTree::node& text) ===============
//
// Draws a text node at the end of the buffer, broken over several lines if
// necessary. Starts at the end of the last line, if space is available.
//
// ========================================================================
void    DocumentHtml::append_text(const DomTree::node& text, const size_t cols)
{
    using namespace std;

    size_t          currLen     = m_buffer.empty() ?
                                    0 :
                                    line_length(m_buffer.back());
    string          currLine    = "";
    istringstream   inBuf(text.text());

    if (m_buffer.empty())
    {
        m_buffer.emplace_back();
    }

    while (inBuf)
    {
        string      token       = "";

        inBuf >> token;

        while (not token.empty())
        {
            size_t      colsLeft        = cols - currLen;

            if (token.length() <= colsLeft)
            {
                currLine += token;
                currLen += token.length();
                if (inBuf and colsLeft != token.length())
                {
                    currLine += ' ';
                    --currLen;
                }
                token.clear();
            }
            else
            {
                if (currLine.empty())
                {
                    currLine += token.substr(0, colsLeft);
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
    auto&               currLink    = m_links.back();

    m_links.emplace_back(a.attributes.at("href"));

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
    auto&               currImg     = m_images.back();

    m_links.emplace_back(img.attributes.at("src"));

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
    const size_t    indentSize = std::max(listDepth * m_tabWidth, cols / 2);
    const string    indent(indentSize, ' ');
    for (auto iter = ul.cbegin(); iter != ul.cend(); ++iter)
    {
        const auto& child = *iter;
        m_buffer.emplace_back();
        m_buffer.back().emplace_back(indent);
        append_node(child, cols);
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
    const size_t    indentSize = std::max(listDepth * m_tabWidth, cols / 2);
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
    m_buffer.emplace_back();
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
    m_buffer.emplace_back();
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
    m_buffer.emplace_back();
    m_buffer.back().emplace_back("<TABLE>");
    m_buffer.emplace_back();
}// end DocumentHtml::append_table(const DomTree::node& table, const size_t cols)

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
