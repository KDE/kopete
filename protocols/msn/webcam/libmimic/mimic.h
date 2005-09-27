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

#ifndef MIMIC_H
#define MIMIC_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libmimic libmimic public API
 * @brief The public API of the libmimic library
 *
 * libmimic provides the API required for encoding and decoding
 * MIMIC v2.x-encoded content.
 *
 * @{
 */

/**
 * The mimic encoding/decoding context returned by #mimic_open
 * and used for all further API calls until #mimic_close.
 */
typedef struct _MimCtx MimCtx;

typedef enum {
    MIMIC_RES_LOW,      /**< 160x120 resolution */
    MIMIC_RES_HIGH      /**< 320x240 resolution */
} MimicResEnum;

MimCtx *mimic_open();
void mimic_close(MimCtx *ctx);

gboolean mimic_encoder_init(MimCtx *ctx, const MimicResEnum resolution);
gboolean mimic_decoder_init(MimCtx *ctx, const guchar *frame_buffer);

gboolean mimic_get_property(MimCtx *ctx, const gchar *name, gpointer data);
gboolean mimic_set_property(MimCtx *ctx, const gchar *name, gpointer data);

gboolean mimic_encode_frame(MimCtx *ctx,
                            const guchar *input_buffer,
                            guchar *output_buffer,
                            gint *output_length,
                            gboolean make_keyframe);
gboolean mimic_decode_frame(MimCtx *ctx,
                            const guchar *input_buffer,
                            guchar *output_buffer);

/** @} */

#ifdef __cplusplus
}
#endif

#endif

