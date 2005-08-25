/***************************************************************************
                           qvideo.cpp
                           ----------
    begin                : Sat Jun 12 2004
    copyright            : (C) 2004 by Dirk Ziegelmeier
                           (C) 2002 by George Staikos
    email                : dziegel@gmx.de
 ***************************************************************************/

/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "qvideo.h"

#include <kdebug.h>
#include <qpaintdevice.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

unsigned int QVideo::bytesppForFormat(ImageFormat fmt)
{
    switch (fmt) {
    case FORMAT_RGB32:
    case FORMAT_RGB24:
    case FORMAT_BGR32:
    case FORMAT_BGR24:
        return 4;
        
    case FORMAT_RGB15_LE:
    case FORMAT_RGB16_LE:
    case FORMAT_RGB15_BE:
    case FORMAT_RGB16_BE:
    case FORMAT_YUYV:
    case FORMAT_UYVY:
    case FORMAT_YUV422P:
    case FORMAT_YUV420P:
        return 2;
        
    case FORMAT_GREY:
    case FORMAT_HI240:
        return 1;
        
    default:
        // unknown format
        return 0;
    }
}

bool QVideo::findDisplayProperties(ImageFormat& fmt, int& depth, unsigned int& bitsperpixel, int& bytesperpixel)
{
    XVisualInfo *vi_in, vi_out;
    long mask = VisualScreenMask;
    int  nvis = 0;

    ImageFormat p = FORMAT_NONE;
    int bpp       = 0;
    int d         = 0;
    
	vi_out.screen = QPaintDevice::x11AppScreen();
	vi_in         = XGetVisualInfo(qt_xdisplay(), mask, &vi_out, &nvis);

	if (vi_in) {
		for (int i = 0; i < nvis; i++) {
            bpp = 0;
			int n;
			XPixmapFormatValues *pf = XListPixmapFormats(qt_xdisplay(),&n);
            d = vi_in[i].depth;
			for (int j = 0; j < n; j++) {
				if (pf[j].depth == d) {
					bpp = pf[j].bits_per_pixel;
					break;
				}
			}
			XFree(pf);

            // FIXME: Endianess detection
            
            p = FORMAT_NONE;
			switch (bpp) {
			case 32:
				if (vi_in[i].red_mask   == 0xff0000 &&
				    vi_in[i].green_mask == 0x00ff00 &&
				    vi_in[i].blue_mask  == 0x0000ff) {
					p = FORMAT_BGR32;
					kdDebug() << "QVideo: Found BGR32 display." << endl;
				}
                break;
			case 24:
				if (vi_in[i].red_mask   == 0xff0000 &&
				    vi_in[i].green_mask == 0x00ff00 &&
				    vi_in[i].blue_mask  == 0x0000ff) {
					p = FORMAT_BGR24;
					kdDebug() << "QVideo: Found BGR24 display." << endl;
				}
                break;
			case 16:
				if (vi_in[i].red_mask   == 0x00f800 &&
				    vi_in[i].green_mask == 0x0007e0 &&
				    vi_in[i].blue_mask  == 0x00001f) {
					p = FORMAT_RGB15_LE;
					kdDebug() << "QVideo: Found RGB16_LE display." << endl;
				} else
                    if (vi_in[i].red_mask   == 0x007c00 &&
                        vi_in[i].green_mask == 0x0003e0 &&
                        vi_in[i].blue_mask  == 0x00001f) {
                        p = FORMAT_RGB15_LE;
                        kdDebug() << "QVideo: Found RGB15_LE display." << endl;
                    }
                break;
			case 8:
			default:
				continue;
			}

			if (p != FORMAT_NONE)
				break;
		}
		XFree(vi_in);
	}
    
    if (p != FORMAT_NONE) {
        int bytespp = bytesppForFormat(p);        
        kdDebug() << "QVideo: Display properties: depth: " << d
                  << ", bits/pixel: " << bpp
                  << ", bytes/pixel: " << bytespp << endl;
        fmt           = p;
        bitsperpixel  = bpp;
        bytesperpixel = bytespp;
        depth         = d;
        return true;
    } else {
        kdWarning() << "QVideo: Unable to find out palette. What display do you have????" << endl;
        fmt           = FORMAT_NONE;
        bitsperpixel  = 0;
        bytesperpixel = 0;
        depth         = 0;
        return false;
    }
}
