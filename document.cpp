#include <climits>
#include <unistd.h>

#include "deps.hpp"
#include "utils.hpp"
#include "document.hpp"

// === class Document implementation ======================================
//
// ========================================================================

Document::Document(const Config& cfg)
{
    m_config = cfg;
}// end Document::Document(void)

// --- public accessor(s) -------------------------------------------------

auto Document::title(void) const
    -> const string&
{
    return m_title;
}// end Document::title

auto Document::buffer(void) const
    -> const buffer_type&
{
    return m_buffer;
}// end Document::buffer

auto Document::links(void) const
    -> const link_container&
{
    return m_links;
}// end Document::links

auto Document::images(void) const
    -> const image_container&
{
    return m_images;
}// end Document::images

auto Document::forms(void) const
    -> const form_container&
{
    return m_forms;
}// end Document::forms

auto Document::form_inputs(void) const
    -> const form_input_container&
{
    return m_form_inputs;
}// end Document::form_inputs

auto Document::buffer_iter(BufPos pos)
    -> buffer_node_iterator
{
    if (m_buffer.empty())
    {
        return { m_buffer, 0, 0, 0 };
    }

    switch (pos)
    {
        case BufPos::begin:
            return { m_buffer, 0, 0, 0 };
        case BufPos::end:
            {
                auto    nodeIter    = m_buffer.back().begin();
                size_t  columns     = 0;

                for (size_t i = m_buffer.back().size(); i > 1; ++i)
                {
                    columns += nodeIter->text().length();
                    ++nodeIter;
                }// end for

                return {
                    m_buffer,
                    m_buffer.size() - 1, m_buffer.back().size() - 1,
                    columns
                };
            }
        default:
            throw std::logic_error("unrecognized BufPos");
    }// end switch
}// end Document::buffer_iter

auto Document::buffer_iter(size_t lineIdx, size_t nodeIdx)
    -> buffer_node_iterator
{
    if (m_buffer.empty())
    {
        return {};
    }
    if (lineIdx >= m_buffer.size())
    {
        return { m_buffer, m_buffer.size(), 0, 0 };
    }
    if (nodeIdx >= m_buffer.at(lineIdx).size())
    {
        size_t      columns     = 0;

        for (const auto& node : m_buffer.back())
        {
            columns += node.text().length();
        }// end for

        return {
            m_buffer, lineIdx, m_buffer.at(lineIdx).size(), columns
        };
    }

    {
        auto        nodeIter    = m_buffer.at(nodeIdx).begin();
        size_t      columns     = 0;

        for (size_t i = nodeIdx; i; --i)
        {
            columns += nodeIter->text().length();
            ++nodeIter;
        }// end for

        return { m_buffer, lineIdx, nodeIdx, columns };
    }
}// end Document::buffer_iter

auto Document::get_section_index(const string& id) const
    -> buffer_index_type
{
    if (not m_sections.count(id))
    {
        return { SIZE_MAX, SIZE_MAX };
    }
    return m_sections.at(id);
}// end Document::get_section_index

// --- public mutator(s) --------------------------------------------------
void    Document::clear(void)
{
    m_buffer.clear();
    m_links.clear();
    m_images.clear();
    m_forms.clear();
    m_form_inputs.clear();
    m_sections.clear();
}// end Document::clear(void)

void    Document::set_title(const string& title)
{
    m_title = title;
}// end Document::set_title

auto Document::forms(void)
    -> form_container::iterator
{
    return m_forms.begin();
}// end Document::forms

auto Document::form_inputs(void)
    -> form_input_container::iterator
{
    return m_form_inputs.begin();
}// end Document::form_inputs

auto    Document::emplace_form(string action, string method)
    -> Form&
{
    m_forms.emplace_back(*this, action, method);

    return m_forms.back();
}// end Document::emplace_form

auto    Document::emplace_form_input(
    size_t              formIndex,
    FormInput::Type     type,
    DomTree::node       *domNode,
    string              name,
    string              value
) -> FormInput&
{
    const size_t    index       = m_form_inputs.size();

    m_form_inputs.emplace_back(FormInput(
        *this,
        index,
        formIndex,
        type,
        domNode,
        name,
        value
    ));

    return m_form_inputs.back();
}// end Document::emplace_form_input

// === class Document::BufferNode Implementation ==========================
//
// ========================================================================

Document::BufferNode::BufferNode(
    const wstring& text,
    const bool isReserved,
    const cont::Ref& link,
    const cont::Ref& image,
    const cont::Ref& input
)
{
    m_text = text;
    m_isReserved = isReserved;
    m_linkRef = link;
    m_imageRef = image;
    m_inputRef = input;
}// end Document::BufferNode::BufferNode(string text, linkIdx, imageIdx)

// --- public accessor(s) -------------------------------------------------
auto Document::BufferNode::text(void) const
    -> const wstring&
{
    return m_text;
}

auto Document::BufferNode::reserved(void) const
    -> bool
{
    return m_isReserved;
}// end Document::BufferNode::reserved(void) const

auto Document::BufferNode::link_ref(void) const
    -> const cont::Ref&
{
    return m_linkRef;
}

auto Document::BufferNode::image_ref(void) const
    -> const cont::Ref&
{
    return m_imageRef;
}

auto Document::BufferNode::input_ref(void) const
    -> const cont::Ref&
{
    return m_inputRef;
}

auto Document::BufferNode::stylers(void) const
    -> const std::vector<string>&
{
    return m_stylers;
}// end Document::BufferNode::stylers(void) const

// --- public mutator(s) --------------------------------------------------
void    Document::BufferNode::set_text(const wstring& text)
{
    m_text = text;
}// end Document::BufferNode::set_text(const string& text)

void    Document::BufferNode::set_reserved(const bool state)
{
    m_isReserved = state;
}// end Document::BufferNode::set_reserved(const bool state)

void    Document::BufferNode::set_link_ref(const size_t index)
{
    m_linkRef = index;
}// end Document::BufferNode::set_link_ref(const size_t index)

void    Document::BufferNode::set_image_ref(const size_t index)
{
    m_imageRef = index;
}// end Document::BufferNode::set_image_ref(const size_t index)

void    Document::BufferNode::set_input_ref(const size_t index)
{
    m_inputRef = index;
}// end Document::BufferNode::set_input_ref(const size_t index)

void    Document::BufferNode::append_styler(const string& styler)
{
    m_stylers.emplace_back(styler);
}// end Document::BufferNode::append_styler(const string& styler)

void    Document::BufferNode::clear_text(void)
{
    m_text.clear();
}// end Document::BufferNode::clear_text(void)

void    Document::BufferNode::clear_link_ref(void)
{
    m_linkRef.clear();
}// end Document::BufferNode::clear_link_ref(void)

void    Document::BufferNode::clear_image_ref(void)
{
    m_imageRef.clear();
}// end Document::BufferNode::clear_image_ref(void)

void    Document::BufferNode::clear_stylers(void)
{
}// end Document::BufferNode::clear_stylers(void)

// === class Document::Reference Implementation ===========================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
Document::Reference::Reference(const string& url)
{
    m_url = url;
}// end Document::Reference::Reference(const string& url)

// --- public accessor(s) -------------------------------------------------
auto Document::Reference::get_url(void) const
    -> const string&
{
    return m_url;
}// end Document::Reference::get_url(void) const

auto Document::Reference::referers(void) const
    -> const referer_container&
{
    return m_referers;
}// end Document::Reference::referers

// --- public mutator(s) --------------------------------------------------

void Document::Reference::set_url(const string& url)
{
    m_url = url;
}// end Document::Reference::set_url(const string& url)

void Document::Reference::append_referer(size_t line, size_t col)
{
    m_referers.emplace_back(line, col);
}// end Document::Reference::append_referer(size_t line, size_t col)

void Document::Reference::clear_referers(void)
{
    m_referers.clear();
}// end Document::Reference::clear_referers(void)

// === Document::Form Implementation ======================================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
Document::Form::Form(
    Document&   parent,
    string      action,
    string      method
)
{
    m_parent = &parent;
    m_action = action;
    m_method = method;
}// end Document::Form::Form

// --- public accessor(s) -------------------------------------------------
auto    Document::Form::parent(void) const
    -> const Document*
{
    return m_parent;
}// end Document::Form::parent

auto    Document::Form::action(void) const
    -> const string&
{
    return m_action;
}// end Document::Form::action

auto    Document::Form::method(void) const
    -> const string&
{
    return m_method;
}// end Document::Form::method

auto    Document::Form::active_inputs(void) const
    -> const input_idx_container_map&
{
    return m_activeInputs;
}// end Document::Form::active_inputs

auto    Document::Form::value(const string& key) const
    -> string
{
    const auto&     formInputs  = m_parent->form_inputs();

    if (not m_activeInputs.count(key))
    {
        return "";
    }

    return utils::join_str(m_activeInputs.at(key), ",",
        [&formInputs](size_t idx)
        {
            return formInputs[idx].value();
        }
    );
}// end Document::Form::value

auto    Document::Form::values(void) const
    -> std::map<string,string>
{
    std::map<string,string>     out     = {};

    for (const auto& kv : m_activeInputs)
    {
        const string&   key     = kv.first;
        string          val     = value(key);

        out[key] = val;
    }// end for

    return out;
}// end Document::Form::values

// --- public mutator(s) --------------------------------------------------
void    Document::Form::set_action(const string& action)
{
    m_action = action;
}// end Document::Form::set_action

void    Document::Form::set_method(const string& method)
{
    m_method = method;
}// end Document::Form::set_method

// --- private mutators ---------------------------------------------------
void    Document::Form::insert_input(size_t index)
{
    const auto&     formInput   = m_parent->form_inputs()[index];

    m_activeInputs[formInput.name()].insert(index);
}// end Document::Form::insert_input

void    Document::Form::erase_inputs(const string& key)
{
    m_activeInputs.erase(key);
}// end Document::Form::erase_inputs

void    Document::Form::remove_input(size_t index)
{
    const auto&     formInput   = m_parent->form_inputs()[index];
    const string&   key         = formInput.name();

    if (m_activeInputs.count(key))
    {
        m_activeInputs.at(key).erase(index);

        if (m_activeInputs.at(key).empty())
        {
            m_activeInputs.erase(key);
        }
    }
}// end Document::Form::remove_input

void    Document::Form::clear_active_inputs(void)
{
    m_activeInputs.clear();
}// end Document::Form::clear_active_inputs

// === Document::FormInput Implementation =================================
//
// ========================================================================

// --- public static method(s) ----------------------------
auto Document::FormInput::type(const string& str)
    -> Type
{
    // static const std::map<string,Type> typeMap
    #include "document_FormInput_type_typeMap.gen.hpp"

    return typeMap.at(str);
}// end Document::FormInput::type

// --- public accessor(s) -------------------------------------------------
auto    Document::FormInput::form(void) const
    -> Document::Form&
{
    return m_parent->m_forms.at(m_formIndex);
}// end Document::FormInput::form

auto    Document::FormInput::type(void) const
    -> Type
{
    return m_type;
}// end Document::FormInput::type

auto    Document::FormInput::name(void) const
    -> const string&
{
    return m_name;
}// end Document::FormInput::name

auto    Document::FormInput::value(void) const
    -> const string&
{
    return m_value;
}// end Document::FormInput::value

auto    Document::FormInput::is_active(void) const
    -> bool
{
    return m_isActive;
}// end Document::FormInput::is_active

// --- public mutator(s) --------------------------------------------------
void    Document::FormInput::set_type(Type t)
{
    m_type = t;
}// end Document::FormInput::set_type

void    Document::FormInput::set_name(const string& name)
{
    m_name = name;

    if (m_domNode)
    {
        m_domNode->attributes["name"] = name;
    }
}// end Document::FormInput::set_name

void    Document::FormInput::set_value(const string& value)
{
    m_value = value;

    if (m_domNode)
    {
        m_domNode->attributes["value"] = value;
    }
}// end Document::FormInput::set_value

void    Document::FormInput::set_is_active(bool state)
{
    m_isActive = state;

    if (m_isActive)
    {
        form().insert_input(m_index);
    }
    else
    {
        form().remove_input(m_index);
    }

    if (m_domNode)
    {
        switch (m_type)
        {
            case Document::FormInput::Type::checkbox:
                {
                    if (m_isActive)
                    {
                        form().insert_input(m_index);
                        m_domNode->attributes["checked"] = "1";
                    }
                    else
                    {
                        form().remove_input(m_index);
                        m_domNode->attributes.erase("checked");
                    }
                }
                break;
            case Document::FormInput::Type::radio:
                {
                    if (m_isActive)
                    {
                        if (form().active_inputs().count(name()))
                        {
                            auto    otherInputIndices   = form()
                                .active_inputs().at(name());

                            for (size_t idx : otherInputIndices)
                            {
                                auto&   formInput   = m_parent->form_inputs()[idx];

                                formInput.set_is_active(false);
                            }// end for
                        }
                        m_domNode->attributes["checked"] = "1";
                    }
                    else
                    {
                        form().remove_input(m_index);
                        m_domNode->attributes.erase("checked");
                    }
                }
                break;
            default:
                {
                    if (m_isActive)
                    {
                        form().insert_input(m_index);
                    }
                    else
                    {
                        form().remove_input(m_index);
                    }
                }
                break;
        }// end switch
    }
}// end Document::FormInput::set_is_active

void    Document::FormInput::push_buffer_node(Document::BufferNode* bufNode)
{
    m_bufNodes.push_back(bufNode);
}// end Document::FormInput::insert_buffer_node

void    Document::FormInput::clear_buffer_nodes(void)
{
    m_bufNodes.clear();
}// end Document::FormInput::clear_buffer_nodes

// --- private constructors -----------------------------------------------
Document::FormInput::FormInput(
    Document&               parent,
    size_t                  index,
    size_t                  formIndex,
    Type                    type,
    DomTree::node           *domNode,
    string                  name,
    string                  value
)
{
    m_parent = &parent;
    m_index = index;
    m_formIndex = formIndex;
    m_domNode = domNode;
    m_type = type;
    m_name = name;
    m_value = value;
}// end Document::FormInput

// === Document::BufferIndex Implementation ===============================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
Document::BufferIndex::BufferIndex(size_t lnum, size_t cnum)
{
    line = lnum;
    col = cnum;
}// end Document::BufferIndex::BufferIndex(size_t lnum, size_t cnum)

// === Document::buffer_index_type Implementation =========================
//
// ========================================================================

// --- public accessors ---------------------------------------------------
Document::buffer_index_type::operator bool(void) const
{
    return (line != SIZE_MAX) and (node != SIZE_MAX);
}// end operator bool

// === Document::buffer_node_iterator Implementation ======================
//
// ========================================================================

// --- public constructors ------------------------------------------------
Document::buffer_node_iterator::buffer_node_iterator(void)
{
    // do nothing
}// end void constructor

// --- public accessors ---------------------------------------------------
Document::buffer_node_iterator::operator bool(void) const
{
    return m_buffer and (m_lineIdx < m_buffer->size());
}// end Document::buffer_node_iterator::operator bool

auto Document::buffer_node_iterator::operator*(void) const
    -> Document::BufferNode&
{
    return node();
}// end Document::buffer_node_iterator::operator*

auto Document::buffer_node_iterator::line_index(void) const
    -> size_t
{
    return m_lineIdx;
}// end Document::buffer_node_iterator::line_index

auto Document::buffer_node_iterator::line(void) const
    -> Document::BufferLine&
{
    return m_buffer->at(line_index());
}// end Document::buffer_node_iterator::line

auto Document::buffer_node_iterator::node_index(void) const
    -> size_t
{
    return m_nodeIdx;
}// end Document::buffer_node_iterator::node_index

auto Document::buffer_node_iterator::node(void) const
    -> Document::BufferNode&
{
    return line().at(node_index());
}// end Document::buffer_node_iterator::node

auto Document::buffer_node_iterator::column(void) const
    -> size_t
{
    return m_column;
}// end Document::buffer_node_iterator::column

auto Document::buffer_node_iterator::at_line_end(void) const
    -> bool
{
    return (*this) and (node_index() >= line().size());
}// end Document::buffer_node_iterator::at_line_end

// --- public mutators ----------------------------------------------------
auto Document::buffer_node_iterator::operator++(void)
    -> type&
{
    if (not (*this))
    {
        goto finally;
    }

    if (at_line_end())
    {
        ++m_lineIdx;
        m_nodeIdx = 0;
        m_column = 0;
    }
    else
    {
        m_column += node().text().length();
        ++m_nodeIdx;
    }
finally:
    return *this;
}// end Document::buffer_node_iterator::operator++

auto Document::buffer_node_iterator::operator++(int _)
    -> type
{
    type    ret     = *this;

    ++(*this);

    return ret;
}// end Document::buffer_node_iterator::operator++

// --- friend functions ---------------------------------------------------
auto operator==(
    const Document::buffer_node_iterator& a,
    const Document::buffer_node_iterator& b
) -> bool
{
    return (a.m_buffer == b.m_buffer)
        and (a.m_lineIdx == b.m_lineIdx)
        and (a.m_nodeIdx == b.m_nodeIdx);
}// end operator==

auto operator!=(
    const Document::buffer_node_iterator& a,
    const Document::buffer_node_iterator& b
) -> bool
{
    return not (a == b);
}// end operator!=

// --- private constructors -----------------------------------------------
Document::buffer_node_iterator::buffer_node_iterator(
    Document::buffer_type& buffer,
    size_t lineIdx,
    size_t nodeIdx,
    size_t column
)
{
    m_buffer = &buffer;
    m_lineIdx = lineIdx;
    m_nodeIdx = nodeIdx;
    m_column = column;
}// end Document::buffer_node_iterator::buffer_node_iterator
