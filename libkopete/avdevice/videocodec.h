/*
    videocodec.h  -  Kopete Video Codecs for Webcam Support

    Copyright (c) 2007      by Alexandre DENIS <contact@alexandredenis.net>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    Contains excerpts from luvcview
    Copyright (C) 2005-2006 by Laurent Pinchart & Michel Xhaard

    JPEG decoder from http://www.bootsplash.org/
    (w) August 2001         by Michael Schroeder <mls@suse.de>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qimage.h>

#define ERR_NO_SOI 1
#define ERR_NOT_8BIT 2
#define ERR_HEIGHT_MISMATCH 3
#define ERR_WIDTH_MISMATCH 4
#define ERR_BAD_WIDTH_OR_HEIGHT 5
#define ERR_TOO_MANY_COMPPS 6
#define ERR_ILLEGAL_HV 7
#define ERR_QUANT_TABLE_SELECTOR 8
#define ERR_NOT_YCBCR_221111 9
#define ERR_UNKNOWN_CID_IN_SCAN 10
#define ERR_NOT_SEQUENTIAL_DCT 11
#define ERR_WRONG_MARKER 12
#define ERR_NO_EOI 13
#define ERR_BAD_TABLES 14
#define ERR_DEPTH_MISMATCH 15

namespace Kopete
{
  namespace AV
  {
    class VideoCodec
    {
      
    public:
      static void yuyv2qimage(QImage*qimage, const unsigned char*pic, int width, int height);
      
      static int  jpeg_decode(unsigned char**pic, const unsigned char*buf, int*width, int*height);
      
      static void computeMirrorImage(QImage*dest, const QImage*source);
    };
  }
}
