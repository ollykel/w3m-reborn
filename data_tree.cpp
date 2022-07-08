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
    stack<size_t>       indentStack;
    size_t              indent      = 0;

    treeStack.push(&output);
    indentStack.push(0);

    // TODO: actually implement
    while (ins)
    {
        string      line        = "";
        size_t      lineIndent  = 0;

        getline(ins, line);

        if (line.empty())
            continue;

        lineIndent = line.find_first_not_of(' ');
        if (line.at(lineIndent) == '\t')
        {
            throw except_invalid_data("yaml cannot contain tabspace");
        }
        // Case 2: indent greater than previous
        else if (lineIndent > indentStack.top())
        {
            indentStack.push(lineIndent);
            treeStack.push(&treeStack.top()->m_children.back());
        }
        // Case 3: indent less than previous
        else if (lineIndent < indentStack.top())
        {
            while (not indentStack.empty() and indentStack.top() > lineIndent)
            {
                indentStack.pop();
                treeStack.pop();
            }// end while
            if (indentStack.empty() or indentStack.top() != lineIndent)
            {
                throw except_invalid_data("inconsistent indent in yaml data");
            }
        }
        // Case 4: indent is the same; do nothing
        //
        // Parse key, possible value pair
        line.erase(0, lineIndent);

        size_t      keyLen      = line.find(':');

        if (keyLen == string::npos)
        {
            throw except_invalid_data("yaml parser found line without colon (:)");
        }
    }// end while ins

    return output;
}// end DataTree::from_yaml

// === class DataTree::except_invalid_data Implementation =================

// === public constructor(s) ==============================================
DataTree::except_invalid_data::except_invalid_data(const string& text)
{
    set_text("invalid data: " + text);
}// end DataTree::except_invalid_data::except_invalid_data
