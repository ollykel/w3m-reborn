#include "deps.hpp"
#include "document.hpp"

// === class Document implementation ======================================
//
// ========================================================================

Document::Document(void)
{
    // do nothing
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

// --- public mutator(s) --------------------------------------------------
void    Document::clear(void)
{
    m_buffer.clear();
    m_links.clear();
    m_images.clear();
    m_forms.clear();
}// end Document::clear(void)

void    Document::set_title(const string& title)
{
    m_title = title;
}// end Document::set_title

auto    Document::emplace_form(string action, string method)
    -> Form&
{
    m_forms.emplace_back(*this, action, method);

    return m_forms.back();
}// end Document::emplace_form

auto    Document::emplace_form_input(
    size_t                  formIndex,
    FormInput::Type         type,
    const DomTree::node     *domNode,
    string                  name,
    string                  value
) -> FormInput&
{
    m_form_inputs.emplace_back(*this, formIndex, type, domNode, name, value);
    m_forms[formIndex].insert_input_index(m_form_inputs.size() - 1);

    return m_form_inputs.back();
}// end Document::emplace_form_input

// === class Document::BufferNode Implementation ==========================
//
// ========================================================================

Document::BufferNode::BufferNode(
    const string& text,
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
    -> const string&
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
void    Document::BufferNode::set_text(const string& text)
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

auto    Document::Form::inputs(void) const
    -> input_const_ptr_container
{
    input_const_ptr_container   out;

    for (const auto& index : m_input_indices)
    {
        out.push_back(&parent()->form_inputs().at(index));
    }// end for

    return out;
}// end Document::Form::inputs

// --- public mutator(s) --------------------------------------------------
void    Document::Form::insert_input_index(size_t index)
{
    m_input_indices.insert(index);
}// end Document::Form::insert_input_index

void    Document::Form::erase_input_index(size_t index)
{
    m_input_indices.erase(index);
}// end Document::Form::erase_input_index

void    Document::Form::clear_input_indices(void)
{
    m_input_indices.clear();
}// end Document::Form::clear_input_indices

void    Document::Form::set_action(const string& action)
{
    m_action = action;
}// end Document::Form::set_action

void    Document::Form::set_method(const string& method)
{
    m_method = method;
}// end Document::Form::set_method

// === Document::FormInput Implementation =================================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
Document::FormInput::FormInput(
    Document&               parent,
    size_t                  formIndex,
    Type                    type,
    const DomTree::node     *domNode,
    string                  name,
    string                  value
)
{
    m_parent = &parent;
    m_formIndex = formIndex;
    m_domNode = domNode;
    m_type = type;
    m_name = name;
    m_value = value;
}

// --- public static method(s) ----------------------------
auto Document::FormInput::type(const string& str)
    -> Type
{
    // static const std::map<string,Type> typeMap
    #include "document_FormInput_type_typeMap.hpp"

    return typeMap.at(str);
}// end Document::FormInput::type

// === Document::BufferIndex Implementation ===============================
//
// ========================================================================

// --- public constructor(s) ----------------------------------------------
Document::BufferIndex::BufferIndex(size_t lnum, size_t cnum)
{
    line = lnum;
    col = cnum;
}// end Document::BufferIndex::BufferIndex(size_t lnum, size_t cnum)
