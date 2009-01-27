/*
    videodevice.cpp  -  Kopete Video Device Low-level Support

    Copyright (c) 2005-2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

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

#include <config-kopete.h>

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
#ifndef __s64 //required by videodev.h
#define __s64 signed long long
#endif // __s64


#ifndef pgoff_t
#define pgoff_t unsigned long
#endif

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/videodev.h>
#define VIDEO_MODE_PAL_Nc  3
#define VIDEO_MODE_PAL_M   4
#define VIDEO_MODE_PAL_N   5
#define VIDEO_MODE_NTSC_JP 6
#define __STRICT_ANSI__

#endif // __linux__

#include <qstring.h>
#include <qfile.h>
#include <qimage.h>
#include <q3valuevector.h>
#include <kcombobox.h>

#include "videoinput.h"

namespace Kopete {

namespace AV {

/**
@author Kopete Developers
*/

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
	int pixelformat;
	QVector <uchar> data; // maybe it should be a rawbuffer instead of it? It could make us avoid a memory copy
};
struct rawbuffer // raw buffer
{
	uchar * start;
	size_t length;
};


class VideoDevice{
public:
	VideoDevice();
	virtual ~VideoDevice();
	int setFileName(QString filename);
	int open();
	bool isOpen();
	int inputs();
	virtual int setSize(QSize newSize);
	QSize frameSize();
	virtual int detectPixelFormats();
	virtual int detectSignalStandards();
	virtual int checkDevice();
	virtual int initDevice();
	
	int showDeviceCapabilities();
	void sortFrameSizes();

	//Those method might be reworked too, I really don't see why we
	//use our own pixel_format type as long as the v4l pixelformat is 
	//just as good.
	virtual unsigned int setPixelFormat(unsigned int newformat);
	int pixelFormatDepth(unsigned int pixelformat);
	QString pixelFormatName(unsigned int pixelformat);
	
	//Idem
	QString signalStandardName(int standard);

	int currentInput();
	virtual int selectInput(int input);
	int setInputParameters();
	virtual int startCapturing();
	virtual int getFrame();
	int getFrame(imagebuffer *imgbuffer);
	int getImage(QImage *qimage);
	virtual int stopCapturing();
	int close();

	float getBrightness();
	virtual float setBrightness(float brightness);
	float getContrast();
	virtual float setContrast(float contrast);
	float getSaturation();
	virtual float setSaturation(float saturation);
	float getWhiteness();
	virtual float setWhiteness(float whiteness);
	float getHue();
	virtual float setHue(float Hue);

	bool getAutoBrightnessContrast();
	bool setAutoBrightnessContrast(bool brightnesscontrast);
	bool getAutoColorCorrection();
	bool setAutoColorCorrection(bool colorcorrection);
	bool getImageAsMirror();
	bool setImageAsMirror(bool imageasmirror);

	bool canCapture();
	bool canChromakey();
	bool canScale();
	bool canOverlay();
	bool canRead();
	bool canAsyncIO();
	bool canStream();

	void setUdi( const QString & );
	QString udi() const;
	QString m_model;
	QString m_name;
	size_t m_modelindex; // Defines what's the number of a device when more than 1 device of a given model is present;
	QString full_filename;
	//videodev_driver m_driver;
	int descriptor;

	QVector<Kopete::AV::VideoInput> m_input;

protected:
	QVector<rawbuffer> m_rawbuffers;
	unsigned int m_streambuffers;
	imagebuffer m_currentbuffer;
	int m_buffer_size;

	int m_current_input;

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

	QString m_udi;

	QList<QSize> m_frameSizes;
	QList<unsigned int> m_pixelFormats;

	QSize currentFrameSize;
	unsigned int m_pixelformat;
};

}

}

#endif
