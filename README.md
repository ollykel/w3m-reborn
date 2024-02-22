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
## Running w3m-reborn

Once the program has been compiled, there are a few environment variables you
will need to set in order to run it. The most important is W3M\_USER\_AGENT,
which is used by the default http/https request handling script. By default, you
may use the following value for W3M\_USER\_AGENT:

``` {bash}
export W3M_USER_AGENT="Lynx/2.8.9rel.1 libwww-FM/2.14 SSL-MM/1.4.1 OpenSSL/1.1.1d"
```

If you want a default home page, set the url as the value of WWW\_HOME. This is
necessary for running w3m-reborn without a url argument. For example, if you
want to set your homepage to duckduckgo.com, run:

``` {bash}
export WWW_HOME="https://duckduckgo.com/"
```

If you want to view file types besides plain text and html using external
programs, make sure your MAILCAPS is set. An example MAILCAPS path is as
follows:

``` {bash}
export MAILCAPS="${HOME}/.mailcap:/usr/etc/mailcap:/etc/mailcap"
```

For information on how to use mailcaps, see `man mailcap.5`.

To run the program:

``` {bash}
w3mr [URL]
```

Remember that a url argument is required if WWW\_HOME is not set.

## Environment

Currently relevant environment variables are:

- W3M\_USER\_AGENT: The user agent to use for HTTP requests. Needed by default
  url-fetching script for http/https requests.
- WWW\_HOME: The url of the home page. Needed when calling w3mr without url
  argument.
- MAILCAPS: Colon-separated list of mailcap files to use. Necessary for handling
  mime types beyond text/plain and text/html.
