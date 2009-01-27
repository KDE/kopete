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

#ifndef KOPETE_AV_V4L1DEVICE_H
#define KOPETE_AV_V4L1DEVICE_H

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
#include "videodevice.h"

namespace Kopete {

namespace AV {
class V4l1Device : public VideoDevice
{
public:
	V4l1Device();
	virtual ~V4l1Device();
	virtual int setSize(QSize newSize);
	virtual int detectPixelFormats();
	virtual int detectSignalStandards();
	virtual int checkDevice();
	virtual int initDevice();
	
	virtual unsigned int setPixelFormat(unsigned int newformat);
	
	virtual int selectInput(int input);
	virtual int startCapturing();
	virtual int getFrame();
	virtual int stopCapturing();

	virtual float setBrightness(float brightness);
	virtual float setContrast(float contrast);
	virtual float setSaturation(float saturation);
	virtual float setWhiteness(float whiteness);
	virtual float setHue(float Hue);

#if defined(__linux__) && defined(ENABLE_AV)
	struct video_capability V4L_capabilities;
	struct video_buffer V4L_videobuffer;
#endif	

protected:
	int initRead(); 
	int initMmap(); //FIXME:MMAP not supported in v4l1
	int initUserptr(); // FIXME:USERPTR not supported in v4l1
};

}

}

#endif
