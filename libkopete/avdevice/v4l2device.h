/*
    v4l2device.cpp  -  Kopete V4L2 Video Device Support

    Copyright (c) 2005-2009 by Cláudio da Silveira Pinheiro	<taupter@gmail.com>
    Copyright (c) 2005-2009 by Detlev Casanova			<detlev.casanova@gmail.com>

    Kopete    (c) 2002-2009      by the Kopete developers  <kopete-devel@kde.org>

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
#ifdef V4L2_CAP_VIDEO_CAPTURE

#ifndef KOPETE_AV_V4L2DEVICE_H
#define KOPETE_AV_V4L2DEVICE_H

#include <config-kopete.h>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

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


#include <qstring.h>
#include <qfile.h>
#include <qimage.h>
#include <q3valuevector.h>
#include <kcombobox.h>

#include "videoinput.h"
#include "videodevice.h"

namespace Kopete {

namespace AV {
class V4l2Device : public VideoDevice
{
public:
	V4l2Device();
	virtual ~V4l2Device();
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


protected:
	int initRead();
	int initMmap();
	int initUserptr();
	void sortFrameSizes();
	
	struct v4l2_capability V4L2_capabilities;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	struct v4l2_fmtdesc fmtdesc; // Not sure if it must be here or inside detectPixelFormats(). Should inve
//	struct v4l2_input m_input;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_querymenu querymenu;
	void enumerateControls(void);
	void enumerateMenu (void);

};

}

}

#endif //HEADER
#endif //V4L2
