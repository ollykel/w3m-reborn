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
DocumentHtml::DocumentHtml(const Document::Config& cfg)
    : Document(cfg)
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
    m_dispatcher["input"] = &DocumentHtml::append_input;
    m_dispatcher["ul"] = &DocumentHtml::append_ul;
    m_dispatcher["ol"] = &DocumentHtml::append_ol;
    m_dispatcher["p"] = &DocumentHtml::append_p;
    m_dispatcher["table"] = &DocumentHtml::append_table;
    m_dispatcher["tbody"] = &DocumentHtml::append_tbody;
}// end DocumentHtml(void)

DocumentHtml::DocumentHtml(
    const Document::Config& cfg,
    std::istream& ins,
    const size_t cols
) : DocumentHtml(cfg)
{
    from_stream(ins, cols);
}// end DocumentHtml(std::istream& ins, const size_t cols)

DocumentHtml::DocumentHtml(
    const Document::Config& cfg,
    const string& text,
    const size_t cols
) : DocumentHtml(cfg)
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
    Stacks      stacks;

    m_buffer.clear();
    m_links.clear();
    m_images.clear();
    m_forms.clear();
    m_form_inputs.clear();

    // "NULL" form
    emplace_form("", "");
    stacks.formIndices.push_back(0);

    // default format
    Format      fmt     = {};

    for (auto& nd : *m_dom.root())
    {
        if (nd.identifier() == "document" or nd.identifier() == "html")
        {
            for (auto& child : nd)
            {
                // skip <head>
                if (child.identifier() != "head")
                {
                    append_node(child, cols, fmt, stacks);
                }
            }// end for child
        }
        else
        {
            append_node(nd, cols, fmt, stacks);
        }
    }// end for (auto& nd : *m_dom->root())

    // remove extra spaces from ends of lines
    for (auto& line : m_buffer)
    {
        if (not line.empty())
        {
            // as a friend class, we can directly mutate m_text
            wstring&    text    = line.back().m_text;

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
    DomTree::node& nd,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    const size_t    currLines   = m_buffer.size();
    const size_t    currNodes   = (not currLines) ?
                                    0 : m_buffer.back().size();

    if (nd.is_text())
    {
        append_text(nd, cols, fmt, stacks);
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

        (this->*func)(nd, cols, fmt, stacks);
    }
    else
    {
        append_other(nd, cols, fmt, stacks);
    }

    // if node has an id, add it to m_sections
    if (nd.attributes.count("id"))
    {
        const string&   id      = nd.attributes.at("id");

        if (currLines)
        {
            if (m_buffer.at(currLines - 1).size() > currNodes)
            {
                m_sections[id] = { currLines - 1 , currNodes };
            }
            else if (m_buffer.size() > currLines)
            {
                m_sections[id] = { currLines, 0 };
            }
        }
        else if (m_buffer.size() and m_buffer.front().size())
        {
            m_sections[id] = { 0, 0 };
        }
    }
}// end DocumentHtml::append_node(DomTree::node& nd, const size_t cols, Format fmt, Stacks& stacks)

void    DocumentHtml::append_children(DomTree::node& nd, const size_t cols, Format fmt, Stacks& stacks)
{
    for (auto iter = nd.begin(); iter != nd.end(); ++iter)
    {
        auto& child = *iter;

        append_node(child, cols, fmt, stacks);
    }// end for (const auto& child : nd)
}// end DocumentHtml::append_children(DomTree::node& nd, const size_t cols, Format fmt, Stacks& stacks)

// === DocumentHtml::append_str(const string& str, const size_t cols, Format fmt, Stacks& stacks) =====
//
// ========================================================================
void    DocumentHtml::append_str(
    const string& str,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    using namespace std;

    size_t          currLen     = m_buffer.empty() ?
                                    0 :
                                    line_length(m_buffer.back());
    wstring         currLine    = wstring();
    istringstream   inBuf(str);

    if (m_buffer.empty())
    {
        m_buffer.emplace_back();
    }

    if (m_buffer.back().empty())
    {
        m_buffer.back().emplace_back(wstring(fmt.indent, ' '), true);
        currLen = fmt.indent;
    }

    if (currLen > fmt.indent and inBuf and isspace(inBuf.peek()))
    {
        currLine += ' ';
        ++currLen;
    }

    while (inBuf)
    {
        string      tokenRaw    = "";
        wstring     token       = wstring();

        inBuf >> tokenRaw;
        token = decode_text(tokenRaw);

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
                for (const auto& styler : stacks.styles)
                {
                    m_buffer.back().back().append_styler(styler);
                }// end for styler
                m_buffer.emplace_back();
                if (fmt.indent)
                {
                    currLen = fmt.indent;
                    m_buffer.back().emplace_back(wstring(fmt.indent, ' '), true);
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
    for (const auto& styler : stacks.styles)
    {
        m_buffer.back().back().append_styler(styler);
    }// end for styler
}// end DocumentHtml::append_str(const string& str, const size_t cols, Format fmt, Stacks& stacks)

// === DocumentHtml::append_text(DomTree::node& text) ===============
//
// Draws a text node at the end of the buffer, broken over several lines if
// necessary. Starts at the end of the last line, if space is available.
//
// ========================================================================
void    DocumentHtml::append_text(DomTree::node& text, const size_t cols, Format fmt, Stacks& stacks)
{
    append_str(text.text(), cols, fmt, stacks);
}// end DocumentHtml::append_text(DomTree::node& text)

// === DocumentHtml::append_a =============================================
//
// Append an anchor. If has an href, add it to list of links and add
// reference to newly added buffer nodes.
//
// ========================================================================
void    DocumentHtml::append_a(
    DomTree::node& a,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    if (m_buffer.empty())
    {
        m_buffer.emplace_back();
    }

    // store last line, column indices before appending children
    const size_t    startLineIdx = m_buffer.size() - 1;
    const size_t    startNodeIdx = m_buffer.back().size();

    append_children(a, cols, fmt, stacks);

    if (not a.attributes.count("href"))
    {
        return;
    }

    const size_t        linkIdx     = m_links.size();
    string              linkUrl     = "";

    {
        const string&       src         = a.attributes.at("href");
        size_t              idx         = 0;
        size_t              beg         = 0;

        do
        {
            idx = src.find('&', beg);

            linkUrl += src.substr(beg, idx);
            if (string::npos == idx)
            {
                break;
            }

            beg = idx + 1;
            idx = src.find(';', beg);
            if (string::npos == idx)
            {
                linkUrl.push_back('&');
            }
            else
            {
                linkUrl.push_back(parse_html_entity(src.substr(beg, idx)));
                beg = idx + 1;
            }
        } while (beg < src.size());
    }

    m_links.emplace_back(linkUrl);
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
}// end DocumentHtml::append_a(DomTree::node& a)

// === DocumentHtml::append_br(DomTree::node& br, const size_t cols, Format fmt, Stacks& stacks)
//
// ========================================================================
void    DocumentHtml::append_br(DomTree::node& br, const size_t cols, Format fmt, Stacks& stacks)
{
    m_buffer.emplace_back();
}// end DocumentHtml::append_br(DomTree::node& br, const size_t cols, Format fmt, Stacks& stacks)

void    DocumentHtml::append_div(
    DomTree::node& div,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    begin_block(cols, fmt);
    append_children(div, cols, fmt, stacks);
}// end DocumentHtml::append_div(DomTree::node& br, const size_t cols, Format fmt, Stacks& stacks)

// === DocumentHtml::append_form(DomTree::node& form, const size_t cols, Format fmt, Stacks& stacks)
//
// TODO: actually implement
//
// ========================================================================
void    DocumentHtml::append_form(
    DomTree::node& form,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    static const string     NULL_STR        = "";

    Form                    *newForm        = nullptr;
    // NOTE: will allocate empty strings for these attributes if they are
    // not set. This is intended behavior.
    #define     GET_ATTR(ATTR) (form.attributes.count((ATTR)) ? \
                    form.attributes.at((ATTR)) : \
                    (NULL_STR))
    const string&           action          = GET_ATTR("action");
    const string&           method          = GET_ATTR("method");
    #undef      GET_ATTR

    m_buffer.emplace_back();

    // create new form
    emplace_form(action, method);

    stacks.formIndices.push_back(m_forms.size() - 0x01);
    append_children(form, cols, fmt, stacks);
    stacks.formIndices.pop_back();

    m_buffer.emplace_back();
}// end DocumentHtml::append_form(DomTree::node& form, const size_t cols, Format fmt, Stacks& stacks)

// === DocumentHtml::append_hn(DomTree::node& hn) ===================
//
// Append a header (i.e. h1, h2, h3, etc.)
//
// TODO: actually implement; need to update BufferNode struct to account
// for styling (i.e. "style" enum)
//
// ========================================================================
void    DocumentHtml::append_hn(
    DomTree::node& hn,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    stacks.styles.emplace_back(hn.identifier());

    begin_block(cols, fmt);
    append_children(hn, cols, fmt, stacks);
    m_buffer.emplace_back();

    stacks.styles.pop_back();
}// end DocumentHtml::append_hn(DomTree::node& hn)

// === DocumentHtml::append_hr(DomTree::node& hr, const size_t cols, Format fmt, Stacks& stacks)
//
// ========================================================================
void    DocumentHtml::append_hr(
    DomTree::node& hr,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    static size_t       nCols       = 0;
    static wstring      rule        = wstring();

    if (cols != nCols)
    {
        rule = wstring(cols, '-');
        nCols = cols;
    }

    m_buffer.emplace_back();
    m_buffer.back().emplace_back(rule);
    m_buffer.emplace_back();
}// end DocumentHtml::append_hr(DomTree::node& hr, const size_t cols, Format fmt, Stacks& stacks)

// === DocumentHtml::append_img(DomTree::node& img) =================
//
// ========================================================================
void    DocumentHtml::append_img(
    DomTree::node& img,
    const size_t cols,
    Format fmt,
    Stacks& stacks
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
    append_str(imgText, cols, fmt, stacks);

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
}// end DocumentHtml::append_img(DomTree::node& img)

void    DocumentHtml::append_input(
    DomTree::node& input,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    static const string     NULL_STR                = "";
    static const string     DEFAULT_INPUT_TYPE      = "text";

    #define     GET_ATTR(ATTR, DEF)     (input.attributes.count((ATTR)) ? \
                                            input.attributes.at((ATTR)) : \
                                            (DEF))
    const string&       typeName    = GET_ATTR("type", DEFAULT_INPUT_TYPE);
    const string&       name        = GET_ATTR("name", NULL_STR);
    const string&       value       = GET_ATTR("value", NULL_STR);
    #undef GET_ATTR

    const size_t        formIdx     = stacks.formIndices.back();
    Document::Form&     form        = m_forms.at(formIdx);
    auto                type        = Document::FormInput::type(typeName);
    FormInput&          bufInput    = emplace_form_input(
                                        formIdx,
                                        type,
                                        &input,
                                        name,
                                        value
                                    );
    const size_t        inputIndex  = m_form_inputs.size() - 0x01;

    #define     FMT_FIELD_INC(FMT)  \
    { \
        if (line_length(m_buffer.back()) + (FMT).size() > cols) \
        { \
            m_buffer.emplace_back(); \
        } \
    \
        m_buffer.back().emplace_back((FMT)); \
        m_buffer.back().back().set_input_ref(inputIndex); \
    }

    #define     FMT_FIELD_ENCLOSED(FMT, OPEN, CLOSE)  \
    { \
        const wstring   fmt     = (FMT); \
        \
        if (line_length(m_buffer.back()) + fmt.size() + 0x02 > cols) \
        { \
            m_buffer.emplace_back(); \
        } \
    \
        m_buffer.back().emplace_back((OPEN)); \
        FMT_FIELD_INC(fmt); \
        m_buffer.back().emplace_back((CLOSE)); \
    }

    #define     FMT_FIELD(FMT)  FMT_FIELD_ENCLOSED( \
        FMT, \
        utils::to_wstr("["), \
        utils::to_wstr("]") \
    )

    switch (type)
    {
        case FormInput::Type::hidden:
            if (value.size())
            {
                form.set_value(name, value);
            }
            break;
        // text-based input fields
        // input handlers should take care to validate input based on type
        case FormInput::Type::text:
        case FormInput::Type::email:
        case FormInput::Type::password:
        case FormInput::Type::search:
        case FormInput::Type::url:
        case FormInput::Type::number:
        case FormInput::Type::range:
            {
                const size_t    remLen
                    = value.size() >= m_config.inputWidth.def ?
                    0 : m_config.inputWidth.def - value.size();
                const string    rem     = remLen ?
                    string(remLen, ' ') :
                    "";

                if (value.size())
                {
                    form.set_value(name, value);
                }
                FMT_FIELD(utils::to_wstr(value + rem));
            }
            break;
        // button-based input fields
        case FormInput::Type::button:
        case FormInput::Type::submit:
        case FormInput::Type::reset:
            FMT_FIELD_INC(utils::to_wstr("[<" + value + ">]"));
            break;
        case FormInput::Type::checkbox:
            if (value.size())
            {
                form.set_value(name, value);
            }
            FMT_FIELD(utils::to_wstr(value.size() ? "X" : " "));
            break;
        case FormInput::Type::color:
            if (value.size())
            {
                form.set_value(name, value);
            }
            FMT_FIELD(utils::to_wstr(value.size() ? value : "#RRGGBB"));
            break;
        case FormInput::Type::date:
            if (value.size())
            {
                form.set_value(name, value);
            }
            FMT_FIELD(utils::to_wstr(value.size() ? value : "YYYY-MM-DD"));
            break;
        case FormInput::Type::datetime_local:
            if (value.size())
            {
                form.set_value(name, value);
            }
            FMT_FIELD(utils::to_wstr(value.size() ? value : "YYYY-MM-DD hh:mm:ss"));
            break;
        // file input fields
        case FormInput::Type::file:
        case FormInput::Type::image:
            FMT_FIELD(utils::to_wstr(value.size() ? value : "{{FILE}}"));
            break;
        case FormInput::Type::month:
            if (value.size())
            {
                form.set_value(name, value);
            }
            FMT_FIELD(utils::to_wstr(value.size() ? value : "YYYY-MM"));
            break;
        case FormInput::Type::radio:
            if (input.attributes.count("checked") and value.size())
            {
                form.set_value(name, value);
            }
            FMT_FIELD_ENCLOSED(
                utils::to_wstr(value.size() ? "*" : " "),
                utils::to_wstr("("),
                utils::to_wstr(")")
            );
            break;
        case FormInput::Type::tel:
            if (value.size())
            {
                form.set_value(name, value);
            }
            FMT_FIELD(utils::to_wstr(value.size() ? value : "(+1) (NNN) NNN-NNNN"));
            break;
        case FormInput::Type::time:
            if (value.size())
            {
                form.set_value(name, value);
            }
            FMT_FIELD(utils::to_wstr(value.size() ? value : "hh:mm:ss"));
            break;
        case FormInput::Type::week:
            if (value.size())
            {
                form.set_value(name, value);
            }
            FMT_FIELD(utils::to_wstr(value.size() ? value : "YYYY-WW"));
            break;
        // unhandled/undefined fields
        default:
            {
                if (input.attributes.count("value"))
                {
                    std::stringstream   builder;

                    builder << "[<" << value << ">]";
                    m_buffer.back().emplace_back(
                        utils::to_wstr(builder.str())
                    );
                }
                else
                {
                    // TODO: actually implement
                    m_buffer.back().emplace_back(
                        utils::to_wstr("[XXX]")
                    );
                }

                m_buffer.back().back().set_input_ref(inputIndex);
            }
    }// end switch
    #undef  FMT_FIELD
    #undef  FMT_FIELD_INC
}// end DocumentHtml::append_input

// === DocumentHtml::append_ul(DomTree::node& img, const size_t cols, Format fmt, Stacks& stacks)
//
// ========================================================================
void    DocumentHtml::append_ul(
    DomTree::node& ul,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    begin_block(cols, fmt);
    fmt.set_in_list(true);
    fmt.set_ordered_list(false);
    if (fmt.listLevel)
        fmt.indent += 2;

    for (auto iter = ul.begin(); iter != ul.end(); ++iter)
    {
        auto& child = *iter;

        if (child.identifier() == "li")
        {
            m_buffer.emplace_back();
            append_li_ul(child, cols, fmt, stacks);
        }
        else
        {
            append_node(child, cols, fmt, stacks);
        }
    }// end for child

    m_buffer.emplace_back();
}// end DocumentHtml::append_ul(DomTree::node& ul, const size_t cols, Format fmt, Stacks& stacks)

// === DocumentHtml::append_ol(DomTree::node& ol, const size_t cols, Format fmt, Stacks& stacks)
//
// ========================================================================
void    DocumentHtml::append_ol(
    DomTree::node& ol,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    begin_block(cols, fmt);
    fmt.set_in_list(true);
    fmt.set_ordered_list(true);
    fmt.listIndex = 1;

    for (auto iter = ol.begin(); iter != ol.end(); ++iter)
    {
        auto& child = *iter;

        if (child.identifier() == "li")
        {
            m_buffer.emplace_back();
            append_li_ol(child, cols, fmt, stacks);
            ++fmt.listIndex;
        }
        else
        {
            append_node(child, cols, fmt, stacks);
        }
    }// end for child
    m_buffer.emplace_back();
}// end DocumentHtml::append_ol(DomTree::node& ol, const size_t cols, Format fmt, Stacks& stacks)

// === DocumentHtml::append_li_ul =========================================
//
// ========================================================================
void    DocumentHtml::append_li_ul(
    DomTree::node& li,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    if (fmt.in_list())
    {
        fmt.set_block_ignore(true);
        m_buffer.back().emplace_back(wstring(fmt.indent, ' '), true);
        if (fmt.listLevel & 1)// odd list levels
            m_buffer.back().emplace_back(utils::to_wstr("+ "));
        else
            m_buffer.back().emplace_back(utils::to_wstr("* "));
        ++fmt.listLevel;
        if (fmt.indent < cols / 2)
            fmt.indent += 2;
    }
    fmt.set_in_list(false);
    fmt.set_ordered_list(false);
    append_children(li, cols, fmt, stacks);
}// end DocumentHtml::append_li_ul

// === DocumentHtml::append_li_ol =========================================
//
// ========================================================================
void    DocumentHtml::append_li_ol(
    DomTree::node& li,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    if (fmt.in_list())
    {
        std::stringstream       format;

        fmt.set_block_ignore(true);
        m_buffer.back().emplace_back(wstring(fmt.indent, ' '), true);
        format << fmt.listIndex << ". ";
        m_buffer.back().emplace_back(utils::to_wstr(format.str()));
        if (fmt.indent < cols / 2)
            fmt.indent += format.str().length();
    }
    append_children(li, cols, fmt, stacks);
}// end DocumentHtml::append_li_ol

// === DocumentHtml::append_p(DomTree::node& p) =====================
//
// TODO: implement, with styling (see append_hn)
//
// ========================================================================
void    DocumentHtml::append_p(
    DomTree::node& p,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    begin_block(cols, fmt);
    fmt.set_block_ignore(false);
    append_children(p, cols, fmt, stacks);
    // append an extra newline to denote new paragraph
    m_buffer.emplace_back();
    m_buffer.emplace_back();
}// end DocumentHtml::append_p(DomTree::node& p)

// === DocumentHtml::append_table(DomTree::node& table, const size_t cols, Format fmt, Stacks& stacks)
//
// TODO: actually implement
//
// ========================================================================
void    DocumentHtml::append_table(
    DomTree::node& table,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    begin_block(cols, fmt);
    append_hr(table, cols, fmt, stacks);
    append_tbody(table, cols, fmt, stacks);// behavior is mostly the same
    m_buffer.emplace_back();
}// end DocumentHtml::append_table(DomTree::node& table, const size_t cols, Format fmt, Stacks& stacks)

// === DocumentHtml::append_tbody(DomTree::node& tbody, const size_t cols, Format fmt, Stacks& stacks)
//
// TODO: actually implement
//
// ========================================================================
void    DocumentHtml::append_tbody(
    DomTree::node& tbody,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    for (auto iter = tbody.begin(); iter != tbody.end(); ++iter)
    {
        auto&     elem        = *iter;

        append_node(elem, cols, fmt, stacks);
        if (elem.identifier() == "tr")
        {
            append_hr(tbody, cols, fmt, stacks);
        }
    }// end for iter
}// end DocumentHtml::append_tbody(DomTree::node& tbody, const size_t cols, Format fmt, Stacks& stacks)

// === DocumentHtml::append_other(DomTree::node& nd) ================
//
// TODO: actually implement in a meaningful way; current implementation
// just appends all children of the node.
//
// ========================================================================
void    DocumentHtml::append_other(
    DomTree::node& nd,
    const size_t cols,
    Format fmt,
    Stacks& stacks
)
{
    append_children(nd, cols, fmt, stacks);
}// end DocumentHtml::append_other(DomTree::node& nd)

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

wchar_t     DocumentHtml::parse_html_entity(const string& id)
{
    // static const std::map<string, unsigned>   entMap
    #include "document_html_entMap.gen.hpp"

    wchar_t     out     = '&';

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

wstring  DocumentHtml::decode_text(const string& text)
{
    using namespace std;

    istringstream       inBuf(text);
    wstring             output      = wstring();

    while (inBuf)
    {
        output += utils::to_wstr(utils::read_token_until(inBuf, "&"));
        if (inBuf and inBuf.peek() == '&')
        {
            inBuf.ignore(1);
            string      entId       = utils::read_token_until(inBuf, ";");

            output.push_back(parse_html_entity(entId));
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
