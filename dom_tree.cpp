#include "deps.hpp"
#include "dom_tree.hpp"

// === convenience definitions ============================================
#define     NODE_T      DomTree::node

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//
//              DomTree Implementation
//
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// === DomTree Default Constructor ========================================
//
// ========================================================================
DomTree::DomTree(void)
{
    // member variables already initialized
}// end DomTree::DomTree(void)

// === DomTree Copy Constructor ===========================================
//
// ========================================================================
DomTree::DomTree(const DomTree& original)
{
    copy_from(original);
}// end DomTree::DomTree(const DomTree& original)

// === DomTree Move Constructor ===========================================
//
// ========================================================================
DomTree::DomTree(DomTree&& original)
{
    move_from(original);
}// end DomTree::DomTree(const DomTree&& original)

// === DomTree Destructor =================================================
//
// ========================================================================
DomTree::~DomTree(void)
{
    destruct();
}// end DomTree::~DomTree(void)

// === DomTree::operator=(const DomTree& other) ===========================
//
// ========================================================================
DomTree&    DomTree::operator=(const DomTree& other)
{
    if (&other != this)
    {
        destruct();
        copy_from(other);
    }
    return *this;
}// end DomTree::operator=(const DomTree& other)

// === DomTree::operator=(DomTree&& other) ==========================
//
// ========================================================================
DomTree&    DomTree::operator=(DomTree&& other)
{
    if (&other != this)
    {
        destruct();
        move_from(other);
    }
    return *this;
}// end DomTree::operator=(DomTree&& other)

// === DomTree::root(void) const -> node* =================================
//
// ========================================================================
auto            DomTree::root(void) const -> node*
{
    return m_root.get();
}// end DomTree::root(void) const -> node*

// === DomTree::size(void) const ==========================================
//
// ========================================================================
size_t          DomTree::size(void) const
{
    return m_root ? m_root->size() : 0;
}// end DomTree::size(void) const

// === DomTree::clear(void) ===============================================
//
// ========================================================================
void            DomTree::clear(void)
{
    m_root.reset();
}// end DomTree::clear(void)

// === DomTree::reset_root(...) -> node* ==================================
//
// ========================================================================
auto DomTree::reset_root(
    const string& identifier,
    const string& text
) -> node*
{
    clear();
    m_root = std::make_unique<node>(identifier, text);
    return m_root.get();
}

// === operator<<(std::ostream& outs, const DomTree& tree) ================
//
// ========================================================================
std::ostream&    operator<<(std::ostream& outs, const DomTree& tree)
{
    using namespace std;

    outs << "DOM Tree:";
    if (tree.m_root)
    {
        outs << endl << *tree.m_root;
    }
    else
    {
        outs << " (empty)";
    }

    return outs;
}// end operator<<(std::ostream& outs, const DomTree& tree)

// === DomTree::copy_from(const DomTree& other) ===========================
//
// ========================================================================
void        DomTree::copy_from(const DomTree& other)
{
    m_root = std::make_unique<node>(*other.m_root);
}// end DomTree::copy_from(const DomTree& other)

// === DomTree::move_from(DomTree& other) =================================
//
// ========================================================================
void        DomTree::move_from(DomTree& other)
{
    m_root.reset(other.m_root.release());
}// end DomTree::move_from(DomTree& other)

// === DomTree::destruct(void) ============================================
//
// ========================================================================
void        DomTree::destruct(void)
{
    m_root.reset(nullptr);
}// end DomTree::destruct(void)

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//
//              DomTree::node (NODE_T) Implementation
//
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// === NODE_T Type Constructor ============================================
//
// ========================================================================
NODE_T::node(const string& identifier, const string& text)
{
    m_identifier = identifier;
    m_text = text;
}// end NODE_T::node(const string& identifier, const string& text)

// === NODE_T Copy Constructor ============================================
//
// ========================================================================
NODE_T::node(const node& original)
{
    copy_from(original);
}// end NODE_T::node(const node& original)

// === NODE_T Move Constructor ============================================
//
// ========================================================================
NODE_T::node(node&& original)
{
    move_from(original);
}// end NODE_T::node(node&& original)

// === NODE_T Destructor ==================================================
//
// ========================================================================
NODE_T::~node(void)
{
    destruct();
}// end NODE_T::~node(void)

// === NODE_T::operator=(const node& other) -> node& ======================
//
// ========================================================================
auto        NODE_T::operator=(const node& other) -> node&
{
    if (&other != this)
    {
        destruct();
        copy_from(other);
    }
    return *this;
}// end NODE_T::operator=(const node& other) -> node&

// === NODE_T::operator=(node&& other) -> node& ===========================
//
// ========================================================================
auto        NODE_T::operator=(node&& other) -> node&
{
    if (&other != this)
    {
        destruct();
        move_from(other);
    }
    return *this;
}// end NODE_T::operator=(node&& other) -> node&

// === NODE_T::size(void) const ===========================================
//
// ========================================================================
size_t      NODE_T::size(void) const
{
    return m_nDescendants + 1;
}// end NODE_T::size(void) const

// === NODE_T::is_text(void) const ========================================
//
// ========================================================================
bool        NODE_T::is_text(void) const
{
    return !m_text.empty();
}// end NODE_T::is_text(void) const

// === NODE_T::identifier(void) const =====================================
//
// ========================================================================
auto        NODE_T::identifier(void) const -> const string&
{
    return m_identifier;
}// end NODE_T::identifier(void) const -> const string&

// === NODE_T::text(void) const -> const string& ==========================
//
// ========================================================================
auto        NODE_T::text(void) const -> const string&
{
    return m_text;
}// end NODE_T::text(void) const -> const string&

// === NODE_T::parent(void) const -> const node* ==========================
//
// ========================================================================
auto        NODE_T::parent(void) const -> const node*
{
    return m_parent;
}// end NODE_T::parent(void) const -> const node*

// === NODE_T::child_front(void) const -> const node& =====================
//
// ========================================================================
auto        NODE_T::child_front(void) const -> const node&
{
    if (is_text())
        throw text_node_childless();
    return m_children.front();
}// end NODE_T::child_front(void) const -> const node&

// === NODE_T::child_back(void) const -> const node& ======================
//
// ========================================================================
auto        NODE_T::child_back(void) const -> const node&
{
    if (is_text())
        throw text_node_childless();
    return m_children.back();
}// end NODE_T::child_back(void) const -> const node&

// === NODE_T::child_at(const size_t index) const =========================
//
// Time Complexity: O(n)
//
// ========================================================================
auto        NODE_T::child_at(size_t index) const -> const node&
{
    if (is_text())
        throw text_node_childless();
    const_iterator it = cbegin();
    for (; index; ++it)
    {
        if (it == cend())
            throw std::out_of_range("child index out of range");
        --index;
    }
    return *it;
}// end NODE_T::child_at(size_t index) const

auto        NODE_T::count_descendants_by_id(
    const string& id,
    int depth
) const -> size_t
{
    if (is_text() or not depth)
        return 0;

    size_t      out     = 0;

    for (auto iter = m_children.cbegin(); iter != m_children.cend(); ++iter)
    {
        const NODE_T& child = *iter;

        if (child.identifier() == id)
            ++out;
        out += child.count_descendants_by_id(id, depth - 1);
    }// end for iter

    return out;
}// end NODE_T::count_descendants_by_id(const string& id) const -> size_t

// === NODE_T::cbegin(void) const -> const_iterator =======================
//
// ========================================================================
auto        NODE_T::cbegin(void) const -> const_iterator
{
    if (is_text())
        throw text_node_childless();
    return m_children.cbegin();
}// end NODE_T::cbegin(void) const -> const_iterator

// === NODE_T::cend(void) const -> const const_iterator ===================
//
// ========================================================================
auto        NODE_T::cend(void) const -> const const_iterator
{
    if (is_text())
        throw text_node_childless();
    return m_children.cend();
}// end NODE_T::cend(void) const -> const const_iterator

// === NODE_T::crbegin(void) const -> const_reverse_iterator ==============
//
// ========================================================================
auto        NODE_T::crbegin(void) const -> const_reverse_iterator
{
    if (is_text())
        throw text_node_childless();
    return m_children.crbegin();
}// end NODE_T::crbegin(void) const -> const_reverse_iterator

// === NODE_T::crend(void) const -> const const_reverse_iterator ==========
//
// ========================================================================
auto        NODE_T::crend(void) const -> const const_reverse_iterator
{
    if (is_text())
        throw text_node_childless();
    return m_children.crend();
}// end NODE_T::crend(void) const -> const const_reverse_iterator

// === NODE_T::clear_children(void) =======================================
//
// ========================================================================
void        NODE_T::clear_children(void)
{
    if (is_text())
        throw text_node_childless();
    m_children.clear();
}// end NODE_T::clear_children(void)

// === NODE_T::pop_child_front(void) ======================================
//
// ========================================================================
void        NODE_T::pop_child_front(void)
{
    if (is_text())
        throw text_node_childless();
    m_children.pop_front();
}// end NODE_T::pop_child_front(void)

// === NODE_T::pop_child_back(void) =======================================
//
// ========================================================================
void        NODE_T::pop_child_back(void)
{
    if (is_text())
        throw text_node_childless();
    m_children.pop_back();
}// end NODE_T::pop_child_back(void)

// === NODE_T::remove_child_at(size_t index) ==============================
//
// ========================================================================
void        NODE_T::remove_child_at(size_t index)
{
    if (is_text())
        throw text_node_childless();
    iterator it = begin();
    for (; index; ++it)
    {
        if (it == end())
            throw std::out_of_range("child index out of range");
        --index;
    }
    m_children.erase(it);
}// end NODE_T::remove_child_at(size_t index)

// === NODE_T::emplace_child_front(...) -> node& ==========================
//
// ========================================================================
auto        NODE_T::emplace_child_front(
    const string identifier,
    const string text
) -> node&
{
    if (is_text())
        throw text_node_childless();
    m_children.emplace_front(identifier, text);
    child_front().set_parent(this);
    return child_front();
}// end NODE_T::emplace_child_front(...) -> node&

// === NODE_T::emplace_child_back(...) -> node& ===========================
//
// ========================================================================
auto        NODE_T::emplace_child_back(
    const string identifier,
    const string text
) -> node&
{
    if (is_text())
        throw text_node_childless();
    m_children.emplace_back(identifier, text);
    child_back().set_parent(this);
    return child_back();
}// end emplace_child_back(...) -> node&

// === NODE_T::emplace_child_at(...) -> node& =============================
//
// ========================================================================
auto        NODE_T::emplace_child_at(
    size_t index,
    const string identifier,
    const string text
) -> node&
{
    if (is_text())
        throw text_node_childless();
    iterator it = begin();
    for (; index; ++it)
    {
        if (it == end())
            throw std::out_of_range("child index out of range");
        --index;
    }
    m_children.emplace(it, identifier, text);
    --it;
    it->set_parent(this);
    return *it;
}// end NODE_T::emplace_child_at(...) -> node&

// === NODE_T::parent(void) -> node* ======================================
//
// ========================================================================
auto        NODE_T::parent(void) -> node*
{
    return m_parent;
}// end NODE_T::parent(void) -> node*

// === NODE_T::child_front(void) -> node& =================================
//
// ========================================================================
auto        NODE_T::child_front(void) -> node&
{
    if (is_text())
        throw text_node_childless();
    return m_children.front();
}// end NODE_T::child_front(void) -> node&

// === NODE_T::child_back(void) -> node& ==================================
//
// ========================================================================
auto        NODE_T::child_back(void) -> node&
{
    if (is_text())
        throw text_node_childless();
    return m_children.back();
}// end NODE_T::child_back(void) -> node&

// === NODE_T::child_at(size_t index) -> node& ============================
//
// ========================================================================
auto        NODE_T::child_at(size_t index) -> node&
{
    if (is_text())
        throw text_node_childless();
    iterator it = begin();
    for (; index; ++it)
    {
        if (it == end())
            throw std::out_of_range("child index out of range");
        --index;
    }
    return *it;
}// end NODE_T::child_at(size_t index) -> node&

// === NODE_T::begin(void) -> iterator ====================================
//
// ========================================================================
auto        NODE_T::begin(void) -> iterator
{
    if (is_text())
        throw text_node_childless();
    return m_children.begin();
}// end NODE_T::begin(void) -> iterator

// === NODE_T::end(void) -> const iterator ================================
//
// ========================================================================
auto        NODE_T::end(void) -> const iterator
{
    if (is_text())
        throw text_node_childless();
    return m_children.end();
}// end NODE_T::end(void) -> const iterator

// === NODE_T::rbegin(void) -> iterator ===================================
//
// ========================================================================
auto        NODE_T::rbegin(void) -> reverse_iterator
{
    if (is_text())
        throw text_node_childless();
    return m_children.rbegin();
}// end NODE_T::rbegin(void) -> reverse_iterator

// === NODE_T::rend(void) -> const reverse_iterator =======================
//
// ========================================================================
auto        NODE_T::rend(void) -> const reverse_iterator
{
    if (is_text())
        throw text_node_childless();
    return m_children.rend();
}// end NODE_T::rend(void) -> const reverse_iterator

// === operator<<( std::ostream& outs, const NODE_T& nd) ==================
//
// ========================================================================
std::ostream& operator<<(std::ostream& outs, const NODE_T& nd)
{
    using namespace std;

    string      indent      = "";
    size_t      index       = 1;

    // indentation
    for (const auto *curr = &nd; curr; curr = curr->m_parent)
        indent += '\t';

    if (nd.is_text())
    {
        outs << "<text> \"" << nd.m_text << "\"";
    }
    else
    {
        outs << "<" << nd.m_identifier;
        if (!nd.attributes.empty())
        {
            for (auto p : nd.attributes)
                outs << ' ' << p.first << "=\"" << p.second << '"';
        }
        outs << "> (" << nd.m_children.size() << ' '
            << (nd.m_children.size() == 1 ? "child" : "children")
            << "):";

        for (auto& curr : nd.m_children)
        {
            outs << endl << indent << "[" << index << "]: " << curr;
            ++index;
        }// end for (auto& nd : m_children)
    }

    return outs;
}// end operator<<( std::ostream& outs, const NODE_T& nd)

// === NODE_T::copy_from(const node& other) ===============================
//
// ========================================================================
void        NODE_T::copy_from(const node& other)
{
    m_parent = nullptr;
    m_children = other.m_children;
    // set parent of children to this
    for (auto& child : m_children)
    {
        child.set_parent(this);
    }
    attributes = other.attributes;
    m_identifier = other.m_identifier;
    m_text = other.m_text;
    m_nDescendants = other.m_nDescendants;
}// end NODE_T::copy_from(const node& other)

// === NODE_T::move_from(node&& other) ====================================
//
// ========================================================================
void        NODE_T::move_from(node& other)
{
    m_parent = nullptr;
    m_children = other.m_children;
    // set parent of children to this
    for (auto& child : m_children)
    {
        child.set_parent(this);
    }
    attributes = other.attributes;
    m_identifier = other.m_identifier;
    m_text = other.m_text;
    m_nDescendants = other.m_nDescendants;
}// end NODE_T::move_from(node&& other)

// === NODE_T::destruct(void) =============================================
//
// ========================================================================
void        NODE_T::destruct(void)
{
    m_children.clear();
    if (m_parent)
    {
        m_parent->decrem_num_children(1);
    }
}// end NODE_T::destruct(void)

// === NODE_T::set_parent(const node *nd) =================================
//
// ========================================================================
void        NODE_T::set_parent(node *nd)
{
    if (m_parent)
    {
        m_parent->decrem_num_children(m_nDescendants + 1);
    }

    m_parent = nd;

    if (m_parent)
    {
        m_parent->increm_num_children(m_nDescendants + 1);
    }
}// end NODE_T::set_parent(const node *nd)

// === NODE_T::increm_num_children(size_t num) ============================
//
// ========================================================================
void        NODE_T::increm_num_children(size_t num)
{
    m_nDescendants += num;
    if (m_parent)
        m_parent->increm_num_children(num);
}// end NODE_T::increm_num_children(size_t num)

// === NODE_T::decrem_num_children(size_t num) ============================
//
// ========================================================================
void        NODE_T::decrem_num_children(size_t num)
{
    m_nDescendants -= num;
    if (m_parent)
        m_parent->decrem_num_children(num);
}// end NODE_T::decrem_num_children(size_t num)

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//
//              DomTree::node::text_node_childless Implementation
//
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// === operator<<(std::ostream&, const NODE_T::text_node_childless&) ======
//
// ========================================================================
std::ostream&   operator<<(
   std::ostream& outs,
   const NODE_T::text_node_childless& exception
)
{
    return outs << "DOM text node cannot have children";
}// end operator<<(std::ostream&, const NODE_T::text_node_childless&)
