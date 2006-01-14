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

#define RED_INDEX_1      0
#define GREEN_INDEX_1    1
#define BLUE_INDEX_1     2

#define RED_INDEX_2      3
#define GREEN_INDEX_2    4
#define BLUE_INDEX_2     5

/*
 * _rgb_to_yuv
 *
 * Internal helper-function used to convert an image
 * from RGB 24-bpp packed-pixel to YUV420 planar.
 */
void _rgb_to_yuv(const guchar *input_rgb,
                 guchar *output_y,
                 guchar *output_cb,
                 guchar *output_cr,
                 gint width,
                 gint height)
{
    gint y, x;

    for (y = 0; y < height; y += 2) {

        const guchar *src1, *src2;
        guchar *dst1, *dst2, *dst3, *dst4;
        gint num_cols;

        src1 = input_rgb + ((height - 1 - y) * width * 3);
        src2 = input_rgb + ((height - 2 - y) * width * 3);

        dst1 = output_y + (y * width);
        dst2 = output_y + ((y + 1) * width);
        dst3 = output_cb + ((y / 2) * (width / 2));
        dst4 = output_cr + ((y / 2) * (width / 2));

        num_cols = width / 2;

        for (x = 0; x < num_cols; x++) {

            gint expr1, expr2, expr3, expr4, expr5, v;

            expr1 = (src1[BLUE_INDEX_1] * 19595) + (src1[GREEN_INDEX_1] * 38470) + (src1[RED_INDEX_1] * 7471);
            expr2 = (src1[BLUE_INDEX_2] * 19595) + (src1[GREEN_INDEX_2] * 38470) + (src1[RED_INDEX_2] * 7471);
            expr3 = (src2[BLUE_INDEX_1] * 19595) + (src2[GREEN_INDEX_1] * 38470) + (src2[RED_INDEX_1] * 7471);
            expr4 = (src2[BLUE_INDEX_2] * 19595) + (src2[GREEN_INDEX_2] * 38470) + (src2[RED_INDEX_2] * 7471);

            expr5 = expr1 + expr2 + expr3 + expr4;

            dst1[0] = expr1 >> 16;
            dst1[1] = expr2 >> 16;
            dst2[0] = expr3 >> 16;
            dst2[1] = expr4 >> 16;

            v = (((src1[BLUE_INDEX_1] + src1[BLUE_INDEX_2] + src2[BLUE_INDEX_1] + src2[BLUE_INDEX_2]) << 16) - expr5 + 131071) >> 16;
            dst3[0] = _clamp_value(((v * 57475) >> 18) + 128);
            
            v = (((src1[RED_INDEX_1] + src1[RED_INDEX_2] + src2[RED_INDEX_1] + src2[RED_INDEX_2]) << 16) - expr5 + 131071) >> 16;
            dst4[0] = ((v * 32244) >> 18) + 128;

            src1 += 6;
            src2 += 6;

            dst1 += 2;
            dst2 += 2;
            dst3++;
            dst4++;

        }

    }

}

/*
 * _yuv_to_rgb
 *
 * Internal helper-function used to convert an image
 * from YUV420 planar to RGB 24-bpp packed-pixel.
 */
void _yuv_to_rgb(const guchar *input_y,
                 const guchar *input_cb,
                 const guchar *input_cr,
                 guchar *output_rgb,
                 guint width,
                 guint height)
{
    const guchar *src_y, *src_cb, *src_cr;
    guchar *dst_rgb;
    guint i, j, rgb_stride;
    
    src_y  = input_y;
    src_cb = input_cb;
    src_cr = input_cr;
    
    rgb_stride = width * 3;
    dst_rgb = output_rgb + (rgb_stride * (height - 1));
    
    for (i = 0; i < height; i++) {
        const guchar *p_y, *p_cb, *p_cr;
        guchar *p_rgb;

        p_y = src_y;
        p_cb = src_cb;
        p_cr = src_cr;

        p_rgb = dst_rgb;

        for (j = 0; j < width; j++) {
            gint v;

            v = ((p_y[0] * 65536) + ((p_cr[0] - 128) * 133169)) / 65536;
            p_rgb[0] = _clamp_value(v);

            v = ((p_y[0] * 65536) - ((p_cr[0] - 128) * 25821) - ((p_cb[0] - 128) * 38076)) / 65536;
            p_rgb[1] = _clamp_value(v);

            v = ((p_y[0] * 65536) + ((p_cb[0] - 128) * 74711)) / 65536;
            p_rgb[2] = _clamp_value(v);

            p_y++;
            if ((j + 1) % 2 == 0) {
                p_cb++;
                p_cr++;
            }

            p_rgb += 3;
        }

        src_y += width;
        if ((i + 1) % 2 == 0) {
            src_cb += (width + 1) / 2;
            src_cr += (width + 1) / 2;
        }

        dst_rgb -= rgb_stride;

    }

}

