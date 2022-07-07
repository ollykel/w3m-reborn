#ifndef __DATA_TREE_HPP__
#define __DATA_TREE_HPP__

// === Data Tree ==========================================================
//
// A tree structure which can hold an arbitrary number or structure of
// string-to-string and/or string-to-tree relationships.
//
// Useful for parsing yaml or json files.
//
// ========================================================================

#include "deps.hpp"

class   DataTree
{
    public:
        // === public member type(s) ======================================
        typedef     std::pair<string,u_ptr<DataTree> >  child_type;
        class       except_invalid_data;

        // === public constructor(s) ======================================
        DataTree(void);// default

        // === public accessor(s) =========================================
        auto    value(void) const       -> const string&;
        auto    children(void) const    -> const std::vector<child_type>&;

        // === public static function(s) ==================================
        static auto     from_yaml(std::istream& ins)    -> DataTree;
    protected:
        // === protected member variable(s) ===============================
        string                      m_value         = "";
        std::vector<child_type>     m_children      = {};
};// end class DataTree

class DataTree::except_invalid_data : public StringException
{
    public:
        // === public constructor(s) ======================================
        except_invalid_data(const string& text);// default
};// end DataTree::except_invalid_data

#endif
