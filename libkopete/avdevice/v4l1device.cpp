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

#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <kdebug.h>

#include "videoinput.h"
#include "v4l1device.h"
#include "videodevice.h"

#include "bayer.h"
#include "sonix_compress.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

V4l1Device::V4l1Device()
//: new VideoDevice()
{
	kDebug() << "Create a V4L1 video device.";
}


V4l1Device::~V4l1Device()
{
}

int V4l1Device::checkDevice()
{
	kDebug() << "checkDevice() called.";
	
	if(!isOpen())
	{
		kDebug() << "File is not open, stopping";
		return EXIT_FAILURE;
	}

	m_videocapture=false;
	m_videochromakey=false;
	m_videoscale=false;
	m_videooverlay=false;
	m_videoread=false;
	m_videoasyncio=false;
	m_videostream=false;
	
#if defined(__linux__) && defined(ENABLE_AV)
	CLEAR(V4L_capabilities);

	kDebug() << "checkDevice(): " << full_filename << " Trying V4L API.";
	if (-1 == xioctl (VIDIOCGCAP, &V4L_capabilities))
	{
		perror ("ioctl (VIDIOCGCAP)");
		return EXIT_FAILURE;
	}
	else
	{
		kDebug() << full_filename << " is a V4L device.";
		
		m_model=QString::fromLocal8Bit((const char*)V4L_capabilities.name);
		
		if(V4L_capabilities.type & VID_TYPE_CAPTURE)
			m_videocapture=true;
		if(V4L_capabilities.type & VID_TYPE_CHROMAKEY)
			m_videochromakey=true;
		if(V4L_capabilities.type & VID_TYPE_SCALES)
			m_videoscale=true;	
		if(V4L_capabilities.type & VID_TYPE_OVERLAY)
			m_videooverlay=true;
		m_frameSizes << QSize(V4L_capabilities.minwidth, V4L_capabilities.minheight)
			     << QSize(V4L_capabilities.maxwidth, V4L_capabilities.maxheight);

		int inputisok=EXIT_SUCCESS;
		m_input.clear();
		for(int loop=0; loop < V4L_capabilities.channels; loop++)
		{
			struct video_channel videoinput;
			CLEAR(videoinput);
			videoinput.channel = loop;
			videoinput.norm    = 1;
			inputisok = xioctl(VIDIOCGCHAN, &videoinput);
			if(inputisok == EXIT_SUCCESS)
			{
				VideoInput tempinput;
				tempinput.name = QString::fromLocal8Bit((const char*)videoinput.name);
				tempinput.hastuner=videoinput.flags & VIDEO_VC_TUNER;
// TODO The routine to detect the appropriate video standards for V4L must be placed here
				m_input.push_back(tempinput);
			}
		}
	}
#endif
	m_name = m_model; // Take care about changing the name to be different from the model itself...
	//FIXME:This comment is the opposit of the affection.

	kDebug() << "checkDevice() exited successfuly.";

	return EXIT_SUCCESS;
}


/*!
    \fn V4l1DevicePool::initDevice()
 */
int V4l1Device::initDevice()
{
	kDebug() << "initDevice() started";
	if(!isOpen())
	{
		kDebug() << "Device is not open";
		return EXIT_FAILURE;
	}
	
	m_io_method = IO_METHOD_NONE;

#if defined(__linux__) && defined(ENABLE_AV)
	m_videoread=true;
	m_io_method=IO_METHOD_READ;
	if(-1 != xioctl(VIDIOCGFBUF,&V4L_videobuffer))
	{
		kDebug() << "    Streaming interface";
	}
#endif

// Select video input, video standard and tune here.
	showDeviceCapabilities();
	kDebug() << "Exited successfuly";
	return EXIT_SUCCESS;
}

int V4l1Device::setSize(QSize newSize)
{
	//TODO:How should it work here ?
	//	We must set the format on the current pixel format.
	//	The current pixel format can be changed with setPixelFormat().
	//FIXME:
	//	If the size we want to set for a particular pixel format is not supported,
	//	the closer size will be set instead by the driver.
	
	kDebug() << "setting size" << newSize;
	if(!isOpen())
	{
		kDebug() << "The device is not opened.";
		return EXIT_FAILURE;
	}

	if (newSize.width() > m_frameSizes.last().width())
		newSize.setWidth(m_frameSizes.last().width());
	if (newSize.height() > m_frameSizes.last().height())
		newSize.setHeight(m_frameSizes.last().height());
	
	if (newSize.width() < m_frameSizes.first().width())
		newSize.setWidth(m_frameSizes.first().width());
	if (newSize.height() < m_frameSizes.first().height())
		newSize.setHeight(m_frameSizes.first().height());

	currentFrameSize = newSize;

// Change frame size for the video device
#if defined(__linux__) && defined(ENABLE_AV)
	struct video_window V4L_videowindow;

	if (xioctl (VIDIOCGWIN, &V4L_videowindow)== -1)
	{
		perror ("ioctl VIDIOCGWIN");
	//	return (NULL);
	}
	V4L_videowindow.width  = frameSize().width();
	V4L_videowindow.height = frameSize().height();
	V4L_videowindow.clipcount = 0;
	if (xioctl (VIDIOCSWIN, &V4L_videowindow)== -1)
	{
		perror ("ioctl VIDIOCSWIN");
	}

	kDebug() << "------------- width: " << V4L_videowindow.width << " Height: " << V4L_videowindow.height << " Clipcount: " << V4L_videowindow.clipcount << " -----------------";
#endif
	m_buffer_size = frameSize().width() * frameSize().height() * pixelFormatDepth(m_pixelformat) / 8;
	kDebug() << "------------------------- ------- -- m_buffer_size: " << m_buffer_size << " !!! -- ------- -----------------------------------------";

	m_currentbuffer.pixelformat=m_pixelformat;
	m_currentbuffer.data.resize(m_buffer_size);

	switch (m_io_method)
	{
		case IO_METHOD_NONE:                    break;
		case IO_METHOD_READ:    initRead ();    break;
		case IO_METHOD_MMAP:    initMmap ();    break;
		case IO_METHOD_USERPTR: initUserptr (); break;
	}

	kDebug() << "exited successfuly.";

	return EXIT_SUCCESS;
}

int V4l1Device::detectPixelFormats()
{
	if (!isOpen())
		return EXIT_FAILURE;
	kDebug() << "Supported pixel formats:";
	if(0 != setPixelFormat(V4L2_PIX_FMT_RGB332))	kDebug() << pixelFormatName(V4L2_PIX_FMT_RGB332);
	if(0 != setPixelFormat(V4L2_PIX_FMT_RGB444))	kDebug() << pixelFormatName(V4L2_PIX_FMT_RGB444);
	if(0 != setPixelFormat(V4L2_PIX_FMT_RGB555))	kDebug() << pixelFormatName(V4L2_PIX_FMT_RGB555);
	if(0 != setPixelFormat(V4L2_PIX_FMT_RGB565))	kDebug() << pixelFormatName(V4L2_PIX_FMT_RGB565);
	if(0 != setPixelFormat(V4L2_PIX_FMT_RGB555X))	kDebug() << pixelFormatName(V4L2_PIX_FMT_RGB555X);
	if(0 != setPixelFormat(V4L2_PIX_FMT_RGB565X))	kDebug() << pixelFormatName(V4L2_PIX_FMT_RGB565X);
	if(0 != setPixelFormat(V4L2_PIX_FMT_BGR24))	kDebug() << pixelFormatName(V4L2_PIX_FMT_BGR24);
	if(0 != setPixelFormat(V4L2_PIX_FMT_RGB24))	kDebug() << pixelFormatName(V4L2_PIX_FMT_RGB24);
	if(0 != setPixelFormat(V4L2_PIX_FMT_BGR32))	kDebug() << pixelFormatName(V4L2_PIX_FMT_BGR32);
	if(0 != setPixelFormat(V4L2_PIX_FMT_RGB32))	kDebug() << pixelFormatName(V4L2_PIX_FMT_RGB32);

// Bayer RGB format
	if(0 != setPixelFormat(V4L2_PIX_FMT_SBGGR8))	kDebug() << pixelFormatName(V4L2_PIX_FMT_SBGGR8);

// YUV formats
	if(0 != setPixelFormat(V4L2_PIX_FMT_GREY))	kDebug() << pixelFormatName(V4L2_PIX_FMT_GREY);
	if(0 != setPixelFormat(V4L2_PIX_FMT_YUYV))	kDebug() << pixelFormatName(V4L2_PIX_FMT_YUYV);
	if(0 != setPixelFormat(V4L2_PIX_FMT_UYVY))	kDebug() << pixelFormatName(V4L2_PIX_FMT_UYVY);
	if(0 != setPixelFormat(V4L2_PIX_FMT_YUV420))	kDebug() << pixelFormatName(V4L2_PIX_FMT_YUV420);
	if(0 != setPixelFormat(V4L2_PIX_FMT_YUV422P))	kDebug() << pixelFormatName(V4L2_PIX_FMT_YUV422P);

// Compressed formats
	if(0 != setPixelFormat(V4L2_PIX_FMT_JPEG))	kDebug() << pixelFormatName(V4L2_PIX_FMT_JPEG);
	if(0 != setPixelFormat(V4L2_PIX_FMT_MPEG))	kDebug() << pixelFormatName(V4L2_PIX_FMT_MPEG);

// Reserved formats
	if(0 != setPixelFormat(V4L2_PIX_FMT_DV))		kDebug() << pixelFormatName(V4L2_PIX_FMT_DV);
	if(0 != setPixelFormat(V4L2_PIX_FMT_ET61X251))	kDebug() << pixelFormatName(V4L2_PIX_FMT_ET61X251);
	if(0 != setPixelFormat(V4L2_PIX_FMT_HI240))	kDebug() << pixelFormatName(V4L2_PIX_FMT_HI240);
	if(0 != setPixelFormat(V4L2_PIX_FMT_HM12))	kDebug() << pixelFormatName(V4L2_PIX_FMT_HM12);
	if(0 != setPixelFormat(V4L2_PIX_FMT_MJPEG))	kDebug() << pixelFormatName(V4L2_PIX_FMT_MJPEG);
	if(0 != setPixelFormat(V4L2_PIX_FMT_PWC1))	kDebug() << pixelFormatName(V4L2_PIX_FMT_PWC1);
	if(0 != setPixelFormat(V4L2_PIX_FMT_PWC2))	kDebug() << pixelFormatName(V4L2_PIX_FMT_PWC2);
	if(0 != setPixelFormat(V4L2_PIX_FMT_SN9C10X))	kDebug() << pixelFormatName(V4L2_PIX_FMT_SN9C10X);
	if(0 != setPixelFormat(V4L2_PIX_FMT_WNVA))	kDebug() << pixelFormatName(V4L2_PIX_FMT_WNVA);
	if(0 != setPixelFormat(V4L2_PIX_FMT_YYUV))	kDebug() << pixelFormatName(V4L2_PIX_FMT_YYUV);

	return EXIT_SUCCESS;
}

unsigned int V4l1Device::setPixelFormat(unsigned int newformat)
{
	// Change the pixel format for the video device
	int ret = 0;


	kDebug() << "Setting new pixel format :" << pixelFormatName(newformat);
#if defined(__linux__) && defined(ENABLE_AV)
	struct video_picture V4L_picture;
	if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
		kDebug() << "VIDIOCGPICT failed (" << errno << ").";
	V4L_picture.palette = newformat;
	V4L_picture.depth   = pixelFormatDepth(newformat);
	if(-1 == xioctl(VIDIOCSPICT, &V4L_picture))
	{
		kDebug() << "error setting new format.";
	}

	if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
		kDebug() << "VIDIOCGPICT failed (" << errno << ").";

	//kDebug() << "V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth;
	m_pixelformat = V4L_picture.palette;
	if (m_pixelformat == newformat)
		ret = newformat;

#endif
	return ret;
}


/*!
    \fn Kopete::AV::V4l1Device::selectInput(int input)
 */
int V4l1Device::selectInput(int newinput)
{
	if (m_current_input >= inputs() || !isOpen())
		return EXIT_FAILURE;

#if defined(__linux__) && defined(ENABLE_AV)
	struct video_channel V4L_input;
	V4L_input.channel=newinput;
	V4L_input.norm = 4; // Hey, it's plain wrong! It should be input's signal standard!
	if (-1 == ioctl (descriptor, VIDIOCSCHAN, &V4L_input))
	{
		perror ("ioctl (VIDIOCSCHAN)");
		return EXIT_FAILURE;
	}
#endif
	kDebug() << "Selected input " << newinput << " (" << m_input[newinput].name << ")";
	
	m_current_input = newinput;
	
	setInputParameters();

	return EXIT_SUCCESS;
}

/*!
    \fn V4l1Device::startCapturing()
 */
int V4l1Device::startCapturing()
{
	kDebug() << "called.";
	
	if(isOpen() && m_io_method == IO_METHOD_READ)
		return EXIT_SUCCESS;
	
	return EXIT_FAILURE;
}

/*!
    \fn V4l1Device::getFrame()
 */
int V4l1Device::getFrame()
{
	ssize_t bytesread;

	if(!isOpen())
	{
		return EXIT_FAILURE;
	}
	switch (m_io_method)
	{
		case IO_METHOD_READ:
// 			kDebug() << "Using IO_METHOD_READ.File descriptor: " << descriptor << " Buffer address: " << &m_currentbuffer.data[0] << " Size: " << m_currentbuffer.data.size();
			if (m_currentbuffer.data.isEmpty())
				return EXIT_FAILURE;

			bytesread = read (descriptor, &m_currentbuffer.data[0], m_currentbuffer.data.size());
			if (-1 == bytesread) // must verify this point with ov511 driver.
			{
				kDebug() << "IO_METHOD_READ failed.";
				switch (errno)
				{
					case EAGAIN:
						return EXIT_FAILURE;
					case EIO: /* Could ignore EIO, see spec. fall through */
					default:
					return errnoReturn ("read");
				}
			}
			if((int)m_currentbuffer.data.size() < bytesread)
			{
				kDebug() << "IO_METHOD_READ returned less bytes (" << bytesread << ") than it was asked for (" << m_currentbuffer.data.size() <<").";
			}
			break;
		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
		case IO_METHOD_NONE:
		default :
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


/*!
    \fn V4l1Device::stopCapturing()
 */
int V4l1Device::stopCapturing()
{
	// Nothing to do here, the read IO method cannot be stopped as it is not started.
	// Simply stop reading.
	return EXIT_SUCCESS;
}

float V4l1Device::setBrightness(float brightness)
{
	kDebug() << "(" << brightness << ") called.";
	m_input[m_current_input].setBrightness(brightness); // Just to check bounds

#if defined(__linux__) && defined(ENABLE_AV)
		struct video_picture V4L_picture;
		if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
			kDebug() << "VIDIOCGPICT failed (" << errno << ").";
		V4L_picture.brightness = uint(65535 * getBrightness());
		if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
			kDebug() << "Device seems to not support adjusting image brightness. Fallback to it is not yet implemented.";
#endif
	return getBrightness();
}

float V4l1Device::setContrast(float contrast)
{
	kDebug() << "(" << contrast << ") called.";
	m_input[m_current_input].setContrast(contrast); // Just to check bounds

#if defined(__linux__) && defined(ENABLE_AV)
		struct video_picture V4L_picture;
		if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
			kDebug() << "VIDIOCGPICT failed (" << errno << ").";
		V4L_picture.contrast = uint(65535*getContrast());
		if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
			kDebug() << "Device seems to not support adjusting image contrast. Fallback to it is not yet implemented.";
#endif
	return getContrast();
}

float V4l1Device::setSaturation(float saturation)
{
	kDebug() << "(" << saturation << ") called.";
	m_input[m_current_input].setSaturation(saturation); // Just to check bounds

#if defined(__linux__) && defined(ENABLE_AV)
		struct video_picture V4L_picture;
		if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
			kDebug() << "VIDIOCGPICT failed (" << errno << ").";
		V4L_picture.colour = uint(65535 * getSaturation());
		if(-1 == xioctl(VIDIOCSPICT, &V4L_picture))
			kDebug() << "Device seems to not support adjusting image saturation. Fallback to it is not yet implemented.";
#endif
	return getSaturation();
}

float V4l1Device::setWhiteness(float whiteness)
{
	kDebug() << "(" << whiteness << ") called.";
	m_input[m_current_input].setWhiteness(whiteness); // Just to check bounds

#if defined(__linux__) && defined(ENABLE_AV)
		struct video_picture V4L_picture;
		if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
			kDebug() << "VIDIOCGPICT failed (" << errno << ").";
		V4L_picture.whiteness = uint(65535*getWhiteness());
		if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
			kDebug() << "Device seems to not support adjusting white level. Fallback to it is not yet implemented.";
#endif
	return getWhiteness();
}

float V4l1Device::setHue(float hue)
{
	kDebug() << "(" << hue << ") called.";
	m_input[m_current_input].setHue(hue); // Just to check bounds

#if defined(__linux__) && defined(ENABLE_AV)
		struct video_picture V4L_picture;
		if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
			kDebug() << "VIDIOCGPICT failed (" << errno << ").";
		V4L_picture.hue = uint(65535 * getHue());
		if(-1 == xioctl(VIDIOCSPICT, &V4L_picture))
			kDebug() << "Device seems to not support adjusting image hue. Fallback to it is not yet implemented.";
#endif
	return getHue();
}

int V4l1Device::detectSignalStandards()
{
	//Unneeded for v4l1
	return EXIT_SUCCESS;
}

/*!
    \fn V4l1Device::initRead()
 */
int V4l1Device::initRead()
{
	kDebug() << "called.";
	if(isOpen())
	{
		m_rawbuffers.resize(1);
		if (m_rawbuffers.size()==0)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}
		kDebug() << "m_buffer_size: " << m_buffer_size;

//		m_rawbuffers[0].pixelformat=m_pixelformat;
		m_rawbuffers[0].length = m_buffer_size;
		m_rawbuffers[0].start = (uchar *)malloc (m_buffer_size);

		if (!m_rawbuffers[0].start)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}
		kDebug() << "exited successfuly.";
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/*!
    \fn V4l1Device::initMmap()
 */
//TODO:To be removed...
int V4l1Device::initMmap()
{
	return EXIT_FAILURE;
}

/*!
    \fn V4l1Device::initUserptr()
 */
//TODO:To be removed...
int V4l1Device::initUserptr()
{
	return EXIT_FAILURE;
}

}

}
