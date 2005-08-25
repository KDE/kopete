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
#include "mimic-private.h"

static void deblock_horizontal(guchar *blocks, guint stride, guint row_count);
static void deblock_vertical(guchar *blocks, guint stride, guint row_count);

static gboolean deblock_h_consider_entire(guchar *blocks, guint stride);
static void deblock_h_do_entire(guchar *blocks, guint stride);
static void deblock_h_do_boundaries(guchar *blocks, guint stride);

static gboolean deblock_v_consider_entire(guchar *blocks, guint stride);
static void deblock_v_do_entire(guchar *blocks, guint stride);
static void deblock_v_do_boundaries(guchar *blocks, guint stride);

/*
 * _deblock
 *
 * Internal helper-function used for de-blocking.
 */
void _deblock(guchar *blocks, guint stride, guint row_count)
{
    deblock_horizontal(blocks, stride, row_count);
    deblock_vertical(blocks, stride, row_count);
}

static void deblock_horizontal(guchar *blocks, guint stride, guint row_count)
{
    guchar *p1;
    gint i, j, n1, n2;

    if (stride <= 8 || row_count == 0)
        return;

    p1 = blocks + 4;
    n1 = ((row_count - 1) >> 2) + 1;
    n2 = ((stride - 9) >> 3) + 1;

    for (i = 0; i < n1; i++) {
        guchar *p;

        p = p1;

        for (j = 0; j < n2; j++) {

            if (deblock_h_consider_entire(p - 1, stride) == TRUE) {

                gint v1, v2, v;

                v1 = p[0];
                v2 = p[7];

                v = v1 - v2;
                if (v <= 0)
                    v = v2 - v1;

                if (v < 20)
                    deblock_h_do_entire(p - 1, stride);

            } else {
                deblock_h_do_boundaries(p - 1, stride);
            }

            p += 8;
        }

        p1 += stride * 4;
    }
}

static void deblock_vertical(guchar *blocks, guint stride, guint row_count)
{
    gint i, j, k, n1, n2;
    guchar *p1, *p2;

    if (stride == 0 || row_count <= 8)
        return;

    p1 = blocks + (stride * 3);
    p2 = blocks + (stride * 4);

    n1 = ((row_count - 9) >> 3) + 1;
    n2 = ((stride - 1) >> 3) + 1;

    for (i = 0; i < n1; i++) {
        guchar *p3, *p4;

        p3 = p1;
        p4 = p2;

        for (j = 0; j < n2; j++) {

            if (deblock_v_consider_entire(p3, stride) == TRUE) {
                guchar *p5;
                gboolean do_entire;

                p5 = p3 + (stride * 8);
                do_entire = TRUE;

                for (k = 0; k < 8; k++) {
                    gint v1, v2, v;

                    v1 = p4[k];
                    v2 = p5[k];

                    v = v1 - v2;
                    if (v <= 0)
                        v = v2 - v1;

                    if (v > 20) {
                        do_entire = FALSE;
                        break;
                    }
                }

                if (do_entire)
                    deblock_v_do_entire(p3, stride);
            } else {
                deblock_v_do_boundaries(p3, stride);
            }

            p3 += 8;
            p4 += 8;
        }

        p1 += stride * 8;
        p2 += stride * 8;
    }
}

static gboolean deblock_h_consider_entire(guchar *blocks, guint stride)
{
    guchar *p;
    gint i, j, count;

    count = 0;
    p = blocks;

    for (i = 0; i < 4; i++) {

        for (j = 1; j <= 7; j++) {
            gint v1, v2, v;

            v1 = p[j];
            v2 = p[j+1];

            v = v1 - v2;
            if (v <= 0)
                v = v2 - v1;

            if (v <= 1)
                count--;
        }

        p += stride;
    }

    return (count <= -20);
}

static void deblock_h_do_entire(guchar *blocks, guint stride)
{
    guchar buf[8], *p;
    gint i;

    p = blocks;

    for (i = 0; i < 4; i++) {
        gint v, low, high;

        v = p[0] - p[1];
        if (v <= 0)
            v = p[1] - p[0];

        if (v < 10)
            low = p[0];
        else
            low = p[1];

        v = p[8] - p[9];
        if (v <= 0)
            v = p[9] - p[8];

        if (v >= 10)
            high = p[8];
        else
            high = p[9];

        v = (low * 3) + p[1] + p[2] + p[3] + p[4] + 4;
        buf[0] = (((p[1] + v) << 1) - p[4] + p[5]) >> 4;

        v += p[5] - low;
        buf[1] = (((p[2] + v) << 1) - p[5] + p[6]) >> 4;

        v += p[6] - low;
        buf[2] = (((p[3] + v) << 1) - p[6] + p[7]) >> 4;

        v += p[7] - low;
        buf[3] = (((p[4] + v) << 1) - p[1] - p[7] + p[8] + low) >> 4;

        v += p[8] - p[1];
        buf[4] = (((p[5] + v) << 1) + p[1] - p[2] - p[8] + high) >> 4;

        v += high - p[2];
        buf[5] = (((p[6] + v) << 1) + p[2] - p[3]) >> 4;

        v += high - p[3];
        buf[6] = (((p[7] + v) << 1) + p[3] - p[4]) >> 4;

        v += high;
        buf[7] = (((p[8] + v) << 1) - p[4] - p[5]) >> 4;

        memcpy(p + 1, buf, 8);

        p += stride;
    }
}

static void deblock_h_do_boundaries(guchar *blocks, guint stride)
{
    guchar *p;
    gint i;

    p = blocks;

    for (i = 0; i < 4; i++) {
        gint v, v1, v2, v3;

        v = p[4] - p[5];

        if ((v / 2) != 0) {

            v1 = ((p[3] - p[6]) * 2) - (v * 5);

            if (abs(v1) < 80) {

                v2 = ((p[3] - p[2]) * 5) + ((p[1] - p[4]) * 2);
                v3 = (p[5] * 2) + (p[7] * 5) - (p[8] * 7);

                v = abs(v1) - MIN(abs(v2), abs(v3));

                if (v > 0) {

                    v = ((v * 5) + 32) >> 6;
                    if (v > 0) {

                        v2 = (p[4] - p[5]) / 2;
                        v3 = (((v1 < 0) * 2) - 1) * v;

                        if (v2 > 0)
                            v = MIN(v2, ((v3 < 0) - 1) & v3);
                        else
                            v = MAX(v2, ((v3 > 0) - 1) & v3);

                        p[4] -= v;
                        p[5] += v;
                    }
                }
            }
        }

        p += stride;
    }
}

static gboolean deblock_v_consider_entire(guchar *blocks, guint stride)
{
    gint count, i, j;
    guchar *p1, *p2;

    count = 0;

    p1 = blocks + stride;
    p2 = blocks + (stride * 2);

    for (i = 0; i < 7; i++) {

        for (j = 0; j < 8; j++) {
            gint v1, v2, v;

            v1 = p1[j];
            v2 = p2[j];

            v = v1 - v2;
            if (v <= 0)
                v = v2 - v1;

            if (v <= 1)
                count++;
        }

        p1 += stride;
        p2 += stride;
    }

    return (count > 40);
}

static void deblock_v_do_entire(guchar *blocks, guint stride)
{
    gint offset0, offset1, offset2, offset3;
    gint offset4, offset5, offset6, offset7;
    gint offset8, i;
    guchar *p, buf[8];

    offset0 = stride - (stride * 6);
    offset1 = (stride * 2) - (stride * 6);
    offset2 = (stride * 3) - (stride * 6);
    offset3 = (stride * 4) - (stride * 6);
    offset4 = (stride * 5) - (stride * 6);
    offset5 = 0;
    offset6 = (stride * 7) - (stride * 6);
    offset7 = (stride * 8) - (stride * 6);
    offset8 = (stride * 9) - (stride * 6);

    p = blocks + (stride * 6);

    for (i = 0; i < 8; i++) {
        gint v, low, high;

        v = blocks[i] - p[offset0];
        if (v <= 0)
            v = p[offset0] - blocks[i];

        if (v < 10)
            low = blocks[i];
        else
            low = p[offset0];

        v = p[offset7] - p[offset8];
        if (v <= 0)
            v = p[offset8] - p[offset7];

        if (v < 10)
            high = p[offset8];
        else
            high = p[offset7];

        v = p[offset0] + (low * 3) + p[offset1] + p[offset2] + p[offset3] + 4;

        buf[0] = (((p[offset0] + v) << 1) - p[offset3] + p[offset4]) >> 4;

        v += p[offset4] - low;

        buf[1] = (((p[offset1] + v) << 1) - p[offset4] + p[0]) >> 4;

        v += p[0] - low;

        buf[2] = (((p[offset2] + v) << 1) - p[0] + p[offset6]) >> 4;

        v += p[offset6] - low;

        buf[3] = (((p[offset3] + v) << 1) - p[offset0] - p[offset6] + p[offset7] + low) >> 4;

        v += p[offset7] - p[offset0];

        buf[4] = (((p[offset4] + v) << 1) - p[offset7] - p[offset1] + p[offset0] + high) >> 4;

        v += high - p[offset1];

        buf[5] = (((p[0] + v) << 1) - p[offset2] + p[offset1]) >> 4;

        v += high - p[offset2];

        buf[6] = (((p[offset6] + v) << 1) - p[offset3] + p[offset2]) >> 4;

        v += high;

        buf[7] = (((p[offset7] + v) << 1) - p[offset4] - p[offset3]) >> 4;

        p[offset0] = buf[0];
        p[offset1] = buf[1];
        p[offset2] = buf[2];
        p[offset3] = buf[3];
        p[offset4] = buf[4];
        p[offset5] = buf[5];
        p[offset6] = buf[6];
        p[offset7] = buf[7];

        p++;
    }
}

static void deblock_v_do_boundaries(guchar *blocks, guint stride)
{
    guchar *p;
    gint offset0, offset1, offset2, offset3;
    gint offset4, offset5, offset6, offset7;
    gint i;

    p = blocks + (stride * 3);

    offset0 = stride - (stride * 3);
    offset1 = (stride * 2) - (stride * 3);
    offset2 = 0;
    offset3 = (stride * 4) - (stride * 3);
    offset4 = (stride * 5) - (stride * 3);
    offset5 = (stride * 6) - (stride * 3);
    offset6 = (stride * 7) - (stride * 3);
    offset7 = (stride * 8) - (stride * 3);

    for (i = 0; i < 8; i++) {
        gint v1, v2, v3, v;

        v1 = ((p[offset4] - p[offset3]) * 5) + ((p[offset2] - p[offset5]) * 2);

        if (abs(v1) < 80) {

            v2 = ((p[offset2] - p[offset1]) * 5) + ((p[offset0] - p[offset3]) * 2);
            v3 = ((p[offset6] - p[offset5]) * 5) + ((p[offset4] - p[offset7]) * 2);

            v = abs(v1) - MIN(abs(v2), abs(v3));
            if (v < 0)
                v = 0;

            v2 = (p[offset3] - p[offset4]) / 2;
            v3 = (((v * 5) + 32) >> 6) * (((v1 < 0) * 2) - 1);

            if (v2 > 0)
                v = MIN(v2, ((v3 < 0) - 1) & v3);
            else
                v = MAX(v2, ((v3 > 0) - 1) & v3);
        } else {
            v = 0;
        }

        p[offset3] -= v;
        p[offset4] += v;

        p++;
    }
}

