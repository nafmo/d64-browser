// D64 browser
//
// Copyright (c) 1999 Peter Karlsson
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <wchar.h>

#pragma pack(1)

// Adapt these defines to fit your system ---------------------------------
// Command to invoke to display BASIC programs
#define BASTEXT "/home/peter/bin/bastext -ia %s 2> /dev/null"

// Where to put temporary files
#define TMPNAME "/tmp/d64-XXXXXX"

// How large can a file be?
#define MAXSIZE 664

// VIRTUAL - Use a virtual tree:
//  basedir/path/                     for action=list
//  basedir/path/filenum/type/name    for action=extract
// These need to be mapped via htaccess rewrites. "name" is not used.
// If not defined, regular CGI links will be used
#define VIRTUAL

#ifndef VIRTUAL
# define SELF "d64.cgi"
#endif

// You should not need to change anything below here ----------------------

// Structure for BAM (18,0)
struct bam_s
{
    char    t, s;
    char    format;
    char    unused1;
    char    bitmap[140];
    char    diskname[16];
    char    fill1[2];
    char    id[5];
    char    fill2[4];
    char    unused2[86];
};

// Structure for directory entries
struct dirent_s
{
    char            filler[2];
    unsigned char   filetype;
    unsigned char   t, s;
    char            name[16];
    unsigned char   st, ss;
    char            rlen;
    char            unused;
    unsigned char   year, month, day; // GEOS
    unsigned char   hour, minute; // GEOS. Also: @SAVE temporary storage
    unsigned char   length[2];
};

// Structure for directory blocks (chain starting at 18,1)
union dirblock_u
{
    dirent_s    file[8];
    struct
    {
        char    t, s;
    } link;
};

// Unicode PETSCII table
wchar_t petsciiunicode[256] =
{
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // WHITE COLOR SWITCH (CUS)
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // DISABLE CHARACTER SET SWITCHING (CUS)
    0xFFFD, // ENABLE CHARACTER SET SWITCHING (CUS)
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0x000D, // CARRIAGE RETURN
    0x000E, // SHIFT OUT
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // CURSOR DOWN (CUS)
    0xFFFD, // REVERSE VIDEO ON (CUS)
    0xFFFD, // HOME (CUS)
    0x007F, // DELETE
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // RED COLOR SWITCH (CUS)
    0xFFFD, // CURSOR RIGHT (CUS)
    0xFFFD, // GREEN COLOR SWITCH (CUS)
    0xFFFD, // BLUE COLOR SWITCH (CUS)
    0x0020, // SPACE
    0x0021, // EXCLAMATION MARK
    0x0022, // QUOTATION MARK
    0x0023, // NUMBER SIGN
    0x0024, // DOLLAR SIGN
    0x0025, // PERCENT SIGN
    0x0026, // AMPERSAND
    0x0027, // APOSTROPHE
    0x0028, // LEFT PARENTHESIS
    0x0029, // RIGHT PARENTHESIS
    0x002A, // ASTERISK
    0x002B, // PLUS SIGN
    0x002C, // COMMA
    0x002D, // HYPHEN-MINUS
    0x002E, // FULL STOP
    0x002F, // SOLIDUS
    0x0030, // DIGIT ZERO
    0x0031, // DIGIT ONE
    0x0032, // DIGIT TWO
    0x0033, // DIGIT THREE
    0x0034, // DIGIT FOUR
    0x0035, // DIGIT FIVE
    0x0036, // DIGIT SIX
    0x0037, // DIGIT SEVEN
    0x0038, // DIGIT EIGHT
    0x0039, // DIGIT NINE
    0x003A, // COLON
    0x003B, // SEMICOLON
    0x003C, // LESS-THAN SIGN
    0x003D, // EQUALS SIGN
    0x003E, // GREATER-THAN SIGN
    0x003F, // QUESTION MARK
    0x0040, // COMMERCIAL AT
    0x0061, // LATIN SMALL LETTER A
    0x0062, // LATIN SMALL LETTER B
    0x0063, // LATIN SMALL LETTER C
    0x0064, // LATIN SMALL LETTER D
    0x0065, // LATIN SMALL LETTER E
    0x0066, // LATIN SMALL LETTER F
    0x0067, // LATIN SMALL LETTER G
    0x0068, // LATIN SMALL LETTER H
    0x0069, // LATIN SMALL LETTER I
    0x006A, // LATIN SMALL LETTER J
    0x006B, // LATIN SMALL LETTER K
    0x006C, // LATIN SMALL LETTER L
    0x006D, // LATIN SMALL LETTER M
    0x006E, // LATIN SMALL LETTER N
    0x006F, // LATIN SMALL LETTER O
    0x0070, // LATIN SMALL LETTER P
    0x0071, // LATIN SMALL LETTER Q
    0x0072, // LATIN SMALL LETTER R
    0x0073, // LATIN SMALL LETTER S
    0x0074, // LATIN SMALL LETTER T
    0x0075, // LATIN SMALL LETTER U
    0x0076, // LATIN SMALL LETTER V
    0x0077, // LATIN SMALL LETTER W
    0x0078, // LATIN SMALL LETTER X
    0x0079, // LATIN SMALL LETTER Y
    0x007A, // LATIN SMALL LETTER Z
    0x00E4, // LATIN SMALL LETTER A WITH DIAERESIS
    0x00F6, // LATIN SMALL LETTER O WITH DIAERESIS
    0x00E5, // LATIN SMALL LETTER A WITH RING ABOVE
    0x2191, // UPWARDS ARROW
    0x2190, // LEFTWARDS ARROW
    0x2501, // BOX DRAWINGS LIGHT HORIZONTAL
    0x0041, // LATIN CAPITAL LETTER A
    0x0042, // LATIN CAPITAL LETTER B
    0x0043, // LATIN CAPITAL LETTER C
    0x0044, // LATIN CAPITAL LETTER D
    0x0045, // LATIN CAPITAL LETTER E
    0x0046, // LATIN CAPITAL LETTER F
    0x0047, // LATIN CAPITAL LETTER G
    0x0048, // LATIN CAPITAL LETTER H
    0x0049, // LATIN CAPITAL LETTER I
    0x004A, // LATIN CAPITAL LETTER J
    0x004B, // LATIN CAPITAL LETTER K
    0x004C, // LATIN CAPITAL LETTER L
    0x004D, // LATIN CAPITAL LETTER M
    0x004E, // LATIN CAPITAL LETTER N
    0x004F, // LATIN CAPITAL LETTER O
    0x0050, // LATIN CAPITAL LETTER P
    0x0051, // LATIN CAPITAL LETTER Q
    0x0052, // LATIN CAPITAL LETTER R
    0x0053, // LATIN CAPITAL LETTER S
    0x0054, // LATIN CAPITAL LETTER T
    0x0055, // LATIN CAPITAL LETTER U
    0x0056, // LATIN CAPITAL LETTER V
    0x0057, // LATIN CAPITAL LETTER W
    0x0058, // LATIN CAPITAL LETTER X
    0x0059, // LATIN CAPITAL LETTER Y
    0x005A, // LATIN CAPITAL LETTER Z
    0x00C4, // LATIN CAPITAL LETTER A WITH DIAERESIS
    0x00D6, // LATIN CAPITAL LETTER O WITH DIAERESIS
    0x00C5, // LATIN CAPITAL LETTER A WITH RING ABOVE
    0x2592, // MEDIUM SHADE
    0x2592, // MEDIUM SHADE SLASHED (CUS) **
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // ORANGE COLOR SWITCH (CUS)
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // FUNCTION KEY 1 (CUS)
    0xFFFD, // FUNCTION KEY 3 (CUS)
    0xFFFD, // FUNCTION KEY 5 (CUS)
    0xFFFD, // FUNCTION KEY 7 (CUS)
    0xFFFD, // FUNCTION KEY 2 (CUS)
    0xFFFD, // FUNCTION KEY 4 (CUS)
    0xFFFD, // FUNCTION KEY 6 (CUS)
    0xFFFD, // FUNCTION KEY 8 (CUS)
    0x0020, // SPACE
    0x000F, // SHIFT IN
    0xFFFD, // undefined in PETSCII-sv
    0xFFFD, // BLACK COLOR SWITCH (CUS)
    0xFFFD, // CURSOR UP (CUS)
    0xFFFD, // REVERSE VIDEO OFF (CUS)
    0x000C, // FORM FEED
    0xFFFD, // INSERT (CUS)
    0xFFFD, // BROWN COLOR SWITCH (CUS)
    0xFFFD, // LIGHT RED COLOR SWITCH (CUS)
    0xFFFD, // GRAY 1 COLOR SWITCH (CUS)
    0xFFFD, // GRAY 2 COLOR SWITCH (CUS)
    0xFFFD, // LIGHT GREEN COLOR SWITCH (CUS)
    0xFFFD, // LIGHT BLUE COLOR SWITCH (CUS)
    0xFFFD, // GRAY 3 COLOR SWITCH (CUS)
    0xFFFD, // PURPLE COLOR SWITCH (CUS)
    0xFFFD, // CURSOR LEFT (CUS)
    0xFFFD, // YELLOW COLOR SWITCH (CUS)
    0xFFFD, // CYAN COLOR SWITCH (CUS)
    0x0020, // SPACE
    0x258C, // LEFT HALF BLOCK
    0x2584, // LOWER HALF BLOCK
    0x2594, // UPPER ONE EIGHTH BLOCK
    0x2581, // LOWER ONE EIGHTH BLOCK
    0x258F, // LEFT ONE EIGHTH BLOCK
    0x2592, // MEDIUM SHADE
    0x2595, // RIGHT ONE EIGHTH BLOCK
    0x2584, // LOWER HALF BLOCK MEDIUM SHADE (CUS) **
    0x25E4, // BLACK UPPER LEFT TRIANGLE
    0x2590, // RIGHT ONE QUARTER BLOCK (CUS) **
    0x251C, // BOX DRAWINGS LIGHT VERTICAL AND RIGHT
    0x00A0, // BLACK SMALL SQUARE LOWER RIGHT (CUS) **
    0x2514, // BOX DRAWINGS LIGHT UP AND RIGHT
    0x2510, // BOX DRAWINGS LIGHT DOWN AND LEFT
    0x2582, // LOWER ONE QUARTER BLOCK
    0x250C, // BOX DRAWINGS LIGHT DOWN AND RIGHT
    0x2534, // BOX DRAWINGS LIGHT UP AND HORIZONTAL
    0x252C, // BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
    0x2524, // BOX DRAWINGS LIGHT VERTICAL AND LEFT
    0x258E, // LEFT ONE QUARTER BLOCK
    0x258D, // LEFT THREE EIGTHS BLOCK
    0x2590, // RIGHT THREE EIGHTHS BLOCK (CUS) **
    0x2580, // UPPER ONE QUARTER BLOCK (CUS) **
    0x2580, // UPPER THREE EIGHTS BLOCK (CUS) **
    0x2583, // LOWER THREE EIGHTHS BLOCK
    0x2713, // CHECK MARK
    0x00A0, // BLACK SMALL SQUARE LOWER LEFT (CUS) **
    0x00A0, // BLACK SMALL SQUARE UPPER RIGHT (CUS) **
    0x2518, // BOX DRAWINGS LIGHT UP AND LEFT
    0x00A0, // BLACK SMALL SQUARE UPPER LEFT (CUS) **
    0x00A0, // TWO SMALL BLACK SQUARES DIAGONAL LEFT TO RIGHT (CUS) **
    0x2501, // BOX DRAWINGS LIGHT HORIZONTAL
    0x0041, // LATIN CAPITAL LETTER A
    0x0042, // LATIN CAPITAL LETTER B
    0x0043, // LATIN CAPITAL LETTER C
    0x0044, // LATIN CAPITAL LETTER D
    0x0045, // LATIN CAPITAL LETTER E
    0x0046, // LATIN CAPITAL LETTER F
    0x0047, // LATIN CAPITAL LETTER G
    0x0048, // LATIN CAPITAL LETTER H
    0x0049, // LATIN CAPITAL LETTER I
    0x004A, // LATIN CAPITAL LETTER J
    0x004B, // LATIN CAPITAL LETTER K
    0x004C, // LATIN CAPITAL LETTER L
    0x004D, // LATIN CAPITAL LETTER M
    0x004E, // LATIN CAPITAL LETTER N
    0x004F, // LATIN CAPITAL LETTER O
    0x0050, // LATIN CAPITAL LETTER P
    0x0051, // LATIN CAPITAL LETTER Q
    0x0052, // LATIN CAPITAL LETTER R
    0x0053, // LATIN CAPITAL LETTER S
    0x0054, // LATIN CAPITAL LETTER T
    0x0055, // LATIN CAPITAL LETTER U
    0x0056, // LATIN CAPITAL LETTER V
    0x0057, // LATIN CAPITAL LETTER W
    0x0058, // LATIN CAPITAL LETTER X
    0x0059, // LATIN CAPITAL LETTER Y
    0x005A, // LATIN CAPITAL LETTER Z
    0x00C4, // LATIN CAPITAL LETTER A WITH DIAERESIS
    0x00D6, // LATIN CAPITAL LETTER O WITH DIAERESIS
    0x00C5, // LATIN CAPITAL LETTER A WITH RING ABOVE
    0x2592, // MEDIUM SHADE
    0x2592, // MEDIUM SHADE SLASHED (CUS) **
    0x0020, // SPACE
    0x258C, // LEFT HALF BLOCK
    0x2584, // LOWER HALF BLOCK
    0x2594, // UPPER ONE EIGHTH BLOCK
    0x2581, // LOWER ONE EIGHTH BLOCK
    0x258F, // LEFT ONE EIGHTH BLOCK
    0x2592, // MEDIUM SHADE
    0x2595, // RIGHT ONE EIGHTH BLOCK
    0x2584, // LOWER HALF BLOCK MEDIUM SHADE (CUS) **
    0x25E4, // BLACK UPPER LEFT TRIANGLE
    0x2590, // RIGHT ONE QUARTER BLOCK (CUS) **
    0x251C, // BOX DRAWINGS LIGHT VERTICAL AND RIGHT
    0x00A0, // BLACK SMALL SQUARE LOWER RIGHT (CUS) **
    0x2514, // BOX DRAWINGS LIGHT UP AND RIGHT
    0x2510, // BOX DRAWINGS LIGHT DOWN AND LEFT
    0x2582, // LOWER ONE QUARTER BLOCK
    0x250C, // BOX DRAWINGS LIGHT DOWN AND RIGHT
    0x2534, // BOX DRAWINGS LIGHT UP AND HORIZONTAL
    0x252C, // BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
    0x2524, // BOX DRAWINGS LIGHT VERTICAL AND LEFT
    0x258E, // LEFT ONE QUARTER BLOCK
    0x258D, // LEFT THREE EIGTHS BLOCK
    0x2590, // RIGHT THREE EIGHTHS BLOCK (CUS) **
    0x2580, // UPPER ONE QUARTER BLOCK (CUS) **
    0x2580, // UPPER THREE EIGHTS BLOCK (CUS) **
    0x2583, // LOWER THREE EIGHTHS BLOCK
    0x2713, // CHECK MARK
    0x00A0, // BLACK SMALL SQUARE LOWER LEFT (CUS)
    0x00A0, // BLACK SMALL SQUARE UPPER RIGHT (CUS)
    0x2518, // BOX DRAWINGS LIGHT UP AND LEFT
    0x00A0, // BLACK SMALL SQUARE UPPER LEFT (CUS)
    0x2592  // MEDIUM SHADE
};

// Convert a string from Swedish PETSCII to UTF-8
string utf8petscii(char *input, int n, bool esc)
{
    string o;
    unsigned char *p = (unsigned char *) input;

    while (*p && n)
    {
        wchar_t ch = petsciiunicode[*p];
        if (ch <= 0x7F)
        {
            if (esc)
            {
                switch (ch)
                {
                    case '&':
                        o += "&amp;";
                        break;

                    case '<':
                        o += "&lt;";
                        break;

                    case '>':
                        o += "&gt;";
                        break;

                    default:
                        o += char(ch);
                        break;
                }
            }
            else
            {
                o += char(ch);
            }
        }
        else if (ch <= 0x7FF)
        {
            o += char(0xC0 | ((ch >> 6) & 0x1F));
            o += char(0x80 | ( ch       & 0x3F));
        }
        else if (ch <= 0xFFFF)
        {
            o += char(0xE0 | ((ch >> 12) & 0x0F));
            o += char(0x80 | ((ch >>  6) & 0x3F));
            o += char(0x80 | ( ch        & 0x3F));
        }

        n --;
        p ++;
    }
    *p = 0;
    return o;
}

// Convert a string from Swedish PETSCII to a rudimentry ASCII representation
// (kills most special characters and always uses mixed case)
string petscii(char *input, int n, bool esc)
{
    string o;
    unsigned char *p = (unsigned char *) input;

    while (*p && n)
    {
        if      (*p ==  13) {o += '\n';     }
        else if (*p <   32) {o += '_';      }
        else if (*p ==  38 && esc)
                            {o += "&amp;";  }
        else if (*p ==  60 && esc)
                            {o += "&lt;";   }
        else if (*p ==  62 && esc)
                            {o += "&gt;";   }
        else if (*p <   64) {o += *p;       }
        else if (*p ==  91) {o += 'ä';      }
        else if (*p ==  92) {o += 'ö';      }
        else if (*p ==  93) {o += 'å';      }
        else if (*p ==  96) {o += '-';      }
        else if (*p <   96) {o += *p | ' '; }
        else if (*p == 123) {o += 'Ä';      }
        else if (*p == 124) {o += 'Ö';      }
        else if (*p == 125) {o += 'Å';      }
        else if (*p == 126) {o += '¶';      }
        else if (*p <  128) {o += *p - ' '; }
        else if (*p <  160) {o += '_';      }
        else if (*p == 160) {o += ' ';      }
        else if (*p <  192) {o += '+';      }
        else if (*p == 192) {o += '-';      }
        else if (*p == 219) {o += 'Ä';      }
        else if (*p == 220) {o += 'Ö';      }
        else if (*p == 221) {o += 'Å';      }
        else if (*p == 222) {o += '¶';      }
        else if (*p <  224) {o += *p & 0x7f;}
        else if (*p <  255) {o += '+';      }
        else if (*p == 255) {o += '¶';      }
        n --;
        p ++;
    }
    *p = 0;
    return o;
}

// Copy the parameter for value "param" into "dest" (which is at most n
// characters long.
// returns 1 on success, 0 on failure
int query(const char *param, char *dest, int n)
{
    char    *p, *old, *srch, hex[3] = "00";
    int     len, rc, ch;

    // Allow running the program from the command line
    if (!getenv("REQUEST_METHOD"))
    {
        printf("Please enter value for parameter %s:\n", param);
        fflush(stdin);
        fgets(dest, n, stdin);
        dest[n - 1] = 0;
        if (n > 2) dest[strlen(dest) - 1] = 0;
        if (strlen(dest)) return 1;
        else return 0;
    }

    // Get query string
    p = getenv("QUERY_STRING");
    dest[0] = 0;
    if (!p) return 0;
    p = strdup(p);
    if (!p) return 0;
    old = p;

    // Set up
    srch = new char[strlen(param) + 2];
    strcpy(srch, param);
    strcat(srch, "=");

    // Find parameter
    len = strlen(srch);
    p = strtok(p, "&");
    while (p && strncmp(p, srch, len))
    {
        p = strtok(NULL, "&");
    }

    // Save to destination if found
    p += len - 1;
    rc = 0;
    if (p)
    {
        rc = 1;
        p += 1;
        while (*p != 0 && n > 1)
        {
            switch (*p)
            {
                case '%':
                    hex[0] = *(++ p);
                    hex[0] = *(++ p);
                    sscanf(hex, "%d", &ch);
                    *(dest ++) = ch;
                    break;

                case '+':
                    *(dest ++) = ' ';
                    break;

                default:
                    *(dest ++) = *p;
                    break;
            }
            p ++;
            n --;
        }
        // Null terminate
        *dest = 0;
    }

    // Free up space and return
    free(old);
    delete srch;
    return rc;
}

// Display error message
void disperror(const char *text)
{
    puts("Content-Type: text/html;charset=us-ascii\n");
    puts("<html><head><title>Error</title></head><body>");
    puts(text);
    puts("</body></html>");
    exit(0);
}

// Opens a D64 file and does some checking
FILE *opend64(const char *fname, bam_s *bam_p)
{
    FILE *f;

    // Check that it exists
    f = fopen(fname, "r");
    if (!f) disperror("Unable to open specified file");

    // Check size
    fseek(f, 0, SEEK_END);
    if (ftell(f) != 174848) disperror("File size mismatch");

    // Check integrity
    fseek(f, 91392, SEEK_SET); // Start of directory header block
    fread(bam_p, sizeof (bam_s), 1, f);
    if (bam_p->t != 18 || bam_p->s != 1 || bam_p->format != 'A')
        disperror("Disk format does not match 1541");

    return f;
}

#ifdef VIRTUAL
// Trims the file name to something that works with HTTP
char *trim (const char *input)
{
    static char output[16 * 6 + 1];
    int         i;

    strcpy(output, input);
    while (' ' == output[strlen(output) - 1])
        output[strlen(output) - 1] = 0;

    for (i = 0; i <= 17; i ++)
        if (' ' == output[i])
            output[i] = '_';
        else if (0 == output[i])
            return output;
    return output;
}
#endif

// Displays directory of a D64 file
void directory(const char *fname)
{
    FILE        *f;
    dirblock_u  dirblock;
    int         sector, dirnr, i, ftype;
    const char  *filetypes[] = { "DEL", "SEQ", "PRG", "USR", "REL" };
#ifdef VIRTUAL
    char        *p;
#endif
    string      tmp, tmp2, ascii;
    bam_s       bam;

    // Open the D64 file
    f = opend64(fname, &bam);

    // Print HTML code
    puts("Content-type: text/html;charset=utf-8\n");
    puts("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">");
    puts("<html><head><title>");
    printf("Contents of %s\n", fname);
    puts("</title>");
    puts("<meta name=\"robots\" content=\"index,nofollow\">");
    puts("<meta http-equiv=\"Content-Type\" content=\"text/html;"
         "charset=utf-8\">\n");
    puts("</head>");
    puts("<body bgcolor=\"#ffffff\" link=\"#1e90ff\" vlink=\"#000080\">");
    printf("<h1 align=center>%s</h1>\n", fname);
    puts("<table align=center border=0>");
    tmp = utf8petscii(bam.diskname, 16, true);
    tmp2= utf8petscii(bam.id,       5,  true);
    printf(" <tr><th><th align=left bgcolor=\"#cccccc\">%s"
           "<th align=left bgcolor=\"#cccccc\">%s\n",
           tmp.c_str(), tmp2.c_str());

    // List files
    sector = 1;
    dirnr = 0;
    while (sector)
    {
        // Load directory block
        fseek(f, 91392 + sector * 256, SEEK_SET);
        fread(&dirblock, sizeof (dirblock), 1, f);

        // Each block contains eight directory entries
        for (i = 0; i < 8; i ++)
        {
            ftype = dirblock.file[i].filetype;
            if (ftype != 0 && (ftype & 0xf) < 5)
            {
                // Displayable file
                printf(" <tr><td align=right>%d<td align=left>",
                       (int) dirblock.file[i].length[0] +
                       (int) dirblock.file[i].length[1] * 256);

                tmp = utf8petscii(dirblock.file[i].name, 16, true);

                if (((ftype & 0xf) == 1 || (ftype & 0xf) == 2) &&
                    (ftype & 0x80) == 0x80)
                {
                    // Properly closed SEQ or PRG

                    dirnr = sector * 8 + i;
#ifdef VIRTUAL
                    ascii = utf8petscii(dirblock.file[i].name, 16, true);
                    p = trim(ascii.c_str());
                    printf("<a href=\"%d/r/%s.%c\">%s</a>"
                           "<td align=left>"
                           "<a href=\"%d/%c/%s\">%s%s</a>",
                           dirnr, p, tolower(filetypes[ftype & 0xf][0]),
                           tmp.c_str(),
                           dirnr, tolower(filetypes[ftype & 0xf][0]),
                           p, filetypes[ftype & 0xf],
                           ((ftype & 0xC0) == 0xC0 ? "<" : ""));
                    if ((ftype & 0xf) == 1)
                    {
                        // Provide UTF-8 alternative for SEQ
                        printf(" <a href=\"%d/u/%s\">(utf-8)</a>",
                               dirnr, p);
                    }
                    putchar('\n');
#else
                    printf("<a href=\"" SELF "?path=%s&amp;action=extract"
                           "&amp;filenum=%d&amp;type=r\">%s</a>"
                           "<td align=left>"
                           "<a href=\"" SELF "?path=%s&amp;action=extract"
                           "&amp;filenum=%d&amp;type=%c\">%s%s</a>",
                           fname, dirnr, tmp.c_str(),
                           fname, dirnr, tolower(filetypes[ftype & 0xf][0]),
                           filetypes[ftype & 0xf],
                           ((ftype & 0xC0) == 0xC0 ? "<" : ""));
                    if ((ftype & 0xf) == 1)
                    {
                        // Provide UTF-8 alternative for SEQ
                        printf(" <a href="\"" SELF "?path=%s&amp;"
                               "action=extract&amp;filenum=%d&amp;"
                               "type=u\">(utf-8)</a>",
                               fname, dirnr);
                    }
                    putchar('\n');
#endif
                }
                else
                {
                    // All other files
                    printf("%s<td>%s%s%s\n",
                           tmp.c_str(),
                           ((ftype & 0x80) == 0x80 ? "" : "*"),
                           filetypes[ftype & 0xf],
                           ((ftype & 0xC0) == 0xC0 ? "<" : ""));
                }

                // GEOS dated files
                if (dirblock.file[i].month >= 1  &&
                    dirblock.file[i].month <= 12 &&
                    dirblock.file[i].day   >= 1  &&
                    dirblock.file[i].day   <= 31)
                {
                    printf("  <td>%02d-%02d-%02d %02d:%02d\n",
                           dirblock.file[i].year, dirblock.file[i].month,
                           dirblock.file[i].day,  dirblock.file[i].hour,
                           dirblock.file[i].minute);

                }
            }
        }

        // Follow the directory links, making sure we stay on the right
        // track.
        sector = dirblock.link.s;
        if (dirblock.link.t != 18) sector = 0;
    }

    // Finish up
    fclose(f);

    puts("</table>");
    puts("You can download files by their filenames, or see them "
         "in your browser by their filetypes."
         "Select the (utf-8) link to get a UTF-8 representation "
         "of sequential (text) files.");
    puts("<hr noshade>");
#ifdef VIRTUAL
    puts("<a href=\"../\">Return to the index</a>");
#else
    puts("<a href=\"./\">Return to the index</a>");
#endif
    puts("</body></html>");
}

// Convert combination of track and sector number to a linear block number
// which is easier to use in D64 files (multiply by 256 for position)
int ts2block(int track, int sector)
{
    if (track < 18) return (track -  1) * 21 + sector;
    if (track < 25) return (track - 18) * 19 + 17 * 21 + sector;
    if (track < 31) return (track - 25) * 18 + 17 * 21 + 19 * 7 + sector;
    return (track - 21) * 17 + 17 * 21 + 19 * 7 + 18 * 6 + sector;
}

// Extract a file in any of three ways:
// (r)aw, detokenize (p)rg or decode petscii (s)eq/(u)tf8 seq
void extract(const char *fname, int filenum, const char action)
{
    FILE        *f, *out;
    bam_s       bam;
    char        tmpname[256], sector[256];
    int         fh, t, s;
    dirent_s    dirent;
    string      tmp;
    int         blocks;

    // Check parameters for sanity
    if (action != 'r' && action != 's' && action != 'p' && action != 'u')
    {
        disperror("I don't know how to extract that type");
    }

    if (filenum < 0 || filenum > 152)
    {
        disperror("The specified file number is invalid");
    }

    // Open D64 file
    f = opend64(fname, &bam);

    // We have a 'out' handle so that we can write to other things than
    // stdout if needed
    out = stdout;

    // Load directory entry
    fseek(f, 91392 + filenum * 32, SEEK_SET);
    fread(&dirent, 1, sizeof (dirent), f);

    // Check starting position for sanity
    t = dirent.t;
    s = dirent.s;
    if (t == 18 || t > 35 || s > 21)
    {
        disperror("Illegal directory entry");
    }

    // Output headers depending on the format we want
    switch (action)
    {
        case 'r':
            puts("Content-type: application/x-c64-binary\n");
            break;

        case 's':
            puts("Content-type: text/plain;charset=iso-8859-1\n");
            break;

        case 'u':
            puts("Content-type: text/plain;charset=utf-8\n");
            break;

        case 'p':
            // Since bastext doesn't grok data on stdin, we need to
            // create a temporary file to hold the binary data, and
            strcpy(tmpname, TMPNAME);
            fh = mkstemp(tmpname);
            if (-1 == fh)
            {
                disperror("Unable to create temporary file");
            }

            // Reassing to a FILE*
            out = fdopen(fh, "w");
            puts("Content-type: text/plain;charset=iso-8859-1\n");
            break;
    }

    // Make sure loops don't kill us
    blocks = MAXSIZE;

    // End of file marker is track == 0
    while (t && blocks)
    {
        // Read requested sector
        fseek(f, ts2block(t, s) * 256, SEEK_SET);
        fread(sector, 1, 256, f);

        // Read links
        t = (unsigned char) sector[0];
        s = (unsigned char) sector[1];

        if ('s' == action)
        {
            // Convert from PETSCII to ASCII
            tmp = petscii(&sector[2], t ? 254 : (s - 2), false);
            fputs(tmp.c_str(), out);
        }
        else if ('u' == action)
        {
            // Convert from PETSCII to UTF-8
            tmp = utf8petscii(&sector[2], t ? 254 : (s - 2), false);
            fputs(tmp.c_str(), out);
        }
        else
        {
            // Just output it as raw data
            fwrite(&sector[2], 1, t ? 254 : (s - 2), out);
        }

        // Check for error conditions
        if (t == 18 || t > 35 || s > 21)
        {
            t = 0;
        }

        blocks --;
    }

    // We're done with the D64 file
    fclose(f);

    // If we are printing a BASIC program, we open a pipe with our
    // PRG->text conversion program running on the temporary file we
    // just created
    if (action == 'p')
    {
        FILE    *pipe;
        char    cmd[256], buf[1024];
        int     count;

        fclose(out);

        sprintf(cmd, BASTEXT, tmpname);
        pipe = popen(cmd, "r");
        while ((count = fread(buf, 1, 1024, pipe)) != 0)
        {
            fwrite(buf, count, 1, stdout);
        }
        fclose(pipe);

        remove(tmpname);
    }
}

// Decide what the heck we want done
int main(void)
{
    char path[64], action[64], filenum[4], type[2];

    // Retrieve parameters
    if (!query("path", path, 64))
    {
        disperror("Required parameter \"path\" not supplied.");
    }

    if (strchr(path, '/'))
    {
        disperror("The path may not contain \"/\" characters.");
    }

    if (!query("action", action, 64))
    {
        disperror("Required parameter \"action\" not supplied.");
    }

    if (0 == strcmp(action, "list"))
    {
        directory(path);
    }
    else if (0 == strcmp(action, "extract"))
    {
        int num;
        
        if (!query("filenum", filenum, 4))
        {
            disperror("Required parameter \"filenum\" not supplied.");
        }

        if (!query("type", type, 2))
        {
            disperror("Required parameter \"type\" not supplied.");
        }

        num = atoi(filenum);
        extract(path, num, type[0]);
    }
    else
    {
        disperror("Illegal action specified");
    }

    return 0;
}
