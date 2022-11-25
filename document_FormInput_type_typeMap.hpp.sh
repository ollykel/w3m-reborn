#!/bin/bash

# Generates a C++ header file containing static const std::map<string,Type>
# typeMap, which contains the tokens read from stdin. Formatted output
# printed to stdout.
#
# Makefile should feed this script appropriate input (i.e. from a .txt
# file) and output to the appropriate destination (i.e. a .hpp file).

cat << _EOF_
// Does not use include guards, by design.
// Should be #include'd at the appropriate location in document.hpp.

static const std::map<string,Type> typeMap =
{
_EOF_

while read token; do
    token_cpp="$(sed -e 's/-/_/g' <<< "${token}")"
cat << _EOF_
    { "${token}", Type::${token_cpp} },
_EOF_
done

cat << _EOF_
};// end static const std::map<string,Type> typeMap
_EOF_
