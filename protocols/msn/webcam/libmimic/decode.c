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


static gboolean decode(MimCtx *ctx, gboolean is_pframe);

/**
 * Decode a MIMIC-encoded frame into RGB data.
 *
 * @param ctx the mimic context
 * @param input_buffer buffer containing the MIMIC-encoded frame to decode
 * @param output_buffer buffer that will receive the decoded frame in RGB 24-bpp packed pixel top-down format
 *                      (use #mimic_get_property to determine the required buffer size, as well as frame width and height)
 * @returns #TRUE on success
 */
gboolean mimic_decode_frame(MimCtx *ctx,
                            const guchar *input_buffer,
                            guchar *output_buffer)
{
    gboolean result, is_pframe;
    guchar *input_y, *input_cr, *input_cb;
    gint width, height;
    
    /*
     * Some sanity checks.
     */
    if (ctx == NULL || input_buffer == NULL || output_buffer == NULL)
    {
        return FALSE;
    }

    if (!ctx->decoder_initialized)
    {
        return FALSE;
    }
    
    /*
     * Get frame dimensions.
     */
    width  = GUINT16_FROM_LE(*((guint16 *) (input_buffer + 4)));
    height = GUINT16_FROM_LE(*((guint16 *) (input_buffer + 6)));

    /*
     * Resolution changing is not supported.
     */
    if (width  != ctx->frame_width ||
        height != ctx->frame_height)
    {
        return FALSE;
    }
    
    /*
     * Increment frame counter.
     */
    ctx->frame_num++;

    /*
     * Initialize state.
     */
    ctx->quality = GUINT16_FROM_LE(*((guint16 *) (input_buffer + 2)));
    is_pframe = GUINT32_FROM_LE(*((guint32 *) (input_buffer + 12)));
    ctx->num_coeffs = input_buffer[16];
    
    ctx->data_buffer = (gchar *) (input_buffer + 20);
    ctx->data_index = 0;
    ctx->cur_chunk_len = 16;
    ctx->read_odd = FALSE;
    
    /*
     * Decode frame.
     */
    if (!(is_pframe && ctx->prev_frame_buf == NULL))
        result = decode(ctx, is_pframe);
    else
    {
        result = FALSE;
    }

    /*
     * Perform YUV 420 to RGB conversion.
     */
    input_y = ctx->cur_frame_buf;
    input_cr = ctx->cur_frame_buf + ctx->y_size;
    input_cb = ctx->cur_frame_buf + ctx->y_size + ctx->crcb_size;

    _yuv_to_rgb(input_y,
                input_cb,
                input_cr,
                output_buffer,
                ctx->frame_width,
                ctx->frame_height);

    return result;
}

/*
 * decode_main
 *
 * Main decoding loop.
 */
static gboolean decode(MimCtx *ctx, gboolean is_pframe)
{
    gint y, x, i, j, chrom_ch, *bptr, base_offset, offset;
    gint dct_block[64];
    guchar *src, *dst, *p;
    guint32 bit;
    
    /*
     * Clear Cr and Cb planes.
     */
    p = ctx->cur_frame_buf + ctx->y_size;
    memset(p, 128, 2 * ctx->crcb_size);

    /*
     * Decode Y plane.
     */
    for (y = 0; y < ctx->num_vblocks_y; y++) {

        base_offset = ctx->y_stride * 8 * y;

        src = ctx->prev_frame_buf + base_offset;
        dst = ctx->cur_frame_buf  + base_offset;

        for (x = 0; x < ctx->num_hblocks_y; x++) {

            /* Check for a change condition in the current block. */

            if (is_pframe)
                bit = _read_bits(ctx, 1);
            else
                bit = 0;

            if (bit == 0) {

                /* Yes: Is the new content the same as it was in one of
                 * the 15 last frames preceding the previous? */
                
                if (is_pframe)
                    bit = _read_bits(ctx, 1);

                if (bit == 0) {

                    /* No: decode it. */
                    
                    if (_vlc_decode_block(ctx, dct_block, ctx->num_coeffs) == FALSE) {

                        return FALSE;
                    }

                    _idct_dequant_block(ctx, dct_block, 0);

                    bptr = dct_block;
                    for (i = 0; i < 8; i++) {
                        offset = ctx->y_stride * i;

                        for (j = 0; j < 8; j++) {
                            guint v;
                            
                            if (bptr[j] <= 255)
                                v = (bptr[j] >= 0) ? bptr[j] : 0;
                            else
                                v = 255;
                            
                            *(dst + offset + j) = v;
                        }

                        bptr += 8;
                    }
                } else {
                    guint32 backref;
                    
                    /* Yes: read the backreference (4 bits) and copy. */

                    backref = _read_bits(ctx, 4);

                    p = ctx->buf_ptrs[(ctx->ptr_index + backref) % 16];
                    p += base_offset + (x * 8);

                    for (i = 0; i < 8; i++) {
                        offset = ctx->y_stride * i;

                        memcpy(dst + offset, p + offset, 8);
                    }
                }
            } else {
                
                /* No change no worries: just copy from the previous frame. */

                for (i = 0; i < 8; i++) {
                    offset = ctx->y_stride * i;

                    memcpy(dst + offset, src + offset, 8);
                }
            }

            src += 8;
            dst += 8;
        }
    }

    /*
     * Decode Cr and Cb planes.
     */
    for (chrom_ch = 0; chrom_ch < 2; chrom_ch++) {

        base_offset = ctx->y_size + (ctx->crcb_size * chrom_ch);

        for (y = 0; y < ctx->num_vblocks_cbcr; y++) {
            guint num_rows = 8;
            
            /* The last row of blocks in chrominance for 160x120 resolution
             * is half the normal height and must be accounted for. */
            if (y + 1 == ctx->num_vblocks_cbcr && ctx->frame_height % 16 != 0)
                num_rows = 4;

            offset = base_offset + (ctx->crcb_stride * 8 * y);

            src = ctx->prev_frame_buf + offset;
            dst = ctx->cur_frame_buf  + offset;
            
            for (x = 0; x < ctx->num_hblocks_cbcr; x++) {
                
                /* Check for a change condition in the current block. */
                
                if (is_pframe)
                    bit = _read_bits(ctx, 1);
                else
                    bit = 1;

                if (bit == 1) {
                    
                    /* Yes: decode it. */

                    if (_vlc_decode_block(ctx, dct_block, ctx->num_coeffs) == FALSE) {

                         /* Corrupted frame: clear Cr and Cb planes and return. */
                        p = ctx->cur_frame_buf + ctx->y_size;
                        memset(p, 128, ctx->crcb_size * 2);

                        return FALSE;
                    }

                    _idct_dequant_block(ctx, dct_block, 1);

                    for (i = 0; i < num_rows; i++) {
                        p = dst + (ctx->crcb_stride * i);

                        for (j = 0; j < 8; j++)
                            p[j] = dct_block[(i * 8) + j];
                    }

                } else {

                    /* No change no worries: just copy from the previous frame. */
                    
                    for (i = 0; i < num_rows; i++) {
                        offset = ctx->crcb_stride * i;
                        
                        memcpy(dst + offset, src + offset, 8);
                    }
                }

                src += 8;
                dst += 8;
            }
        }
    }

    /*
     * Make a copy of the current frame and store in
     * the circular pointer list of 16 entries.
     */
    ctx->prev_frame_buf = ctx->buf_ptrs[ctx->ptr_index];
    memcpy(ctx->prev_frame_buf, ctx->cur_frame_buf,
           ctx->y_size + (ctx->crcb_size * 2));
    
    if (--ctx->ptr_index < 0)
        ctx->ptr_index = 15;
    
    /*
     * Perform deblocking on all planes.
     */
    _deblock(ctx->cur_frame_buf,
             ctx->y_stride, ctx->y_row_count);
    
    _deblock(ctx->cur_frame_buf + ctx->y_size,
             ctx->crcb_stride, ctx->crcb_row_count);
    
    _deblock(ctx->cur_frame_buf + ctx->y_size + ctx->crcb_size,
             ctx->crcb_stride, ctx->crcb_row_count);

    return TRUE;
}

