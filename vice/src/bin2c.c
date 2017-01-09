/*
 * bin2c.c - binary to c array converter.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>


/** \brief  Generate a C file with "\a array = { data }" statement
 *
 * \param[in]   in_filename     data file name
 * \param[in]   out_filename    C source file name
 * \param[in]   array           array name to use
 *
 * FIXME:   multi dimensional arrays aren't handled properly. For example,
 *          suppose \a array is "foo[3][4]", the compiler will complain about
 *          missing braces around initializers (sub-arrays). The C standard
 *          guarantees that a 3*4 doesn't contain padding, so this code would
 *          generate a 3*4 = 12 bytes array. But don't enable too many warnings
 *          (or -Werror) when compiling!
 *          So this needs fixing (BW)
 */
void generate_c_file(const char *in_filename, const char *out_filename,
                     const char *array)
{
    FILE *infile, *outfile;
    int amount, start, i;
    unsigned char buffer[8];

    infile = fopen(in_filename, "rb");
    if (infile == NULL) {
        printf("cannot open %s for reading\n", in_filename);
        return;
    }

    outfile = fopen(out_filename, "wb");
    if (outfile == NULL) {
        printf("cannot open %s for writing\n", out_filename);
        fclose(infile);
        return;
    }

    fprintf(outfile, "/* Autogenerated file, DO NOT EDIT !!! */\n\n");
    fprintf(outfile, "%s = {\n", array);

    start = 1;
    while (!feof(infile))
    {
        amount = fread(buffer, 1, 8, infile);
        if (start == 0) {
            fprintf(outfile, ",\n");
        } else {
            start = 0;
        }
        if (amount >= 1) {
            fprintf(outfile, "    0x%02X", buffer[0]);
        }
        for (i = 2; i < 9; i++)
        {
            if (amount >= i) {
                fprintf(outfile, ", 0x%02X", buffer[i - 1]);
            }
        }
    }
    fprintf(outfile, "\n};\n");

    fclose(infile);
    fclose(outfile);
    return;
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        printf("too few arguments\n");
        exit(1);
    }

    generate_c_file(argv[1], argv[2], argv[3]);

    return 0;
}
