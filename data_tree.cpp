#include <stack>

#include "deps.hpp"
#include "data_tree.hpp"

// === class DataTree Implementation ======================================

// === public constructor(s) ==============================================
DataTree::DataTree(void)
{
    // do nothing
}// end DataTree::DataTree(void)

// === public accessor(s) =================================================
auto    DataTree::value(void) const       -> const string&
{
    return m_value;
}// end DataTree::value

auto    DataTree::children(void) const    -> const std::vector<child_type>&
{
    return m_children;
}// end DataTree::children

// === public static function(s) ==========================================
auto    DataTree::from_yaml(std::istream& ins) -> DataTree
{
    using namespace std;

    DataTree            output;
    stack<DataTree*>    treeStack;

    treeStack.push(&output);

    // TODO: actually implement

    return output;
}// end DataTree::from_yaml

// === class DataTree::except_invalid_data Implementation =================

// === public constructor(s) ==============================================
DataTree::except_invalid_data::except_invalid_data(const string& text)
{
    set_text("invalid data: " + text);
}// end DataTree::except_invalid_data::except_invalid_data
