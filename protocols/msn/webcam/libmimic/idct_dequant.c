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

void _idct_dequant_block(MimCtx *ctx, gint *block, gboolean is_chrom)
{
    gdouble f;
    gint i, *p;

    /*
     * De-quantize.
     */
    f = (10000 - ctx->quality) * 10.0 * (gfloat) 9.9999997e-5;

    if (f > 10.0)
        f = 10.0;

    if (!is_chrom) {
        if (f < 2.0)
            f = 2.0;
    } else {
        if (f < 1.0)
            f = 1.0;
    }

    block[0] <<= 1;
    block[1] <<= 2;
    block[8] <<= 2;

    for (i = 2; i < 64; i++) {
        if (i == 8)
            continue;

        block[i] *= f;
    }

    /*
     * Inverse DCT, first pass (horizontal).
     */
    p = block;

    for (i = 0; i < 8; i++) {
        gint v1, v2, v3, v4, v5, v6, v7, v8;
        gint va, vb;

        va = (p[0] << 11) + (p[4] << 11);
        vb = ((p[2] << 2) * 392) + (((p[2] << 2) + (p[6] << 2)) * 277);
        v1 = va + vb + 512;
        v2 = va - vb + 512;

        va = (p[0] << 11) - (p[4] << 11);
        vb = (((p[2] << 2) + (p[6] << 2)) * 277) - ((p[6] << 2) * 946);
        v3 = va + vb + 512;
        v4 = va - vb + 512;

        va = (p[1] << 9) + (p[3] * 724) + (p[7] << 9);
        vb = (p[1] << 9) + (p[5] * 724) - (p[7] << 9);
        v5 = (((va + vb) * 213) - (vb * 71)) >> 6;
        v6 = (((va + vb) * 213) - (va * 355)) >> 6;

        va = (p[1] << 9) - (p[3] * 724) + (p[7] << 9);
        vb = (p[1] << 9) - (p[5] * 724) - (p[7] << 9);
        v7 = (((va + vb) * 251) - (va * 201)) >> 6;
        v8 = (((va + vb) * 251) - (vb * 301)) >> 6;

        p[0] = (v1 + v5) >> 10;
        p[1] = (v3 + v7) >> 10;
        p[2] = (v4 + v8) >> 10;
        p[3] = (v2 + v6) >> 10;
        p[4] = (v2 - v6) >> 10;
        p[5] = (v4 - v8) >> 10;
        p[6] = (v3 - v7) >> 10;
        p[7] = (v1 - v5) >> 10;

        p += 8;
    }
    
    /*
     * Inverse dct, second pass (vertical).
     */
    p = block;

    for (i = 0; i < 8; i++) {
        gint v1, v2, v3, v4, v5, v6, v7, v8;
        gint va, vb;

        va = (p[0] << 9) + (p[32] << 9);
        vb = ((p[16] + p[48]) * 277) + (p[16] * 392);
        v1 = va + vb + 1024;
        v2 = va - vb + 1024;

        va = (p[0] << 9) - (p[32] << 9);
        vb = ((p[16] + p[48]) * 277) - (p[48] * 946);
        v3 = va + vb + 1024;
        v4 = va - vb + 1024;

        va = ((p[8] << 7) + (p[24] * 181) + (p[56] << 7)) >> 6;
        vb = ((p[8] << 7) + (p[40] * 181) - (p[56] << 7)) >> 6;
        v5 = ((va + vb) * 213) - (vb * 71);
        v6 = ((va + vb) * 213) - (va * 355);

        va = ((p[8] << 7) - (p[24] * 181) + (p[56] << 7)) >> 6;
        vb = ((p[8] << 7) - (p[40] * 181) - (p[56] << 7)) >> 6;
        v7 = ((va + vb) * 251) - (va * 201);
        v8 = ((va + vb) * 251) - (vb * 301);

        p[0] = (v1 + v5) >> 11;
        p[8] = (v3 + v7) >> 11;
        p[16] = (v4 + v8) >> 11;
        p[24] = (v2 + v6) >> 11;
        p[32] = (v2 - v6) >> 11;
        p[40] = (v4 - v8) >> 11;
        p[48] = (v3 - v7) >> 11;
        p[56] = (v1 - v5) >> 11;

        p++;
    }
}

