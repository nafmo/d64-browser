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

#pragma pack(1)

// Adapt these defines to fit your system
#define BASTEXT "/home/peter/bin/bastext -ia %s"
#define TMPNAME "/tmp/d64-XXXXXX"

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
    puts("Content-Type: text/html\n");
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

// Displays directory of a D64 file
void directory(const char *fname)
{
    FILE        *f;
    dirblock_u  dirblock;
    int         sector, dirnr, i, ftype;
    const char  *filetypes[] = { "DEL", "SEQ", "PRG", "USR", "REL" };
    string      tmp, tmp2;
    bam_s       bam;

    // Open the D64 file
    f = opend64(fname, &bam);

    // Print HTML code
    puts("Content-type: text/html\n");
    puts("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">");
    puts("<html><head><title>");
    printf("Contents of %s\n", fname);
    puts("</title></head>");
    puts("<body bgcolor=\"#ffffff\" link=\"#1e90ff\" vlink=\"#000080\">");
    puts("<table align=center border=0>");
    tmp = petscii(bam.diskname, 16, true);
    tmp2= petscii(bam.id,       5,  true);
    printf(" <tr><th><th align=left bgcolor=\"#cccccc\">%s"
           "<th align=left bgcolor=\"#cccccc\">%s",
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

                tmp = petscii(dirblock.file[i].name, 16, true);

                if (((ftype & 0xf) == 1 || (ftype & 0xf) == 2) &&
                    (ftype & 0x80) == 0x80)
                {
                    // Properly closed SEQ or PRG

                    dirnr = sector * 8 + i;
                    printf("<a href=\"d64.cgi?path=%s&amp;action=extract"
                           "&amp;filenum=%d&amp;type=r\">%s</a>"
                           "<td align=left>"
                           "<a href=\"d64.cgi?path=%s&amp;action=extract"
                           "&amp;filenum=%d&amp;type=%c\">%s%s</a>\n",
                           fname, dirnr, tmp.c_str(),
                           fname, dirnr, tolower(filetypes[ftype & 0xf][0]),
                           filetypes[ftype & 0xf],
                           ((ftype & 0xC0) == 0xC0 ? "<" : ""));
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
// (r)aw, detokenize (p)rg or decode petscii (s)eq
void extract(const char *fname, int filenum, const char action)
{
    FILE        *f, *out;
    bam_s       bam;
    char        tmpname[256], sector[256];
    int         fh, t, s;
    dirent_s    dirent;
    string      tmp;

    // Check parameters for sanity
    if (action != 'r' && action != 's' && action != 'p')
    {
        printf("ACTION=%c(%d)\n", action, (int) action);
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
            puts("Content-type: text/plain\n");
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
            out = fdopen(fh, "w");
            puts("Content-type: text/plain\n");
            break;
    }

    // End of file marker is track == 0
    while (t)
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
            tmp = petscii(&sector[2], t ? 254 : s, false);
            fputs(tmp.c_str(), out);
        }
        else
        {
            // Just output it as raw data
            fwrite(&sector[2], 1, t ? 254 : s, out);
        }

        // Check for error conditions
        if (t == 18 || t > 35 || s > 21)
        {
            t = 0;
        }
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
