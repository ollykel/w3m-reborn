#ifndef __DOM_TREE_HPP__
#define __DOM_TREE_HPP__

#include <list>
#include <map>

#include "deps.hpp"

class   DomTree
{
    public:
        // === public member class(es) ====================================
        class   node;

        // === public constructor(s) ======================================
        DomTree(void);// default
        DomTree(const DomTree& original);// copy
        DomTree(DomTree&& original);// move
        ~DomTree(void);// destructor

        // === public operator(s) =========================================
        DomTree&    operator=(const DomTree& other);
        DomTree&    operator=(DomTree&& other);

        // === public accessor(s) =========================================
        auto            root(void) const -> node*;
        size_t          size(void) const;

        // === public mutator(s) ==========================================
        void            clear(void);
        auto            reset_root(
                            const string identifier,
                            const string text = ""
                        ) -> node*;

        // === friend operator(s) =========================================
        friend std::ostream&    operator<<(
                                    std::ostream& outs,
                                    const DomTree& tree
                                );
    private:
        // === private member variable(s) =================================
        u_ptr<node>     m_root;

        // === private mutator(s) =========================================
        void        copy_from(const DomTree& other);
        void        move_from(DomTree&& other);
        void        destruct(void);
};// end class   DomTree

// === class DomTree::node ================================================
//
// A node in a DOM tree. Can come in two varieties: text nodes and non-text
// nodes:
//  (1) Text nodes contain a chunk of text and no children
//  (2) Non-text nodes contain a list of child nodes and no text
//
// Iterating through a DOM tree node results in iterating through its child
// nodes; throws a text_node_childless exception if attempting to iterate
// through a text node.
//
// ========================================================================
class   DomTree::node
{
    // === friend class(es) ===============================================
    friend class DomTree;

    public:
        // === public member class(es) ====================================
        typedef std::list<node>::iterator           iterator;
        typedef std::list<node>::const_iterator     const_iterator;
        class   text_node_childless;

        // === public constructor(s) ======================================
        node(const string identifier, const string text = "");// type
        node(const node& original);// copy
        node(node&& original);// move
        ~node(void);// destructor

        // === public operator(s) =========================================
        auto        operator=(const node& other) -> node&;
        auto        operator=(node&& other) -> node&;

        // === public accessor(s) =========================================
        size_t      size(void) const;
        bool        is_text(void) const;
        auto        child_front(void) const -> const node&;
        auto        child_back(void) const -> const node&;
        auto        child_at(size_t index) const -> const node&;
        // ------ iterators -----------------------------------------------
        auto        cbegin(void) const -> const_iterator;
        auto        cend(void) const -> const const_iterator;
        auto        crbegin(void) const -> const_iterator;
        auto        crend(void) const -> const const_iterator;

        // === public mutator(s) ==========================================
        void        clear_children(void);
        void        pop_child_front(void);
        void        pop_child_back(void);
        void        remove_child_at(size_t index);
        auto        emplace_child_front(
                        const string identifier,
                        const string text = ""
                    ) -> node&;
        auto        emplace_child_back(
                        const string identifier,
                        const string text = ""
                    ) -> node&;
        auto        emplace_child_at(
                        size_t index,
                        const string identifier,
                        const string text = ""
                    ) -> node&;
        auto        child_at(size_t index) -> node&;
        // ------ iterators -----------------------------------------------
        auto        begin(void) -> iterator;
        auto        end(void) -> const iterator;
        auto        rbegin(void) -> iterator;
        auto        rend(void) -> const iterator;
    private:
        // === private member variable(s) =================================
        node                        *m_parent       = nullptr;
        std::list<node>             m_children;
        std::map<string,string>     m_attributes;
        string                      m_identifier    = "";
        string                      m_text          = "";
        size_t                      m_nChildren     = 0;

        // === private mutator(s) =========================================
        void        copy_from(const node& other);
        void        move_from(node&& other);
        void        destruct(void);
        void        set_parent(const node *nd);
        void        increm_num_children(size_t num);
        void        decrem_num_children(size_t num);
};// end class   DomTree::node

// === class DomTree::node::text_node_childless ===========================
//
// Exception to be thrown if trying to access or insert nodes as children
// of a text node (text nodes should not have children).
//
// ========================================================================
class   DomTree::node::text_node_childless
{
    public:
        // === friend operator(s) =========================================
        friend std::ostream&   operator<<(
                       std::ostream& outs,
                       const DomTree::node::text_node_childless& exception
                   );
};// end class DomTree::node::text_node_childless

#endif
