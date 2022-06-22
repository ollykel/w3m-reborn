#include "deps.hpp"
#include "html_parser.hpp"

// === HtmlParser::except_invalid_token Type Constructor ==================
//
// ========================================================================
HtmlParser::except_invalid_token
::except_invalid_token(const string& token)
{
    set_text(token);
}// end ::except_invalid_token(const string& token)
