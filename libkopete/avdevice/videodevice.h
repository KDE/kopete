/*
    videodevice.cpp  -  Kopete Video Device Low-level Support

    Copyright (c) 2005 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

    Kopete    (c) 2002-2003      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#define ENABLE_AV

#ifndef KOPETE_AVVIDEODEVICELISTITEM_H
#define KOPETE_AVVIDEODEVICELISTITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#if defined(__linux__) && defined(ENABLE_AV)

#include <asm/types.h>
#undef __STRICT_ANSI__
#ifndef __u64 //required by videodev.h
#define __u64 unsigned long long
#endif // __u64

#ifndef pgoff_t
#define pgoff_t unsigned long
#endif

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/videodev.h>
#define __STRICT_ANSI__

#endif // __linux__

#include <qstring.h>
#include <qfile.h>
#include <qimage.h>
#include <qvaluevector.h>
#include <kcombobox.h>

#include "videoinput.h"

namespace Kopete {

namespace AV {

/**
@author Kopete Developers
*/
typedef enum
{
	VIDEODEV_DRIVER_NONE
#if defined( __linux__) && defined(ENABLE_AV)
        ,
	VIDEODEV_DRIVER_V4L
#ifdef HAVE_V4L2
        ,
	VIDEODEV_DRIVER_V4L2
#endif
#endif
} videodev_driver;

typedef enum
{
	PIXELFORMAT_NONE	= 0,
	PIXELFORMAT_GREY	= (1 << 0),
	PIXELFORMAT_RGB332	= (1 << 1),
	PIXELFORMAT_RGB555	= (1 << 2),
	PIXELFORMAT_RGB555X	= (1 << 3),
	PIXELFORMAT_RGB565	= (1 << 4),
	PIXELFORMAT_RGB565X	= (1 << 5),
	PIXELFORMAT_RGB24	= (1 << 6),
	PIXELFORMAT_BGR24	= (1 << 7),
	PIXELFORMAT_RGB32	= (1 << 8),
	PIXELFORMAT_BGR32	= (1 << 9),
	PIXELFORMAT_YUYV	= (1 << 10),
	PIXELFORMAT_UYVY	= (1 << 11),
	PIXELFORMAT_YUV420P	= (1 << 12),
	PIXELFORMAT_YUV422P	= (1 << 13)
//	PIXELFORMAT_ALL		= 0x00003FFF
} pixel_format;

typedef enum
{
	IO_METHOD_NONE,
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR
} io_method;

struct imagebuffer
{
	int height;
	int width;
	pixel_format pixelformat;
	QValueVector <uchar> data; // maybe it should be a rawbuffer instead of it? It could make us avoid a memory copy
};
struct rawbuffer // raw buffer
{
	uchar * start;
	size_t length;
};


class VideoDevice{
public:
	VideoDevice();
	~VideoDevice();
	int setFileName(QString filename);
	int open();
	bool isOpen();
	int checkDevice();
	int showDeviceCapabilities();
	int initDevice();
	unsigned int inputs();
	int width();
	int minWidth();
	int maxWidth();
	int height();
	int minHeight();
	int maxHeight();
	int setSize( int newwidth, int newheight);
	pixel_format setPixelFormat(pixel_format newformat);
	int pixelFormatCode(pixel_format pixelformat);
	pixel_format pixelFormatForPalette( int palette );
	int pixelFormatDepth(pixel_format pixelformat);
	QString pixelFormatName(pixel_format pixelformat);
	QString pixelFormatName(int pixelformat);
	int currentInput();
	int selectInput(int input);
	int startCapturing();
	int getFrame();
	int getFrame(imagebuffer *imgbuffer);
	int getImage(QImage *qimage);
	int stopCapturing();
	int close();

	float getBrightness();
	float setBrightness(float brightness);
	float getContrast();
	float setContrast(float contrast);
	float getSaturation();
	float setSaturation(float saturation);
	float getHue();
	float setHue(float Hue);
	bool getAutoBrightnessContrast();
	bool setAutoBrightnessContrast(bool brightnesscontrast);
	bool getAutoColorCorrection();
	bool setAutoColorCorrection(bool colorcorrection);

	bool canCapture();
	bool canChromakey();
	bool canScale();
	bool canOverlay();
	bool canRead();
	bool canAsyncIO();
	bool canStream();

	QString m_name;
	QString m_model;
	size_t m_modelindex;
	QString full_filename;
	videodev_driver m_driver;
	int descriptor;

//protected:
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
	struct v4l2_capability V4L2_capabilities;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
#endif
	struct video_capability V4L_capabilities;
	struct video_buffer V4L_videobuffer;
#endif	
	QValueVector<Kopete::AV::VideoInput> m_input;
//	QFile file;
protected:
	int currentwidth, minwidth, maxwidth, currentheight, minheight, maxheight;

	QValueVector<rawbuffer> m_rawbuffers;
	unsigned int m_streambuffers;
	imagebuffer m_currentbuffer;
	int m_buffer_size;

	int m_current_input;
	pixel_format m_pixelformat;

	io_method m_io_method;
	bool m_videocapture;
	bool m_videochromakey;
	bool m_videoscale;
	bool m_videooverlay;
	bool m_videoread;
	bool m_videoasyncio;
	bool m_videostream;

	int xioctl(int request, void *arg);
	int errnoReturn(const char* s);
	int initRead();
	int initMmap();
	int initUserptr();

};

}

}

#endif
