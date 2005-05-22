// -*- c++ -*-
/***************************************************************************
                           qvideo.h
                           --------
    begin                : Sat Jun 12 2004
    copyright            : (C) 2004 by Dirk Ziegelmeier
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

#ifndef QVIDEO_H
#define QVIDEO_H

class QVideo
{
public:
    typedef enum {
        FORMAT_NONE     =       0,
        FORMAT_GREY     =  (1<<0),
        FORMAT_HI240    =  (1<<1),
        FORMAT_RGB15_LE =  (1<<2),
        FORMAT_RGB15_BE =  (1<<3),
        FORMAT_RGB16_LE =  (1<<4),
        FORMAT_RGB16_BE =  (1<<5),
        FORMAT_RGB32    =  (1<<6),
        FORMAT_BGR32    =  (1<<7),
        FORMAT_RGB24    =  (1<<8),
        FORMAT_BGR24    =  (1<<9),
        FORMAT_YUYV     = (1<<10),
        FORMAT_UYVY     = (1<<11),
        FORMAT_YUV422P  = (1<<12),
        FORMAT_YUV420P  = (1<<13),
        FORMAT_ALL      = 0x00003FFF
    } ImageFormat;

    typedef enum {
        METHOD_NONE  =  0,
        METHOD_XSHM  =  1,
        METHOD_XV    =  2,
        METHOD_XVSHM =  4,
        METHOD_X11   =  8,
        METHOD_DGA   = 16, /* unimplemented */
        METHOD_GL    = 32,
        METHOD_SDL   = 64  /* unimplemented */
    } VideoMethod;
    
    static unsigned int bytesppForFormat(ImageFormat fmt);
    static bool findDisplayProperties(ImageFormat& fmt, int& depth, unsigned int& bitsperpixel, int& bytesperpixel);
};

#endif //QVIDEO_H

