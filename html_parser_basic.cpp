#include <stack>

#include "deps.hpp"
#include "html_parser_basic.hpp"
#include "dom_tree.hpp"

// === HtmlParserBasic::parse_html(std::istream& ins) const ===============
//
// Parses an html document read from istream <ins> into a DomTree.
//
// Input:
//      an input stream, presumably containing an html document
//
// Output:
//      a DomTree object, containing the data represented by the inputted
//      html document
//
// Throws:
//      HtmlParser::except_invalid_token, if input stream contains a
//      misplaced/invalid character
//
// ========================================================================
DomTree     HtmlParserBasic::parse_html(std::istream& ins) const
{
    using namespace std;

    DomTree                     output;
    stack<string>               tagStack;
    stack<DomTree::node*>       nodeStack;

    // initialize root
    output.reset_root("window");
    nodeStack.push(output.root());

    // while the input stream is not depleted
    utils::ignore_whitespace(ins);
    while (ins)
    {
        const char      nextChar        = ins.peek();

        switch (nextChar)
        {
            case '<':
                push_node(ins, nodeStack, tagStack);
                break;
            default:
                {
                    string      err     = "";

                    ins >> err;
                    throw except_invalid_token(err);
                }
        }// end switch (nextChar)
        utils::ignore_whitespace(ins);
    }// end while (ins)

    return output;
}// end HtmlParserBasic::parse_html(std::istream& ins) const

// === HtmlParserBasic::push_node =========================================
//
// ========================================================================
void     HtmlParserBasic::push_node(
    std::istream& ins,
    std::stack<DomTree::node*>& nodeStack,
    std::stack<string>& tagStack
)
{
    using namespace std;

    // if we get to any of these cases we have a serious logic error
    if (ins.get() != '<')
    {
        string      errorLine       = "";

        getline(ins, errorLine);
        throw except_invalid_token(errorLine);
    }
    else if (nodeStack.empty())
    {
        throw out_of_range("node stack is empty");
    }
    else if (!nodeStack.top())
    {
        throw out_of_range("node stack top node is null");
    }

    DomTree::node       *parentNode     = nodeStack.top();
    string              tag             = "";

    utils::ignore_whitespace(ins);
    // Case 1: end tag
    if (ins.peek() == '/')
    {
        ins.ignore(1);
        utils::ignore_whitespace(ins);
        tag = utils::read_token_snake_case(ins);
        ins.ignore(numeric_limits<streamsize>::max(), '>');
        if (tag != tagStack.top())
        {
            throw except_invalid_token("</" + tag + ">");
        }
        tagStack.pop();
        nodeStack.pop();
        return;
    }
    // Case 2: comment tag
    else if (ins.peek() == '!')
    {
        ins.ignore(numeric_limits<streamsize>::max(), '>');
        return;
    }

    // Case 3: opening tag
    tag = utils::read_token_snake_case(ins);

    if (tag.empty())
    {
        string      errorLine       = "";

        getline(ins, errorLine);
        throw except_invalid_token(errorLine);
    }

    // if we get here, we have a valid token

    DomTree::node&      currNode    = parentNode->emplace_child_back(tag);
    bool                isEmpty     = false;

    utils::ignore_whitespace(ins);
    while (ins && ins.peek() != '>')
    {
        if (ins.peek() == '/')
        {
            ins.ignore(1);
            isEmpty = true;
        }
        else
        {
            auto attribPair = read_tag_attribute(ins);

            currNode.attributes[attribPair.first] = attribPair.second;
            utils::ignore_whitespace(ins);
            isEmpty = false;
        }

        utils::ignore_whitespace(ins);
    }// end while (ins && ins.peek() != '>')

    ins.ignore(1);// remove ending '>' char

    if (isEmpty)
        return;

    utils::ignore_whitespace(ins);

    // prepare stacks for recursive calls
    tagStack.push(tag);
    nodeStack.push(&currNode);

    const size_t        stackSize       = nodeStack.size();
    string              currText        = "";

    while (ins && nodeStack.size() == stackSize)
    {
        // Case 1: child (non-text) node
        if (ins.peek() == '<')
        {
            if (!currText.empty())
            {
                // emplace text node
                currNode.emplace_child_back("text", currText);
                currText.clear();
            }
            push_node(ins, nodeStack, tagStack);
        }
        // Case 2: text node
        else
        {
            if (!currText.empty())
            {
                currText += ' ';
            }
            currText += read_text_token(ins);
        }

        utils::ignore_whitespace(ins);
    }// end while (ins)
}// end HtmlParserBasic::push_node

// == HtmlParserBasic::read_tag_attribute(std::istream& ins) ==============
//
// ========================================================================
std::pair<string,string>
HtmlParserBasic::read_tag_attribute(std::istream& ins)
{
    using namespace std;

    pair<string,string>     output;

    output.first = utils::read_token_snake_case(ins);

    if (!ins || ins.peek() != '=')
        return output;

    ins.ignore(1);

retry:
    if (!ins)
        return output;

    switch (ins.peek())
    {
        case '\'':
            output.second = utils::read_token_squoted(ins);
            break;
        case '"':
            output.second = utils::read_token_dquoted(ins);
            break;
        case '/':
        case '>':
            break;
        // whitespace
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            ins.ignore(1);
            goto retry;
        default:
            string      err     = "";

            ins >> err;
            throw except_invalid_token(err);
    }// end switch (ins.peek())

    return output;
}// end HtmlParserBasic::read_tag_attribute(std::istream& ins)

// === HtmlParserBasic::read_text_token(std::istream& ins) ================
//
// ========================================================================
string   HtmlParserBasic::read_text_token(std::istream& ins)
{
    using namespace std;

    string      output      = "";

    while (ins && ins.peek() != '<')
    {
        if (!output.empty())
        {
            output += ' ';
        }
        output += utils::read_token_until(ins, "<\r\n");
        utils::ignore_whitespace(ins);
    }// end while (ins && ins.peek() != '<')

    return output;
}// end HtmlParserBasic::read_text_token(std::istream& ins)
