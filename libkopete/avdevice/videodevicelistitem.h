//
// C++ Interface: videodevicelistitem
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KOPETE_AVVIDEODEVICELISTITEM_H
#define KOPETE_AVVIDEODEVICELISTITEM_H

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#ifdef __linux__

#undef __STRICT_ANSI__
#include <asm/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>

#ifndef __u64 //required by videodev.h
#define __u64 unsigned long long
#endif // __u64

#include <linux/videodev.h>
#define __STRICT_ANSI__

#endif // __linux__

#include <qstring.h>
#include <qfile.h>

namespace Kopete {

namespace AV {

/**
@author Kopete Developers
*/
typedef enum
{
	VIDEODEV_DRIVER_NONE,
	VIDEODEV_DRIVER_V4L,
	VIDEODEV_DRIVER_V4L2,
} videodev_driver;

class VideoDeviceListItem{
public:
	VideoDeviceListItem();
	~VideoDeviceListItem();
	QString name;
	QString full_filename;
	videodev_driver m_driver;
//protected:
	struct v4l2_capability V4L2_capabilities;
	struct video_capability V4L_capabilities;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
};

}

}

#endif
