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

#include "mimic-private.h"

/*
 * _read_bits
 *
 * Internal helper-function used to read num_bits
 * from stream.
 */
guint32 _read_bits(MimCtx *ctx, gint num_bits)
{
    guint32 bits;
    
    if (ctx->cur_chunk_len >= 16) {
        guchar *input_buf = (guchar *) ctx->data_buffer + ctx->data_index;

        if (!ctx->read_odd) {
            ctx->read_odd = TRUE;

            ctx->cur_chunk = (input_buf[3] << 24) |
                             (input_buf[2] << 16) |
                             (input_buf[1] <<  8) |
                              input_buf[0];

        } else {
            ctx->read_odd = FALSE;
            
            ctx->cur_chunk = (input_buf[1] << 24) |
                             (input_buf[0] << 16) |
                             (input_buf[7] <<  8) |
                              input_buf[6];

            ctx->data_index += 4;
        }

        ctx->cur_chunk_len -= 16;
    }

    bits = (ctx->cur_chunk << ctx->cur_chunk_len) >> (32 - num_bits);
    ctx->cur_chunk_len += num_bits;

    return bits;
}

/*
 * _write_bits
 *
 * Internal helper-function used to write "length"
 * bits of "bits" to stream.
 */
void _write_bits(MimCtx *ctx, guint32 bits, gint length)
{
    /* Left-align the bit string within its 32-bit container. */
    bits <<= (32 - length);

    /* Append the bit string (one or more of the trailing bits might not fit, but that's ok). */
    ctx->cur_chunk |= bits >> ctx->cur_chunk_len;
    ctx->cur_chunk_len += length;

    /* Is it full? */
	if (ctx->cur_chunk_len >= 32) {

        /* Add the full 32-bit chunk to the stream and update counter. */
        ctx->chunk_ptr[0] = GUINT32_TO_LE(ctx->cur_chunk);
        ctx->chunk_ptr++;
        ctx->cur_chunk_len -= 32;

        /* Add any trailing bits that didn't fit. */
        ctx->cur_chunk = bits << (length - ctx->cur_chunk_len);
    }
}

