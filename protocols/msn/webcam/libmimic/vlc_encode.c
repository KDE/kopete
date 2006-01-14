/* Copyright (C) 2005  Ole André Vadla Ravnås <oleavr@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include "mimic-private.h"

extern guchar _col_zag[64];
extern VlcSymbol _vlc_alphabet[16][128];

/*
 * _vlc_encode_block
 *
 * Serialize an 8x8 block using variable length coding.
 */
void _vlc_encode_block(MimCtx *ctx, const gint *block, gint num_coeffs)
{
    gint i, num_zeroes;

    /* The DC value is written out as is. */
    _write_bits(ctx, block[0], 8);

    /* Number of zeroes prefixing the next non-zero value. */
    num_zeroes = 0;

    for (i = 1; i < num_coeffs && num_zeroes <= 14; i++) {

        /* Fetch AC coefficients from block in zig-zag order. */
        gint value = block[_col_zag[i]];

        if (value != 0) {
            VlcSymbol sym;

            /* Clip input values to [-128, +128]. */
            if (value < -128)
                value = -128;
            else if (value > 128)
                value = 128;

            /* Look up symbol for the current non-zero value. */
            sym = _vlc_alphabet[num_zeroes][abs(value) - 1];

            /* No symbol? very rare... */
            if (sym.length1 <= 0)
                break;

            /* The symbols for negative values are the same as for positives, minus one. */
            if (value < 0) {
                if (sym.length2 > 0)
                    sym.part2 -= 1;
                else
                    sym.part1 -= 1;
            }

            /* Write out the full symbol. */
            _write_bits(ctx, sym.part1, sym.length1);
            if (sym.length2 > 0)
                _write_bits(ctx, sym.part2, sym.length2);

            /* Start counting zeroes again. */
            num_zeroes = 0;
        } else {
            num_zeroes++;
        }
    }

    /* Write out EOB if necessary. */
    if (num_zeroes > 0)
        _write_bits(ctx, 0xA, 4);
}

