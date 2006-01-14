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

#include <string.h>
#include "mimic-private.h"

extern guchar _col_zag[64];

/*
 * _vlc_decode_block
 *
 * De-serialize (reconstruct) a variable length coded 8x8 block.
 */
gboolean _vlc_decode_block(MimCtx *ctx, gint *block, gint num_coeffs)
{
    guint pos;

    memset(block, 0, 64 * sizeof(gint));

    /* The DC-value is read in as is. */
    block[0] = _read_bits(ctx, 8);

    for (pos = 1; pos < num_coeffs; pos++) {
        
        guint prev_data_index, prev_cur_chunk_len, prev_chunk;
        guint value, num_bits;
        gboolean prev_read_odd, found_magic;
        
        /* Save context. */
        prev_data_index = ctx->data_index;
        prev_cur_chunk_len = ctx->cur_chunk_len;
        prev_chunk = ctx->cur_chunk;
        prev_read_odd = ctx->read_odd;

        /* Grab 16 bits. */
        value = _read_bits(ctx, 16) << 16;

        /* Restore context. */
        ctx->data_index = prev_data_index;
        ctx->cur_chunk_len = prev_cur_chunk_len;
        ctx->cur_chunk = prev_chunk;
        ctx->read_odd = prev_read_odd;

        /* Analyze and determine number of bits to read initially. */
        num_bits = 3;
        if ((value >> 30) == 0 || (value >> 30) == 1) {
            num_bits = 2;
        } else if ((value & 0xE0000000) != 0x80000000) {
            guint nibble = value >> 28;

            if (nibble == 11 || nibble == 12) {
                num_bits = 4;
            } else if (nibble == 10) {
                _read_bits(ctx, 4);

                return TRUE;
            } else {
                if (((value << 2) & 0x8000000) == 0)
                    num_bits = 2;

                num_bits += 2;
            }
        }

        /* Read that number of bits. */
        value = _read_bits(ctx, num_bits);

        /*
         * Look up the current value against the magic ones,
         * and continue extending it bit by bit from the input
         * stream until the magic value is found or we have
         * read 32 bits (in which case we give up).
         */
        found_magic = FALSE;
        while (!found_magic) {
            VlcMagic *magic;

            if (num_bits > 32)
                return FALSE;

            magic = _find_magic(value);

            if (magic != NULL) {
                pos += magic->pos_add;
                num_bits = magic->num_bits;

                found_magic = TRUE;
            } else {
                value <<= 1;
                value |= _read_bits(ctx, 1);

                num_bits++;
            }
        }

        /* Read the number of bits given by magic value entry. */
        value = _read_bits(ctx, num_bits);
        
        /* Gotcha! :-) */
        block[_col_zag[pos]] = ctx->vlcdec_lookup[(num_bits * 255) + value];
    }

    return TRUE;
}

