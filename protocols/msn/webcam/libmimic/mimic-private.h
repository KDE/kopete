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

#ifndef MIMIC_PRIVATE_H
#define MIMIC_PRIVATE_H

#include "mimic.h"

#define ENCODER_BUFFER_SIZE         16384
#define ENCODER_QUALITY_DEFAULT     0
#define ENCODER_QUALITY_MIN         0
#define ENCODER_QUALITY_MAX         10000

struct _MimCtx {
    gboolean encoder_initialized;
    gboolean decoder_initialized;
    
    gint frame_width;
    gint frame_height;
    gint quality;
    gint num_coeffs;
    
    gint y_stride;
    gint y_row_count;
    gint y_size;
    
    gint crcb_stride;
    gint crcb_row_count;
    gint crcb_size;
    
    gint num_vblocks_y;
    gint num_hblocks_y;
    
    gint num_vblocks_cbcr;
    gint num_hblocks_cbcr;
    
    guchar *cur_frame_buf;
    guchar *prev_frame_buf;
    
    gint8 vlcdec_lookup[2296];
    
    gchar *data_buffer;
    guint data_index;

    guint32 cur_chunk;
    gint cur_chunk_len;
    
    guint32 *chunk_ptr;
    gboolean read_odd;

    gint frame_num;
    
    gint ptr_index;
    guchar *buf_ptrs[16];
};

typedef struct {
	guchar length1;
	guint32 part1;

	guchar length2;
	guint32 part2;
} VlcSymbol;

typedef struct {
    guint32 magic;
    guchar pos_add;
    guchar num_bits;
} VlcMagic;

void _mimic_init(MimCtx *ctx, gint width, gint height);
guchar _clamp_value(gint value);

guint32 _read_bits(MimCtx *ctx, gint num_bits);
void _write_bits(MimCtx *ctx, guint32 bits, gint length);

void _vlc_encode_block(MimCtx *ctx, const gint *block, gint num_coeffs);
gboolean _vlc_decode_block(MimCtx *ctx, gint *block, gint num_coeffs);

void _fdct_quant_block(MimCtx *ctx, gint *block, const guchar *src,
                       gint stride, gboolean is_chrom, gint num_coeffs);
void _idct_dequant_block(MimCtx *ctx, gint *block, gboolean is_chrom);

VlcMagic *_find_magic(guint magic);
void _initialize_vlcdec_lookup(gint8 *lookup_tbl);

void _rgb_to_yuv(const guchar *input_rgb,
                 guchar *output_y,
                 guchar *output_cb,
                 guchar *output_cr,
                 gint width,
                 gint height);
void _yuv_to_rgb(const guchar *input_y,
                 const guchar *input_cb,
                 const guchar *input_cr,
                 guchar *output_rgb,
                 guint width,
                 guint height);

void _deblock(guchar *blocks, guint stride, guint row_count);

#endif

