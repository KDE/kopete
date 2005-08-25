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
#include <string.h>
#include <math.h>
#include "mimic-private.h"

#define LUMINANCE_THRESHOLD   32.0f
#define CHROMINANCE_THRESHOLD 36.0f

static void encode_main(MimCtx *ctx, guchar *out_buf, gboolean is_pframe);

/**
 * Encode a MIMIC-encoded frame from RGB data.
 *
 * @param ctx the mimic context
 * @param input_buffer buffer containing pixeldata in RGB 24-bpp packed pixel top-down format
 * @param output_buffer buffer that will receive the MIMIC-encoded frame
 *                      (use #mimic_get_property to determine the required buffer size)
 * @param output_length pointer to an integer that receives the length of the encoded data
 *                      written to output_buffer
 * @param make_keyframe whether the encoder should make this frame a keyframe
 * @returns #TRUE on success
 */
gboolean mimic_encode_frame(MimCtx *ctx,
                            const guchar *input_buffer,
                            guchar *output_buffer,
                            gint *output_length,
                            gboolean make_keyframe)
{
    guchar *output_y, *output_cb, *output_cr;
    
    /*
     * Some sanity checks.
     */
    if (ctx == NULL || input_buffer == NULL ||
        output_buffer == NULL || output_length == NULL)
    {
        return FALSE;
    }

    if (!ctx->encoder_initialized)
        return FALSE;
    
    /*
     * Initialize state.
     */
    ctx->chunk_ptr = (guint32 *) (output_buffer + 20);
    ctx->cur_chunk = 0;
    ctx->cur_chunk_len = 0;
    
    if (ctx->frame_num == 0)
        make_keyframe = TRUE;
    
    /*
     * Write header.
     */
    memset(output_buffer, 0, 20);
    *((guint16 *) (output_buffer +  0)) = GUINT16_TO_LE(256);
    *((guint16 *) (output_buffer +  2)) = GUINT16_TO_LE(ctx->quality);
    *((guint16 *) (output_buffer +  4)) = GUINT16_TO_LE(ctx->frame_width);
    *((guint16 *) (output_buffer +  6)) = GUINT16_TO_LE(ctx->frame_height);
    *((guint32 *) (output_buffer + 12)) = GUINT32_TO_LE((make_keyframe == 0));
    *(output_buffer + 16) = ctx->num_coeffs;
    *(output_buffer + 17) = 0;
    
    /*
     * Perform RGB to YUV 420 conversion.
     */
    output_y  = ctx->cur_frame_buf;
    output_cr = ctx->cur_frame_buf + ctx->y_size;
    output_cb = ctx->cur_frame_buf + ctx->y_size + ctx->crcb_size;

    _rgb_to_yuv(input_buffer,
                output_y,
                output_cb,
                output_cr,
                ctx->frame_width,
                ctx->frame_height);

    /*
     * Encode frame.
     */
    encode_main(ctx, output_buffer, (make_keyframe == FALSE));

    /*
     * Write out any pending bits to stream by zero-padding with 32 bits.
     */
    _write_bits(ctx, 0, 32);

    /*
     * Calculate bytes written.
     */
    *output_length = (guchar *) ctx->chunk_ptr - output_buffer;
    
    /*
     * Increment frame counter.
     */
    ctx->frame_num++;
    
    return TRUE;
}

static gdouble compare_blocks(const guchar *p1,
                              const guchar *p2,
                              gint stride,
                              gint row_count,
                              gboolean is_chrom);

/*
 * encode_main
 *
 * Main encoding loop.
 */
static void encode_main(MimCtx *ctx, guchar *out_buf, gboolean is_pframe)
{
    gint x, y, i, offset, chrom_ch;
    gint dct_block[64];
    guchar *src, *dst, *p1, *p2;
    gdouble match;
    gboolean encoded;

    /*
     * Round down small differences in luminance channel.
     */
    if (is_pframe) {
        
        p1 = ctx->cur_frame_buf;
        p2 = ctx->prev_frame_buf;

        for (i = 0; i < ctx->y_size; i++) {

            if (abs(p2[0] - p1[0]) < 7)
                p1[0] = p2[0];

            p1++;
            p2++;
        }
    }

    /*
     * Encode Y plane.
     */
    for (y = 0; y < ctx->num_vblocks_y; y++) {

        for (x = 0; x < ctx->num_hblocks_y; x++) {

            /* Calculate final offset into buffer. */
            offset = (ctx->y_stride * 8 * y) + (x * 8);

            src = NULL;
            encoded = FALSE;

            if (is_pframe) {

                /* Is the current block similar enough to what it was in the previous frame? */

                match = compare_blocks(ctx->cur_frame_buf + offset,
                                       ctx->prev_frame_buf + offset,
                                       ctx->y_stride, 8,
                                       FALSE);

                if (match > LUMINANCE_THRESHOLD) {

                    /* Yes: write out '1' to indicate a no-change condition. */
                    
                    _write_bits(ctx, 1, 1);

                    src = ctx->prev_frame_buf + offset;
                    encoded = TRUE;

                } else {

                    /* No: Is the current block similar enough to what it was in one
                     *     of the (up to) 15 last frames preceding the previous? */

                    gint best_index = 0;
                    gdouble best_match = 0.0;

                    gint num_backrefs = ctx->frame_num - 1;
                    if (num_backrefs > 15)
                        num_backrefs = 15;

                    for (i = 1; i <= num_backrefs; i++) {
                        
                        match = compare_blocks(ctx->buf_ptrs[(ctx->ptr_index + i) % 16] + offset,
                                               ctx->cur_frame_buf + offset,
                                               ctx->y_stride, 8,
                                               FALSE);

                        if (match > LUMINANCE_THRESHOLD && match > best_match) {
                            best_index = i;
                            best_match = match;
                        }

                    }

                    if (best_index != 0) {

                        /* Yes: write out '01' to indicate a "change but like previous"-condition,
                         * followed by 4 bits containing the back-reference. */
                        _write_bits(ctx, 0, 1);
                        _write_bits(ctx, 1, 1);
                        _write_bits(ctx, best_index, 4);

                        src = ctx->buf_ptrs[(ctx->ptr_index + best_index) % 16] + offset;
                        encoded = TRUE;

                    }
                }
            }

            if (!encoded) {

                /* Keyframe or in any case no? ;-) Well, encode it then. */

                if (is_pframe) {
                    _write_bits(ctx, 0, 1);
                    _write_bits(ctx, 0, 1);
                }

                _fdct_quant_block(ctx,
                                  dct_block,
                                  ctx->cur_frame_buf + offset,
                                  ctx->y_stride,
                                  FALSE,
                                  ctx->num_coeffs);

                _vlc_encode_block(ctx,
                                  dct_block,
                                  ctx->num_coeffs);

            }

            /* And if there was some kind of no-change condition,
             * we want to copy the previous block. */
            if (src != NULL) {
                
                dst = ctx->cur_frame_buf + offset;
                for (i = 0; i < 8; i++) {

                    memcpy(dst, src, 8);

                    src += ctx->y_stride;
                    dst += ctx->y_stride;
                }
                
            }

        }
        
    }

    /*
     * Encode Cr and Cb planes.
     */
    for (chrom_ch = 0; chrom_ch < 2; chrom_ch++) {

        /* Calculate base offset into buffer. */
        gint base_offset = ctx->y_size + (ctx->crcb_size * chrom_ch);

        for (y = 0; y < ctx->num_vblocks_cbcr; y++) {
            guchar tmp_block[64];
            guint num_rows = 8;
            
            /* The last row of blocks in chrominance for 160x120 resolution
             * is half the normal height and must be accounted for. */
            if (y + 1 == ctx->num_vblocks_cbcr && ctx->frame_height % 16 != 0)
                num_rows = 4;
            
            for (x = 0; x < ctx->num_hblocks_cbcr; x++) {

                /* Calculate final offset into buffer. */
                offset = base_offset + (ctx->crcb_stride * 8 * y) + (x * 8);

                src = NULL;
                encoded = FALSE;

                if (is_pframe) {
                    
                    /* Is the current block similar enough to what it was in the previous frame? */

                    match = compare_blocks(ctx->prev_frame_buf + offset,
                                           ctx->cur_frame_buf + offset,
                                           ctx->crcb_stride, num_rows,
                                           TRUE);

                    if (match > CHROMINANCE_THRESHOLD) {

                        /* Yes: write out '0' to indicate a no-change condition. */
                        
                        _write_bits(ctx, 0, 1);
                        
                        encoded = TRUE;

                        src = ctx->prev_frame_buf + offset;
                        dst = ctx->cur_frame_buf + offset;
                        for (i = 0; i < num_rows; i++) {

                            memcpy(dst, src, 8);

                            src += ctx->crcb_stride;
                            dst += ctx->crcb_stride;
                        }
                    }
                
                }

                if (!encoded) {

                    /* Keyframe or just not similar enough? ;-) Well, encode it then. */
                    
                    if (is_pframe)
                        _write_bits(ctx, 1, 1);
                    
                    /* Use a temporary array to handle cases where the
                     * current block is not of normal height (see above). */
                    src = ctx->cur_frame_buf + offset;
                    dst = tmp_block;
                    for (i = 0; i < 8; i++) {
                        
                        memcpy(dst, src, 8);

                        if (i < (num_rows - 1))
                            src += ctx->crcb_stride;
                        dst += 8;
                    }
                    
                    _fdct_quant_block(ctx,
                                      dct_block,
                                      tmp_block,
                                      8,
                                      TRUE,
                                      ctx->num_coeffs);

                    _vlc_encode_block(ctx,
                                      dct_block,
                                      ctx->num_coeffs);
                    
                }

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
}

/*
 * compare_blocks
 *
 * Helper-function used to compare two blocks and
 * determine how similar they are.
 */
static gdouble compare_blocks(const guchar *p1,
                              const guchar *p2,
                              gint stride,
                              gint row_count,
                              gboolean is_chrom)
{
	gint i, j, sum;
    gdouble d;

	sum = 0;

	for (i = 0; i < row_count; i++) {

		for (j = 0; j < 8; j++) {

			gint d = p2[j] - p1[j];

			sum += d * d;
		}

		p1 += stride;
		p2 += stride;
	}

    if (is_chrom) {
        if (row_count == 8)
            d = sum * 0.015625;
        else
            d = sum * 0.03125;
    } else {
        d = sum / 64;
    }
    
    if (d == 0.0f)
        return 100.0f;
    else
        return (10.0f * log(65025.0f / d)) / G_LN10;
}

