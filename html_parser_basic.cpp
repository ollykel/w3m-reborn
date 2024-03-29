#include <stack>

#include "deps.hpp"
#include "utils.hpp"
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
void    HtmlParserBasic::parse_html(
    DomTree::node& root,
    std::istream& ins)
const
{
    using namespace std;

    stack<string>               tagStack;
    stack<DomTree::node*>       nodeStack;

    // initialize node stack
    nodeStack.push(&root);

    // while the input stream is not depleted
    utils::read_token_until(ins, "<");
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
                    err = "\"" + err + "\" (not a valid tag opening)";
                    throw except_invalid_token(err);
                }
        }// end switch (nextChar)
        utils::read_token_until(ins, "<");
    }// end while (ins)
}// end HtmlParserBasic::parse_html(...) const

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
    if (nodeStack.empty())
    {
        throw logic_error("node stack is empty");
    }
    else if (!nodeStack.top())
    {
        throw logic_error("node stack top node is null");
    }

    DomTree::node       *parentNode     = nodeStack.top();
    tag                 currTag         = tag::from_stream(ins);

    switch (currTag.kind)
    {
        case tag::Kind::initial:
            {
                // emplace new node
                DomTree::node&      currNode
                    = parentNode->emplace_child_back(currTag.identifier);
                currNode.attributes = currTag.attributes;

                // check if tag is inherently childless
                if (is_empty_tag(currTag.identifier))
                {
                    return;
                }
                // handle scripts specially
                else if (currTag.identifier == "script"
                    or currTag.identifier == "style")
                {
                    extract_literal_node(ins, currTag.identifier,
                        currNode);
                    return;
                }

                // prepare stacks for recursive calls
                tagStack.push(currTag.identifier);
                nodeStack.push(&currNode);

                const size_t    stackSize   = nodeStack.size();

                while (ins and nodeStack.size() == stackSize)
                {
                    string          currText    = read_text_token(ins);
                    if (not currText.empty())
                    {
                        // emplace text node
                        currNode.emplace_child_back("text", currText);
                        currText.clear();
                    }
                    push_node(ins, nodeStack, tagStack);
                }// end while (ins)
            }
            break;
        case tag::Kind::solo:
            {
                DomTree::node&      currNode
                    = parentNode->emplace_child_back(currTag.identifier);

                currNode.attributes = currTag.attributes;
            }
            break;
        case tag::Kind::terminal:
            {
                while (currTag.identifier != tagStack.top())
                {
                    tagStack.pop();
                    nodeStack.pop();
                }
                tagStack.pop();
                nodeStack.pop();
            }
            break;
        case tag::Kind::comment:
        case tag::Kind::version:
            // Do nothing
            break;
    }// end switch (currTag.kind)
}// end HtmlParserBasic::push_node

void     HtmlParserBasic::extract_literal_node(
    std::istream& ins,
    const string& tagId,
    DomTree::node& nd
)
{
    string          innerText       = "";
    const string    endSentinel     = "/" + tagId;

    while (ins)
    {
        innerText += utils::read_token_until(ins, "<");
        ins.ignore(1);
        if (ins and ins.peek() == '/')
        {
            std::vector<char>   buf(endSentinel.length() + 1);

            ins.read(&buf[0], endSentinel.length());
            if (endSentinel == &buf[0])
            {
                utils::ignore_whitespace(ins);
                ins.ignore(1);// ignore terminal >
                break;
            }
            else
            {
                innerText += &buf[0];
            }
        }
        else
        {
            innerText += '<';
        }
    }// end while (ins)

    nd.emplace_child_back("text", innerText);
}// end HtmlParserBasic::extract_literal_node

// === HtmlParserBasic::read_text_token(std::istream& ins) ================
//
// ========================================================================
string   HtmlParserBasic::read_text_token(std::istream& ins)
{
    using namespace std;

    string      output      = "";

    while (ins and ins.peek() != '<')
    {
        output += utils::read_token_until(ins, "<");
        utils::ignore_whitespace(ins);
    }// end while (ins && ins.peek() != '<')

    return output;
}// end HtmlParserBasic::read_text_token(std::istream& ins)

// === HtmlParserBasic::is_empty_tag(const string& tag) ===================
//
// ========================================================================
bool     HtmlParserBasic::is_empty_tag(const string& tag)
{
    // TODO; generate these from a config/definition file
    const static std::unordered_set<string>     empty_tags = {
        "area",
        "base",
        "br",
        "col",
        "command",
        "embed",
        "hr",
        "img",
        "input",
        "keygen",
        "link",
        "meta",
        "param",
        "source",
        "track",
        "wbr"
    };

    return empty_tags.count(tag);
}// end HtmlParserBasic

// === HtmlParserBasic::tag::from_stream(std::istream& ins) -> tag ========
//
// ========================================================================
auto     HtmlParserBasic::tag::from_stream(std::istream& ins) -> tag
{
    using namespace std;

    tag     output;


    // if we get to this case we have a serious logic error
    if (ins.get() != '<')
    {
        string      error       = "";

        getline(ins, error);
        error = "\"" + error + "\" (not a valid tag opening)";
        throw HtmlParserBasic::except_invalid_token(error);
    }

    utils::ignore_whitespace(ins);
    // Case 1: terminal tag
    if (ins.peek() == '/')
    {
        output.kind = Kind::terminal;
        ins.ignore(1);
        utils::ignore_whitespace(ins);
        output.identifier = utils::read_token_until(ins, " \t\r\n<>/");
        utils::to_lower(output.identifier);
        ins.ignore(numeric_limits<streamsize>::max(), '>');
        return output;
    }
    // Case 2: comment tag
    else if (ins.peek() == '!')
    {
        output.kind = Kind::comment;
        ignore_comment(ins);
        return output;
    }
    else if (ins.peek() == '?')
    {
        output.kind = Kind::version;
        ignore_version(ins);
        return output;
    }

    // Case 3: initial tag
    output.identifier = utils::read_token_until(ins, " \t\r\n<>/");
    utils::to_lower(output.identifier);

    if (output.identifier.empty())
    {
        string      errorLine       = "";

        getline(ins, errorLine);
        throw except_invalid_token(errorLine);
    }

    // if we get here, we have a valid tag

    utils::ignore_whitespace(ins);
    while (ins && ins.peek() != '>')
    {
        if (ins.peek() == '/')
        {
            ins.ignore(1);
            output.kind = Kind::solo;
        }
        else
        {
            auto attribPair = read_attribute(ins);

            output.attributes[attribPair.first] = attribPair.second;
            utils::ignore_whitespace(ins);
            output.kind = Kind::initial;
        }

        utils::ignore_whitespace(ins);
    }// end while (ins && ins.peek() != '>')

    if (ins)
        ins.ignore(1);// remove ending '>' char

    return output;
}// end HtmlParserBasic::tag::from_stream(std::istream& ins) -> tag

void     HtmlParserBasic::tag::ignore_comment(std::istream& ins)
{
    using namespace std;

    string      token       = "";

    ins.ignore(1);// initial !
    ins >> token;

    // Case 1: <!DOCTYPE ...>
    if (token == "DOCTYPE" or token == "doctype" or token == "[CDATA[")
    {
        ins.ignore(numeric_limits<streamsize>::max(), '>');
    }
    // Case 2: <!--...-->
    else if (token.length() > 1 and token[0] == '-' and token[1] == '-')
    {
        string      rest        = "";

        do
        {
            getline(ins, rest, '>');
        } while (
            rest.length() < 2
                or
            (
                rest[rest.length() - 1] != '-'
                    and
                rest[rest.length() - 2] != '-'
            )
        );// end do-while
    }
    else
    {
        throw HtmlParserBasic::except_invalid_token(
            token + " (parsing comment)"
        );
    }
}// end HtmlParserBasic::tag::ignore_comment(std::istream& ins)

void     HtmlParserBasic::tag::ignore_version(std::istream& ins)
{
    using namespace std;

    ins.ignore(1);// initial ?

    while (ins)
    {
        utils::read_token_until(ins, "?");
        if (ins.get() == '>')
            break;
    }// end while (ins)
}// end HtmlParserBasic::tag::ignore_version(std::istream& ins)

// == HtmlParserBasic::tag::read_attribute(std::istream& ins) =============
//
// ========================================================================
std::pair<string,string>
HtmlParserBasic::tag::read_attribute(std::istream& ins)
{
    using namespace std;

    pair<string,string>     output;

    output.first = utils::read_token_until(ins, " \t\r\n<>=");

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
            output.second = utils::read_token_until(ins, " \t\r\n<>=");
    }// end switch (ins.peek())

    return output;
}// end HtmlParserBasic::tag::read_attribute(std::istream& ins)
