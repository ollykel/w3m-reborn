// === Test DomTree =======================================================
//
// ========================================================================

#include "../deps.hpp"
#include "../dom_tree.hpp"

#define     TEST_MSG(NAME)  std::cout << "Testing " << NAME << "..." \
    << std::endl

// XXX Unit Test Declaration(s) XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool        test_emplace(const std::vector<string>& identifiers);

// XXX MAIN XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
int main(void)
{
    using namespace std;

    // --- emplace --------------------------------------------------------
    test_emplace(vector<string>{"p", "a", "p"});

    return EXIT_SUCCESS;
}// end main(void)

// XXX Unit Test Definition(s) XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// === test_emplace(const vector<string>& identifiers) ====================
//
// Create a DomTree::node object, then emplace a number of nodes as its
// children, taking the identifiers from <identifiers>.
//
// After emplacing all children, the size() should return the number of
// identifiers + 1.
//
// ========================================================================
bool        test_emplace(const std::vector<string>& identifiers)
{
    TEST_MSG("emplace");

    using namespace std;

    DomTree::node       parent("test_parent");

    try
    {
        for (const string& id : identifiers)
        {
            parent
                .emplace_child_back(id)
                .emplace_child_back("value", "Foobar");
            if (id == "a")
                parent.child_back()
                    .attributes["href"] = "http://127.0.0.1:8080";
        }// end for (const string& id : identifiers)

        cout << "Node size: " << parent.size() << endl;
        cout << "Node and its children:" << endl;
        cout << parent << endl;

        cout << "Popping back node..." << endl;
        parent.pop_child_back();

        cout << "Node size: " << parent.size() << endl;
        cout << "Node and its children:" << endl;
        cout << parent << endl;
    }
    catch (...)
    {
        cout << "test threw unexpected exception" << endl;
        return false;
    }

    return parent.size() == identifiers.size() + 1;
}// end test_emplace(const vector<string>& identifiers)
