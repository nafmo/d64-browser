D64 browser
===========
Copyright 1999-2025 Peter Krefting
A Softwolves Software release.

http://www.softwolves.pp.se/cbm/skapelser/

ABOUT D64 BROWSER
-----------------

This software was written to allow browsing of D64 files, i.e Commodore 1541
disk image files, with a web browser. It allows the user to read the directory
of the D64 image, as well as extract individual files.

If you have Softwolves' [bastext][1] utility installed, it will also allow the user
to display BASIC files as detokenized text, which is also valid input to the
bastext utility to convert it back into a binary program image.

COMPILING
---------

Please see the source code for the configuration options that you need to
change before compiling.

The program should work with any modern C++ compiler on any modern
architecture. It has been tested with GNU C++ Compiler on i386 and amd64
platforms.

VIRTUAL TREE SUPPORT
--------------------

Using Apache and its RewriteEngine, it is possible to use D64 browser without
exposing the CGI parameters. A sample htaccess file would look like this.

    # Mapping for D64 files (mapped as directories, please note the trailing
    # slash in the RewriteRule match).
    RewriteRule ^([^/]*)/$ \
                /cbm/syspd/d64/d64.cgi?path=$1&action=list

    # Mapping for extracting contents of D64 files. The file name part is
    # ignored, the file number and extraction type are the important ones.
    RewriteRule ^([^/]*)/([0-9]*)/(.)/.*$ \
                /cbm/syspd/d64/d64.cgi?path=$1&action=extract&filenum=$2&type=$3

If you do not use the virtual tree format, it can be used as a regular
CGI program. For modern web servers without classic CGI support, use
something like fcgiwrap around the program.

[1]: https://www.softwolves.pp.se/cbm/skapelser/verktyg
