# W3M Reborn

w3m-reborn (w3mr) is a terminal user interface (TUI) browser for the
21st century, modelled after Akinori Ito's w3m browser. It expands
upon the features w3m offers by allowing users to write scripts to
enhance their browsing experience. Key features added by w3m reborn
include:

- support for in-browser search engines
- in-browser url redirection using shell utilities (i.e. sed or awk)

## Compatibility

Currently developed for Linux only. Support for other \*nix OSes
will be offered in the future.

## Installation

Using default settings, the application can be compiled and
installed using the following shell commands:

```
./configure.sh
make
make install
```

## Environment

Currently relevant environment variables are:

- W3M\_USER\_AGENT: The user agent to use for HTTP requests
