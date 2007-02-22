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

#ifndef __s64 //required by videodev.h
#define __s64 long long
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
	PIXELFORMAT_YUV422P	= (1 << 13),
	PIXELFORMAT_MJPEG       = (1 << 14)
//	PIXELFORMAT_ALL		= 0x00003FFF
} pixel_format;

typedef enum
{
	STANDARD_NONE		= 0,
	STANDARD_PAL_B		= (1 << 0),
	STANDARD_PAL_B1		= (1 << 1),
	STANDARD_PAL_G		= (1 << 2),
	STANDARD_PAL_H		= (1 << 3),
	STANDARD_PAL_I		= (1 << 4),
	STANDARD_PAL_D		= (1 << 5),
	STANDARD_PAL_D1		= (1 << 6),
	STANDARD_PAL_K		= (1 << 7),
	STANDARD_PAL_M		= (1 << 8),
	STANDARD_PAL_N		= (1 << 9),
	STANDARD_PAL_Nc		= (1 << 10),
	STANDARD_PAL_60		= (1 << 11),
// STANDARD_PAL_60 is a hybrid standard with 525 lines, 60 Hz refresh rate, and PAL color modulation with a 4.43 MHz color subcarrier. Some PAL video recorders can play back NTSC tapes in this mode for display on a 50/60 Hz agnostic PAL TV.
	STANDARD_NTSC_M		= (1 << 12),
	STANDARD_NTSC_M_JP	= (1 << 13),
	STANDARD_NTSC_443	= (1 << 14),
// STANDARD_NTSC_443 is a hybrid standard with 525 lines, 60 Hz refresh rate, and NTSC color modulation with a 4.43 MHz color subcarrier.
	STANDARD_SECAM_B	= (1 << 16),
	STANDARD_SECAM_D	= (1 << 17),
	STANDARD_SECAM_G	= (1 << 18),
	STANDARD_SECAM_H	= (1 << 19),
	STANDARD_SECAM_K	= (1 << 20),
	STANDARD_SECAM_K1	= (1 << 21),
	STANDARD_SECAM_L	= (1 << 22),
	STANDARD_SECAM_LC	= (1 << 23),
// ATSC/HDTV
	STANDARD_ATSC_8_VSB	= (1 << 24),
	STANDARD_ATSC_16_VSB	= (1 << 25),

	STANDARD_PAL_BG		= ( STANDARD_PAL_B   | STANDARD_PAL_B1   | STANDARD_PAL_G  ),
	STANDARD_PAL_DK		= ( STANDARD_PAL_D   | STANDARD_PAL_D1   | STANDARD_PAL_K  ),
	STANDARD_PAL		= ( STANDARD_PAL_BG  | STANDARD_PAL_DK   | STANDARD_PAL_H    | STANDARD_PAL_I  ),
	STANDARD_NTSC		= ( STANDARD_NTSC_M  | STANDARD_NTSC_M_JP ),
	STANDARD_SECAM_DK       = ( STANDARD_SECAM_D | STANDARD_SECAM_K  | STANDARD_SECAM_K1 ),
	STANDARD_SECAM		= ( STANDARD_SECAM_B | STANDARD_SECAM_G  | STANDARD_SECAM_H  | STANDARD_SECAM_DK | STANDARD_SECAM_L),
	STANDARD_525_60		= ( STANDARD_PAL_M   | STANDARD_PAL_60   | STANDARD_NTSC     | STANDARD_NTSC_443),
	STANDARD_625_50		= ( STANDARD_PAL     | STANDARD_PAL_N    | STANDARD_PAL_Nc   | STANDARD_SECAM),
	STANDARD_ALL		= ( STANDARD_525_60  | STANDARD_625_50)
} signal_standard;


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
	bool isOpen() const;
	/**
	 * Device identifiers
	 */
	QString model() const;
	void setModel( const QString & );
	QString name() const;
	void setName( const QString & );
	/**
	 * The path to the device node
	 */
	QString devicePath() const;
	void setDevicePath( const QString & );
	/**
	 * Open the device, initialize it, get its capabilities, and set the current input
	 */
	int open();
	/**
	 * Discover the device's IO capabilities and initialise its cropping
	 */
	int initDevice();
	/**
	 * Get the device's capabilities using the V4L or V4L2 APIs
	 */
	int checkDevice();
	/**
	 * Dump the device's capabilities (debug)
	 */
	int showDeviceCapabilities();
	/**
	 * Number of inputs the device has
	 */
	unsigned int inputCount() const;
	int width() const;
	int minWidth() const;
	int maxWidth() const;
	int height() const;
	int minHeight() const;
	int maxHeight() const;
	int setSize( int newwidth, int newheight);

	pixel_format setPixelFormat(pixel_format newformat);
	/**
	 * Utility conversion routines
	 */
	int pixelFormatCode(pixel_format pixelformat) const;
	pixel_format pixelFormatForPalette( int palette ) const;
	int pixelFormatDepth(pixel_format pixelformat) const;
	QString pixelFormatName(pixel_format pixelformat) const;
	QString pixelFormatName(int pixelformat) const;

	__u64 signalStandardCode(signal_standard standard);
	QString signalStandardName(signal_standard standard);
	QString signalStandardName(int standard);
	int detectSignalStandards();

	int currentInput() const;
	int selectInput(int input);
	/**
	 * Apply the current input's image settings (brightness, contrast,
	 * saturation, whiteness, hue) to the hardware
	 */
	int setInputParameters();
	int startCapturing();
	int getFrame();
	int getImage(QImage *qimage);
	int getPreviewImage(QImage *qimage);
	int stopCapturing();
	int close();

	/**
	 * Store image settings for the current input
	 */
	float brightness() const;
	void setBrightness(float brightness);
	float contrast() const;
	void setContrast(float contrast);
	float saturation() const;
	void setSaturation(float saturation);
	float whiteness() const;
	void setWhiteness(float whiteness);
	float hue() const;
	void setHue(float Hue);

	/**
	 * Image controls for the device
	 */
	bool autoBrightnessContrast() const;
	void setAutoBrightnessContrast(bool brightnesscontrast);
	bool autoColorCorrection() const;
	void setAutoColorCorrection(bool colorcorrection);
	bool imageAsMirror() const;
	void setImageAsMirror(bool imageasmirror);
	/**
	 * User preferences for the device
	 */
	bool mmapDisabled() const;
	void setDisableMMap(bool disablemmap);

	/**
	 * Device capabilities
	 */
	bool canCapture();
	bool canChromakey();
	bool canScale();
	bool canOverlay();
	bool canRead();
	bool canAsyncIO();
	bool canStream();

	QValueVector<Kopete::AV::VideoInput> inputs() const; // Candidate to be private
	size_t getModelIndex();
	void setModelIndex(size_t modelindex);

protected:
	int xioctl(int request, void *arg);
	int errnoReturn(const char* s);
	int initRead();
	int initMmap();
	int initUserptr();
private:
	// Defines  the number of a device when more than 1 device of a given model is present
	size_t m_modelindex;
	// Which AV API is in use: Video4Linux, Video4Linux2, or None.
	videodev_driver m_driver;
	// file descriptor in use
	int descriptor;

	// image settings
	int m_currentheight, m_currentwidth, m_minwidth, m_maxwidth, m_minheight, m_maxheight;
	// device settings
	unsigned int m_current_input;
	pixel_format m_pixelformat;

	// structs defining video api settings
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
	struct v4l2_capability V4L2_capabilities;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
//	struct v4l2_input m_input;
#endif
	struct video_capability V4L_capabilities;
	struct video_buffer V4L_videobuffer;
#endif	

	// I/O plumbing
	io_method m_io_method;
	QValueVector<rawbuffer> m_rawbuffers;
	// seems to be the number of buffers used for reads
	unsigned int m_streambuffers;
	imagebuffer m_currentbuffer;
	int m_buffer_size;

	//flags defining the device's capabilites
	bool m_videocapture;
	bool m_videochromakey;
	bool m_videoscale;
	bool m_videooverlay;
	bool m_videoread;
	bool m_videoasyncio;
	bool m_videostream;
	// name of the device returned by the driver
	QString m_name;
	// path to the device node
	QString m_devicePath;
	QString m_model;
	// array holding settings for each input
	QValueVector<Kopete::AV::VideoInput> m_inputs;
	// user preferences
	bool m_disablemmap;
};

}

}

#endif
