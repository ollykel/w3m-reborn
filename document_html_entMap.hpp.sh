#!/bin/bash

cat << _EOF_
static const std::map<string, wchar_t>   entMap  =
{
_EOF_

awk -F '[ \t]*,[ \t]*' '{printf("\t{\"%s\", %s},\n", $1, $2)}'

cat << _EOF_
};// end static const std::map<string, unsigned>   entMap
_EOF_
