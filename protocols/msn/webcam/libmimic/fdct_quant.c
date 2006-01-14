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

extern guchar _col_zag[64];

void _fdct_quant_block(MimCtx *ctx, gint *block, const guchar *src,
                       gint stride, gboolean is_chrom, gint num_coeffs)
{
	gint sum1, sum2, sum3, sum4;
	gint diff1, diff2, diff3, diff4;
	gint ex1, ex2, ex3, ex4, ex5;
	gint i, j;
	const guchar *p1;
	gint *iptr;

    /*
     * Forward DCT, first pass (horizontal).
     */
    p1 = src;
    iptr = block;

	for (i = 0; i < 8; i++) {
		sum1 = p1[0] + p1[7];
		sum2 = p1[1] + p1[6];
		sum3 = p1[2] + p1[5];
		sum4 = p1[3] + p1[4];

		diff1 = p1[0] - p1[7];
		diff2 = p1[1] - p1[6];
		diff3 = p1[2] - p1[5];
		diff4 = p1[3] - p1[4];

		ex1 = ((diff1 + diff4) *  851) - (diff1 *  282);
		ex2 = ((diff2 + diff3) * 1004) - (diff2 *  804);
		ex3 = ((diff2 + diff3) * 1004) - (diff3 * 1204);
		ex4 = ((diff1 + diff4) *  851) - (diff4 * 1420);

		iptr[0] = sum1 + sum2 + sum3 + sum4;
		iptr[2] = (((sum1 - sum4) * 1337) + ((sum2 - sum3) * 554)) >> 10;
		iptr[4] = sum1 - sum2 - sum3 + sum4;

		iptr[1] = (ex1 + ex2 + ex3 + ex4) >> 10;
		iptr[3] = ((ex4 - ex2) * 181) >> 17;
		iptr[5] = ((ex1 - ex3) * 181) >> 17;

		p1 += stride;
		iptr += 8;
	}

	p1 = src;
    iptr = block;

    /*
     * Forward DCT, first pass (vertical).
     * 
     * This is only known to be correct for i == 0, though it seems to be ...
     */
    for (i = 0; i < 6; i++) {
        sum1 = iptr[ 0 + i] + iptr[56 + i];
        sum2 = iptr[ 8 + i] + iptr[48 + i];
        sum3 = iptr[16 + i] + iptr[40 + i];
        sum4 = iptr[24 + i] + iptr[32 + i];

        diff1 = iptr[ 0 + i] - iptr[56 + i];
        diff2 = iptr[ 8 + i] - iptr[48 + i];
        diff3 = iptr[16 + i] - iptr[40 + i];
        diff4 = iptr[24 + i] - iptr[32 + i];

        ex1 = ((diff1 + diff4) *  851) - (diff1 *  282);
        ex2 = ((diff2 + diff3) * 1004) - (diff2 *  804);
        ex3 = ((diff2 + diff3) * 1004) - (diff3 * 1204);
        ex4 = ((diff1 + diff4) *  851) - (diff4 * 1420);

        ex5 = (sum1 + sum2 - sum3 - sum4) * 554;

        for (j = 0; j < 7 - i; j++) {
            switch (j) {

                case 0:
                    iptr[ 0 + i] = (16 + sum1 + sum2 + sum3 + sum4) >> 5;
                    break;

                case 1:
                    iptr[ 8 + i] = (16384 + ex1 + ex2 + ex3 + ex4) >> 15;
                    break;

                case 2:
                    iptr[16 + i] = (16384 + ((sum1 - sum4) * 783) + ex5) >> 15;
                    break;

                case 3:
                    iptr[24 + i] = (8192 + (((ex4 - ex2) >> 8) * 181)) >> 14;
                    break;

                case 4:
                    iptr[32 + i] = (16 + sum1 - sum2 - sum3 + sum4) >>  5;
                    break;

                case 5:
                    iptr[40 + i] = (8192 + (((ex1 - ex3) >> 8) * 181)) >> 14;
                    break;

                case 6:
                    iptr[48 + i] = (16384 - ((sum2 - sum3) * 1891) + ex5) >> 15;
                    break;
            }
        }
    }

    /*
     * Quantize.
     */
	block[0] /= 2;
	block[8] /= 4;
	block[1] /= 4;
	block[6] = 0;

	if (num_coeffs > 3) {

        gdouble s = (10000 - ctx->quality) * 10.0 * (gfloat) 9.9999997e-5;

        if (s > 10.0)
            s = 10.0;
        else if (is_chrom != 0 && s < 1.0)
            s = 1.0;
        else if (s < 2.0)
            s = 2.0;

		s = 1.0 / s;

		for (i = 3; i < num_coeffs; i++) {

            gdouble coeff, r;

			coeff = block[_col_zag[i]] * s;
            r = coeff - (gint) coeff;

			if (r >= 0.6)
				block[_col_zag[i]] = (gint) (coeff + 1.0);
			else if (r <= -0.6)
				block[_col_zag[i]] = (gint) (coeff - 1.0);
			else
				block[_col_zag[i]] = (gint) coeff;

			if (block[_col_zag[i]] > 120)
				block[_col_zag[i]] = 120;
			else if (block[_col_zag[i]] < -120)
				block[_col_zag[i]] = -120;
		}
	}

	if (block[8] > 120)
		block[8] = 120;
	else if (block[8] < -120)
		block[8] = -120;

	if (block[1] > 120)
		block[1] = 120;
	else if (block[1] < -120)
		block[1] = -120;

	for (i = num_coeffs; i < 64; i++)
		block[_col_zag[i]] = 0;
}

