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

/**
 * Creates a new instance and returns a pointer to the new context
 * that can be used for either encoding or decoding by calling
 * #mimic_encoder_init or #mimic_decoder_init.
 *
 * #mimic_close is called to free any resources associated with
 * the context once done.
 *
 * @returns a new mimic context
 */
MimCtx *mimic_open()
{
    MimCtx *ctx;
    
    ctx = g_new0(MimCtx, 1);
    
    ctx->encoder_initialized = FALSE;
    ctx->decoder_initialized = FALSE;
    
    return ctx;
}

/**
 * Frees any resources associated with the given context.
 *
 * @param ctx the mimic context to free
 */
void mimic_close(MimCtx *ctx)
{
    if (ctx->encoder_initialized || ctx->decoder_initialized) {
        gint i;
        
        g_free(ctx->cur_frame_buf);
        
        for (i = 0; i < 16; i++)
            g_free(ctx->buf_ptrs[i]);
    }
    
    g_free(ctx);
}

/*
 * mimic_init
 *
 * Internal helper-function used to initialize
 * a given context.
 */
static void mimic_init(MimCtx *ctx, gint width, gint height)
{
    gint bufsize, i;
    
    /*
     * Dimensions-related.
     */
    ctx->frame_width = width;
    ctx->frame_height = height;

    ctx->y_stride    = ctx->frame_width;
    ctx->y_row_count = ctx->frame_height;
    ctx->y_size      = ctx->y_stride * ctx->y_row_count;
    
    ctx->crcb_stride    = ctx->y_stride    / 2;
    ctx->crcb_row_count = ctx->y_row_count / 2;
    ctx->crcb_size      = ctx->crcb_stride * ctx->crcb_row_count;
    
    ctx->num_vblocks_y = ctx->frame_height / 8;
    ctx->num_hblocks_y = ctx->frame_width  / 8;

    ctx->num_vblocks_cbcr = ctx->frame_height / 16;
    ctx->num_hblocks_cbcr = ctx->frame_width  / 16;

    if (ctx->frame_height % 16 != 0)
        ctx->num_vblocks_cbcr++;

    /*
     * Initialize state.
     */
    ctx->frame_num = 0;
    ctx->ptr_index = 15;
    ctx->num_coeffs = 28;

    /*
     * Allocate memory for buffers.
     */
    ctx->cur_frame_buf = g_new(guchar, (320 * 240 * 3) / 2);

    bufsize = ctx->y_size + (ctx->crcb_size * 2);
    for (i = 0; i < 16; i++)
        ctx->buf_ptrs[i] = g_new(guchar, bufsize);

    /*
     * Initialize vlc lookup used by decoder.
     */
    _initialize_vlcdec_lookup(ctx->vlcdec_lookup);
}

/**
 * Initialize the mimic encoder and prepare for encoding by
 * initializing internal state and allocating resources as
 * needed.
 * 
 * After initializing use #mimic_get_property to determine
 * the size of the output buffer needed for calls to
 * #mimic_encode_frame. Use #mimic_set_property to set
 * encoding quality.
 *
 * Note that once a given context has been initialized
 * for either encoding or decoding it is not possible
 * to initialize it again.
 *
 * @param ctx the mimic context to initialize
 * @param resolution a #MimicResEnum used to specify the resolution
 * @returns #TRUE on success
 */
gboolean mimic_encoder_init(MimCtx *ctx, const MimicResEnum resolution)
{
    gint width, height;
    
    /* Check if we've been initialized before. */
    if (ctx->encoder_initialized || ctx->decoder_initialized)
        return FALSE;

    /* Check resolution. */
    if (resolution == MIMIC_RES_LOW) {
        width = 160;
        height = 120;
    } else if (resolution == MIMIC_RES_HIGH) {
        width = 320;
        height = 240;
    } else {
        return FALSE;
    }

    /* Initialize! */
    mimic_init(ctx, width, height);

    /* Set a default quality setting. */
    ctx->quality = ENCODER_QUALITY_DEFAULT;

    ctx->encoder_initialized = TRUE;
    
    return TRUE;
}

/**
 * Initialize the mimic decoder. The frame passed in frame_buffer
 * is used to determine the resolution so that the internal state
 * can be prepared and resources allocated accordingly. Note that
 * the frame passed has to be a keyframe.
 *
 * After initializing use #mimic_get_property to determine required
 * buffer-size, resolution, quality, etc.
 *
 * Note that once a given context has been initialized
 * for either encoding or decoding it is not possible
 * to initialize it again.
 *
 * @param ctx the mimic context to initialize
 * @param frame_buffer buffer containing the first frame to decode
 * @returns #TRUE on success
 */
gboolean mimic_decoder_init(MimCtx *ctx, const guchar *frame_buffer)
{
    gint width, height;
    gboolean is_keyframe;
    
    /* Check if we've been initialized before and that
     * frame_buffer is not NULL. */
    if (ctx->encoder_initialized || ctx->decoder_initialized ||
        frame_buffer == NULL)
    {
        return FALSE;
    }
    
    /* Check resolution. */
    width  = GUINT16_FROM_LE(*((guint16 *) (frame_buffer + 4)));
    height = GUINT16_FROM_LE(*((guint16 *) (frame_buffer + 6)));
    
    if (!(width == 160 && height == 120) && !(width == 320 && height == 240))
        return FALSE;

    /* Check that we're initialized with a keyframe. */
    is_keyframe = (GUINT32_FROM_LE(*((guint32 *) (frame_buffer + 12))) == 0);
    
    if (!is_keyframe)
        return FALSE;

    /* Get quality setting (in case we get queried for it before decoding). */
    ctx->quality = GUINT16_FROM_LE(*((guint16 *) (frame_buffer + 2)));
 
    /* Initialize! */
    mimic_init(ctx, width, height);

    ctx->decoder_initialized = TRUE;

    return TRUE;
}

/**
 * Get a property from a given mimic context. The context
 * has to be initialized.
 *
 * Currently the following properties are defined:
 *   - "buffer_size"
 *       - Required output buffer size
 *   - "width"
 *       - Frame width
 *   - "height"
 *       - Frame height
 *   - "quality"
 *       - Encoder: Encoding quality used
 *       - Decoder: Decoding quality of the last known frame
 *
 * @param ctx the mimic context to retrieve the property from
 * @param name of the property to retrieve the current value of
 * @param data pointer to the data that will receive the retrieved value
 * @returns #TRUE on success
 */
gboolean mimic_get_property(MimCtx *ctx, const gchar *name, gpointer data)
{
    /* Either the encoder or the decoder has to be initialized. */
    if (!ctx->encoder_initialized && !ctx->decoder_initialized)
        return FALSE;

    if (ctx->encoder_initialized) {

        if (strcmp(name, "buffer_size") == 0) {
            *((gint *) data) = ENCODER_BUFFER_SIZE;

            return TRUE;
        }
        
    } else { /* decoder_initialized */

        if (strcmp(name, "buffer_size") == 0) {
            *((gint *) data) = ctx->frame_width * ctx->frame_height * 3;

            return TRUE;
        }
    }

    if (strcmp(name, "width") == 0) {
        *((gint *) data) = ctx->frame_width;

        return TRUE;
    } else if (strcmp(name, "height") == 0) {
        *((gint *) data) = ctx->frame_height;
        
        return TRUE;
    } else if (strcmp(name, "quality") == 0) {
        *((gint *) data) = ctx->quality;
        
        return TRUE;
    }

    return FALSE;
}

/**
 * Set a property in a given mimic context. The context
 * has to be initialized.
 *
 * Currently the following properties are defined:
 *   - "quality"
 *       - Encoding quality used by encoder.
 *
 * @param ctx the mimic context to set a property in
 * @param name of the property to set to a new value
 * @param data pointer to the data that contains the new value
 * @returns #TRUE on success
 */
gboolean mimic_set_property(MimCtx *ctx, const gchar *name, gpointer data)
{
    /* Either the encoder or the decoder has to be initialized. */
    if (!ctx->encoder_initialized && !ctx->decoder_initialized)
        return FALSE;

    if (ctx->encoder_initialized) {

        if (strcmp(name, "quality") == 0) {
            gint new_quality = *((gint *) data);

            if (new_quality < ENCODER_QUALITY_MIN ||
                new_quality > ENCODER_QUALITY_MAX)
            {
                return FALSE;
            }

            ctx->quality = new_quality;

            return TRUE;
        }
        
    } else { /* decoder_initialized */ }

    return FALSE;
}

/*
 * _clamp_value
 *
 * Internal helper-function used to clamp a given
 * value to the range [ 0, 255 ].
 */
guchar _clamp_value(gint value)
{
    if (value < 0)
        return 0;
    else if (value > 255)
        return 255;
    else
        return value;
}

