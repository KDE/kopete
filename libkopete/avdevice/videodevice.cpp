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
#define HAVE_V4L2

#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <kdebug.h>

#include "videoinput.h"
#include "videodevice.h"

#include "videocodec.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

VideoDevice::VideoDevice() : m_currentheight( 0 ), m_currentwidth( 0 ), m_maxwidth( 0 ), m_minwidth( 0 ), m_maxheight( 0 ), m_minheight( 0 )
{
//	kdDebug( 14010 ) << "libkopete (avdevice): VideoDevice() called" << endl;
	descriptor = -1;
	m_streambuffers  = 0;
	m_current_input = 0;
	m_disablemmap = false;
//	kdDebug( 14010 ) << "libkopete (avdevice): VideoDevice() exited successfuly" << endl;
}

VideoDevice::~VideoDevice()
{
}

int VideoDevice::xioctl(int request, void *arg)
{
	int r;

	do r = ioctl (descriptor, request, arg);
	while (-1 == r && EINTR == errno);
	return r;
}

int VideoDevice::errnoReturn(const char* s)
{
	fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
	return EXIT_FAILURE;
}

void VideoDevice::setDevicePath(const QString & filename)
{
	m_devicePath=filename;
}

QString VideoDevice::devicePath() const
{
	return m_devicePath;
}

int VideoDevice::open()
{
	kdDebug( 14010 ) <<  k_funcinfo << "called" << endl;
	if(-1 != descriptor)
	{
		kdDebug( 14010 ) <<  k_funcinfo << "Device is already open" << endl;
		return EXIT_SUCCESS;
	}
	kdDebug( 14010 ) <<  k_funcinfo << "Opening file descriptor " << devicePath() << endl;

	descriptor = ::open (QFile::encodeName(m_devicePath), O_RDWR, 0);
	if(!isOpen())
	{
		kdDebug( 14010 ) << k_funcinfo << "Unable to open file " << m_devicePath << "Err: "<< errno << endl;
		return EXIT_FAILURE;
	}
	kdDebug( 14010 ) <<  k_funcinfo << "File " << m_devicePath << " was opened successfuly" << endl;
	if(EXIT_FAILURE==checkDevice())
	{
		kdDebug( 14010 ) <<  k_funcinfo << "File " << m_devicePath << " could not be opened" << endl;
		close();
		return EXIT_FAILURE;
	}

	initDevice();
	selectInput(m_current_input);
	kdDebug( 14010 ) <<  k_funcinfo << "exited successfuly" << endl;
	return EXIT_SUCCESS;
}

bool VideoDevice::isOpen() const
{
//		kdDebug( 14010 ) <<  k_funcinfo << "VideoDevice::isOpen() File " << (-1 == descriptor ? "is not" : "is" ) << " open" << endl;
	return (-1 != descriptor);
}


int VideoDevice::checkDevice()
{
	checkResolution();
	checkInputs();
}

int VideoDevice::checkResolution()
{
	kdDebug( 14010 ) <<  k_funcinfo << "checkDevice() called." << endl;
	if(isOpen())
	{
		m_videocapture=false;
		m_videochromakey=false;
		m_videoscale=false;
		m_videooverlay=false;
		m_videoread=false;
		m_videoasyncio=false;
		m_videostream=false;

		m_driver=VIDEODEV_DRIVER_NONE;
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		CLEAR(V4L2_capabilities);

		if (-1 != xioctl (VIDIOC_QUERYCAP, &V4L2_capabilities))
		{
			if (!(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE))
			{
				kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << m_devicePath << " is not a video capture device." << endl;
				m_driver = VIDEODEV_DRIVER_NONE;
				return EXIT_FAILURE;
			}
			m_videocapture=true;
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << m_devicePath << " is a V4L2 device." << endl;
			m_driver = VIDEODEV_DRIVER_V4L2;
			m_model=QString::fromLocal8Bit((const char*)V4L2_capabilities.card);

// It should not be there. It must remain in a completely distict place, cause this method should not change the pixelformat.
		if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_YUV420P))
		{
		  kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support YUV420P format. Trying MJPEG." << endl;
		  if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_MJPEG))
		    {
		      kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support YUYV format. Trying YUYV." << endl;
		      if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_YUYV))
			{
			  kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support MJPEG format. Trying RGB24." << endl;
			  if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_RGB24))
			    {
				kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support RGB24 format. Trying BGR24." << endl;
				if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_BGR24))
				{
					kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support RGB24 format. Trying RGB32." << endl;
					if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_RGB32))
					{
						kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support RGB32 format. Trying BGR32." << endl;
						if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_BGR32))
							kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support BGR32 format. Fallback to it is not yet implemented." << endl;
					}
				}
			    }
			}
		    }
		}

// Detect maximum and minimum resolution supported by the V4L2 device
			m_minwidth = 1;
			m_minheight = 1;
			m_maxheight = 32767;
			m_maxwidth = 32767;
			setSize(maxWidth(), maxHeight());
			m_maxwidth = width();
			m_maxheight = height();
			setSize(minWidth(), minHeight());
			m_minwidth = width();
			m_minheight = height();
		}
		else
		{
// V4L-only drivers should return an EINVAL in errno to indicate they cannot handle V4L2 calls. Not every driver is compliant, so
// it will try the V4L api even if the error code is different than expected.
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << m_devicePath << " is not a V4L2 device." << endl;
		}
#endif
		m_name=m_model; // Take care about changing the name to be different from the model itself...
// TODO: THis thing can be used to detec what pixel formats are supported in a API-independent way, but V4L2 has VIDIOC_ENUM_PIXFMT.
// The correct thing to do is to isolate these calls and do a proper implementation for V4L and another for V4L2 when this thing will be migrated to a plugin architecture.
		kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << "Supported pixel formats:" << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_GREY))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_GREY) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB332))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_RGB332) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB555))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_RGB555) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB555X))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_RGB555X) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB565))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_RGB565) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB565X))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_RGB565X) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB24))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_RGB24) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_BGR24))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_BGR24) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB32))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_RGB32) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_BGR32))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_BGR32) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_YUYV))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_YUYV) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_UYVY))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_UYVY) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_YUV422P))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_YUV422P) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_YUV420P))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_YUV420P) << endl;
		if(PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_MJPEG))
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << pixelFormatName(PIXELFORMAT_MJPEG) << endl;

// TODO: Now we must execute the proper initialization according to the type of the driver.
		kdDebug( 14010 ) <<  k_funcinfo << "checkDevice() exited successfuly." << endl;
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
#endif
	}
}

int VideoDevice::checkInputs()
{
	kdDebug( 14010 ) <<  k_funcinfo << "checkInputs() called." << endl;
	if(isOpen())
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		int inputisok=EXIT_SUCCESS;
		m_inputs.clear();
		for(unsigned int loop=0; inputisok==EXIT_SUCCESS; loop++)
		{
			struct v4l2_input videoinput;
			CLEAR(videoinput);
			videoinput.index = loop;
			inputisok=xioctl(VIDIOC_ENUMINPUT, &videoinput);
			if(inputisok==EXIT_SUCCESS)
			{
				VideoInput tempinput(
						QString::fromLocal8Bit((const char*)videoinput.name),
						videoinput.type & V4L2_INPUT_TYPE_TUNER,
						videoinput.std );
				m_inputs.push_back(tempinput);
				kdDebug( 14010 ) <<  k_funcinfo << "Input " << loop << ": " << tempinput.name() << " (tuner: " << ((videoinput.type & V4L2_INPUT_TYPE_TUNER) != 0) << ")" << endl;
#if 0
				if((videoinput.type & V4L2_INPUT_TYPE_TUNER) != 0)
				{
//					_tunerForInput[name] = desc.tuner;
//					_isTuner = true;
				}
				else
				{
//					_tunerForInput[name] = -1;
				}
#endif
			}
		}
#endif
#endif
	}
}
/*
		CLEAR(V4L_capabilities);

		if(m_driver==VIDEODEV_DRIVER_NONE)
		{
			kdDebug( 14010 ) <<  k_funcinfo << "checkDevice(): " << m_devicePath << " Trying V4L API." << endl;
			if (-1 == xioctl (VIDIOCGCAP, &V4L_capabilities))
			{
				perror ("ioctl (VIDIOCGCAP)");
				m_driver = VIDEODEV_DRIVER_NONE;
				return EXIT_FAILURE;
			}
			else
			{
				kdDebug( 14010 ) <<  k_funcinfo << m_devicePath << " is a V4L device." << endl;
				m_driver = VIDEODEV_DRIVER_V4L;
				m_model=QString::fromLocal8Bit((const char*)V4L_capabilities.name);
				if(V4L_capabilities.type & VID_TYPE_CAPTURE)
					m_videocapture=true;
				if(V4L_capabilities.type & VID_TYPE_CHROMAKEY)
					m_videochromakey=true;
				if(V4L_capabilities.type & VID_TYPE_SCALES)
					m_videoscale=true;	
				if(V4L_capabilities.type & VID_TYPE_OVERLAY)
					m_videooverlay=true;
//				kdDebug( 14010 ) << "libkopete (avdevice):     Inputs : " << V4L_capabilities.channels << endl;
//				kdDebug( 14010 ) << "libkopete (avdevice):     Audios : " << V4L_capabilities.audios << endl;
				m_minwidth  = V4L_capabilities.minwidth;
				m_maxwidth  = V4L_capabilities.maxwidth;
				m_minheight = V4L_capabilities.minheight;
				m_maxheight = V4L_capabilities.maxheight;

				fprintf(stderr, "checkDevice: V4L1 max %d x %d\n", m_maxwidth, m_maxheight); // AD: 


				int inputisok=EXIT_SUCCESS;
				m_inputs.clear();
				for(int loop=0; loop < V4L_capabilities.channels; loop++)
				{
					struct video_channel videoinput;
					CLEAR(videoinput);
					videoinput.channel = loop;
					videoinput.norm    = 1;
					inputisok=xioctl(VIDIOCGCHAN, &videoinput);
					if(inputisok==EXIT_SUCCESS)
					{
						VideoInput tempinput(
								QString::fromLocal8Bit((const char*)videoinput.name),
								videoinput.flags & VIDEO_VC_TUNER,
								0 );// TODO: FIXME check this is correct
// TODO: The routine to detect the appropriate video standards for V4L must be placed here
						m_inputs.push_back(tempinput);
//						kdDebug( 14010 ) << "libkopete (avdevice): Input " << loop << ": " << tempinput.name << " (tuner: " << ((videoinput.flags & VIDEO_VC_TUNER) != 0) << ")" << endl;
						if((input.type & V4L2_INPUT_TYPE_TUNER) != 0)
						{
//							_tunerForInput[name] = desc.tuner;
//							_isTuner = true;
						}
						else
						{
//							_tunerForInput[name] = -1;
						}
					}
				}

			}
		}
#endif
}
*/

int VideoDevice::showDeviceCapabilities()
{
	kdDebug( 14010 ) <<  k_funcinfo << "showDeviceCapabilities() called." << endl;
	if(isOpen())
	{
/*		kdDebug( 14010 ) << "libkopete (avdevice): Driver: " << (const char*)V4L2_capabilities.driver << " "
			<< ((V4L2_capabilities.version>>16) & 0xFF) << "."
			<< ((V4L2_capabilities.version>> 8) & 0xFF) << "."
			<< ((V4L2_capabilities.version    ) & 0xFF) << endl;
		kdDebug( 14010 ) << "libkopete (avdevice): Card: " << name << endl;
		kdDebug( 14010 ) << "libkopete (avdevice): Capabilities:" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE)
			kdDebug( 14010 ) << "libkopete (avdevice):     Video capture" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_OUTPUT)
			kdDebug( 14010 ) << "libkopete (avdevice):     Video output" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_OVERLAY)
			kdDebug( 14010 ) << "libkopete (avdevice):     Video overlay" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_VBI_CAPTURE)
			kdDebug( 14010 ) << "libkopete (avdevice):     VBI capture" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_VBI_OUTPUT)
			kdDebug( 14010 ) << "libkopete (avdevice):     VBI output" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_RDS_CAPTURE)
			kdDebug( 14010 ) << "libkopete (avdevice):     RDS capture" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_TUNER)
			kdDebug( 14010 ) << "libkopete (avdevice):     Tuner IO" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_AUDIO)
			kdDebug( 14010 ) << "libkopete (avdevice):     Audio IO" << endl;
;*/
		kdDebug( 14010 ) <<  k_funcinfo << "Card model: " << m_model << endl;
		kdDebug( 14010 ) <<  k_funcinfo << "Card name : " << m_name << endl;
		kdDebug( 14010 ) <<  k_funcinfo << "Capabilities:" << endl;
		if(canCapture())
			kdDebug( 14010 ) <<  k_funcinfo << "    Video capture" << endl;
		if(canRead())
			kdDebug( 14010 ) <<  k_funcinfo << "        Read" << endl;
		if(canAsyncIO())
			kdDebug( 14010 ) <<  k_funcinfo << "        Asynchronous input/output" << endl;
		if(canStream())
			kdDebug( 14010 ) <<  k_funcinfo << "        Streaming" << endl;
		if(canChromakey())
			kdDebug( 14010 ) <<  k_funcinfo << "    Video chromakey" << endl;
		if(canScale())
			kdDebug( 14010 ) <<  k_funcinfo << "    Video scales" << endl;
		if(canOverlay())
			kdDebug( 14010 ) <<  k_funcinfo << "    Video overlay" << endl;
//		kdDebug( 14010 ) << "libkopete (avdevice):     Audios : " << V4L_capabilities.audios << endl;
		kdDebug( 14010 ) <<  k_funcinfo << "    Max res: " << maxWidth() << " x " << maxHeight() << endl;
		kdDebug( 14010 ) <<  k_funcinfo << "    Min res: " << minWidth() << " x " << minHeight() << endl;
		kdDebug( 14010 ) <<  k_funcinfo << "    Inputs : " << inputCount() << endl;
		for (unsigned int loop=0; loop < inputCount(); loop++)
			kdDebug( 14010 ) <<  k_funcinfo << "Input " << loop << ": " << m_inputs[loop].name() << " (tuner: " << m_inputs[loop].hasTuner() << ")" << endl;
		kdDebug( 14010 ) <<  k_funcinfo << "showDeviceCapabilities() exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int VideoDevice::initDevice()
{
    /// @todo implement me
	//kdDebug( 14010 ) <<  k_funcinfo << "initDevice() started" << endl;
	if(-1 == descriptor)
	{
		//kdDebug( 14010 ) <<  k_funcinfo << "initDevice() Device is not open" << endl;
		return EXIT_FAILURE;
	}
	m_io_method = IO_METHOD_NONE;
	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			if(V4L2_capabilities.capabilities & V4L2_CAP_READWRITE)
			{
				m_videoread=true;
				m_io_method = IO_METHOD_READ;
				kdDebug( 14010 ) <<  k_funcinfo << "    Read/Write interface" << endl;
			}
			if(V4L2_capabilities.capabilities & V4L2_CAP_ASYNCIO)
			{
				m_videoasyncio=true;
				kdDebug( 14010 ) <<  k_funcinfo << "    Async IO interface" << endl;
			}
			if(V4L2_capabilities.capabilities & V4L2_CAP_STREAMING)
			{
				m_videostream=true;
				m_io_method = IO_METHOD_MMAP;
//				m_io_method = IO_METHOD_USERPTR;
				kdDebug( 14010 ) <<  k_funcinfo << "    Streaming interface" << endl;
			}
			if(m_io_method==IO_METHOD_NONE)
			{
				kdDebug( 14010 ) <<  k_funcinfo << "initDevice() Found no suitable input/output method for " << m_devicePath << endl;
				return EXIT_FAILURE;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			m_videoread=true;
			m_io_method=IO_METHOD_READ;
			if(-1 != xioctl(VIDIOCGFBUF,&V4L_videobuffer))
			{
//				m_videostream=true;
//				m_io_method = IO_METHOD_MMAP;
				kdDebug( 14010 ) <<  k_funcinfo << "    Streaming interface" << endl;
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:

			break;
	}

// Select video input, video standard and tune here.
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl (VIDIOC_CROPCAP, &cropcap))
	{ // Errors ignored.
	}
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c = cropcap.defrect; // reset to default
	if (-1 == xioctl (VIDIOC_S_CROP, &crop))
	{
		switch (errno)
		{
			case EINVAL: break;  // Cropping not supported.
			default:     break;  // Errors ignored.
		}
	}
#endif
#endif

	showDeviceCapabilities();
	kdDebug( 14010 ) <<  k_funcinfo << "initDevice() exited successfuly" << endl;
	return EXIT_SUCCESS;
}

unsigned int VideoDevice::inputCount() const
{
	return m_inputs.size();
}


int VideoDevice::width() const
{
	return m_currentwidth;
}

int VideoDevice::minWidth() const
{
	return m_minwidth;
}

int VideoDevice::maxWidth() const
{
	return m_maxwidth;
}

int VideoDevice::height() const
{
	return m_currentheight;
}

int VideoDevice::minHeight() const
{
	return m_minheight;
}

int VideoDevice::maxHeight() const
{
	return m_maxheight;
}

//This code was contained in setSize() just after thre isOpen() before. I'm removind it 
// It should not be there. It must remain in a completely distict place, cause this method should not change the pixelformat.
/*		if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_YUV420P))
		{
		  kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support YUV420P format. Trying MJPEG." << endl;
		  if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_MJPEG))
		    {
		      kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support YUYV format. Trying YUYV." << endl;
		      if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_YUYV))
			{
			  kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support MJPEG format. Trying RGB24." << endl;
			  if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_RGB24))
			    {
				kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support RGB24 format. Trying BGR24." << endl;
				if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_BGR24))
				{
					kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support RGB24 format. Trying RGB32." << endl;
					if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_RGB32))
					{
						kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support RGB32 format. Trying BGR32." << endl;
						if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_BGR32))
							kdDebug( 14010 ) <<  k_funcinfo << "Card doesn't seem to support BGR32 format. Fallback to it is not yet implemented." << endl;
					}
				}
			    }
			}
		    }
		}
*/
int VideoDevice::setSize( int newwidth, int newheight)
{
kdDebug( 14010 ) <<  k_funcinfo << "setSize(" << newwidth << ", " << newheight << ") called." << endl;
	if(isOpen())
	{
		newwidth = QMIN( newwidth, m_maxwidth );
		newheight = QMIN( newheight, m_maxheight );
		newwidth = QMAX( newwidth, m_minwidth );
		newheight = QMAX( newheight, m_minheight );

		m_currentwidth  = newwidth;
		m_currentheight = newheight;

//kdDebug( 14010 ) << k_funcinfo << "width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << width() << "x" << height() << endl;
// Change resolution for the video device
		switch(m_driver)
		{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
			case VIDEODEV_DRIVER_V4L2:
//				CLEAR (fmt);
				if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
					kdDebug( 14010 ) << k_funcinfo << "VIDIOC_G_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
				fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				fmt.fmt.pix.width       = width();
				fmt.fmt.pix.height      = height();
				fmt.fmt.pix.field       = V4L2_FIELD_ANY;
				if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
				{
					kdDebug( 14010 ) << k_funcinfo << "VIDIOC_S_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
					// Note VIDIOC_S_FMT may change width and height.
				}
				else
				{
// Buggy driver paranoia.
kdDebug( 14010 ) << k_funcinfo << "VIDIOC_S_FMT worked (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
					unsigned int min = fmt.fmt.pix.width * 2;
					if (fmt.fmt.pix.bytesperline < min)
						fmt.fmt.pix.bytesperline = min;
					min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
					if (fmt.fmt.pix.sizeimage < min)
						fmt.fmt.pix.sizeimage = min;
					m_buffer_size=fmt.fmt.pix.sizeimage ;
				}
				break;
#endif
			case VIDEODEV_DRIVER_V4L:
				{
					struct video_window V4L_videowindow;

kdDebug( 14010 ) << "------------- width: " << V4L_videowindow.width << " Height: " << V4L_videowindow.height << " Clipcount: " << V4L_videowindow.clipcount << " -----------------" << endl;

				if (xioctl (VIDIOCGWIN, &V4L_videowindow)== -1)
				{
					perror ("ioctl VIDIOCGWIN");
//					return (NULL);
				}
				V4L_videowindow.width  = width();
				V4L_videowindow.height = height();
				V4L_videowindow.clipcount=0;
				if (xioctl (VIDIOCSWIN, &V4L_videowindow)== -1)
				{
					perror ("ioctl VIDIOCSWIN");
//					return (NULL);
				}
kdDebug( 14010 ) << "------------- width: " << V4L_videowindow.width << " Height: " << V4L_videowindow.height << " Clipcount: " << V4L_videowindow.clipcount << " -----------------" << endl;

//				kdDebug( 14010 ) << "libkopete (avdevice): V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth << endl;

/*				if(-1 == xioctl(VIDIOCGFBUF,&V4L_videobuffer))
					kdDebug( 14010 ) << "libkopete (avdevice): VIDIOCGFBUF failed (" << errno << "): Card cannot stream" << endl;*/

				}
				break;
#endif
			case VIDEODEV_DRIVER_NONE:
			default:
				break;
		}
		m_buffer_size = width() * height() * pixelFormatDepth(m_pixelformat) / 8;
kdDebug( 14010 ) << "------------------------- ------- -- m_buffer_size: " << m_buffer_size << " !!! -- ------- -----------------------------------------" << endl;

		m_currentbuffer.pixelformat=m_pixelformat;
		m_currentbuffer.data.resize(m_buffer_size);

		switch (m_io_method)
		{
			case IO_METHOD_NONE:                    break;
			case IO_METHOD_READ:    initRead ();    break;
			case IO_METHOD_MMAP:    initMmap ();    break;
			case IO_METHOD_USERPTR: initUserptr (); break;
		}

kdDebug( 14010 ) <<  k_funcinfo << "setSize(" << newwidth << ", " << newheight << ") exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
kdDebug( 14010 ) <<  k_funcinfo << "setSize(" << newwidth << ", " << newheight << ") Device is not open." << endl;
	return EXIT_FAILURE;
}

pixel_format VideoDevice::setPixelFormat(pixel_format newformat)
{
	pixel_format ret = PIXELFORMAT_NONE;
//kdDebug( 14010 ) <<  k_funcinfo << "called." << endl;
// Change the pixel format for the video device
	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
//			CLEAR (fmt);
			if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
                        {
//				return errnoReturn ("VIDIOC_S_FMT");
//				kdDebug( 14010 ) << k_funcinfo << "VIDIOC_G_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
			}
			else
				m_pixelformat = pixelFormatForPalette(fmt.fmt.pix.pixelformat);
		
			fmt.fmt.pix.pixelformat = pixelFormatCode(newformat);
			if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
			{
//				kdDebug( 14010 ) << k_funcinfo << "VIDIOC_S_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
			}
			else
			{
				m_pixelformat = newformat;
				ret = m_pixelformat;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			{
			struct video_picture V4L_picture;
			if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
				kdDebug( 14010 ) <<  k_funcinfo << "VIDIOCGPICT failed (" << errno << ")." << endl;
//			kdDebug( 14010 ) <<  k_funcinfo << "V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth << endl;
			V4L_picture.palette = pixelFormatCode(newformat);
			V4L_picture.depth   = pixelFormatDepth(newformat);
			if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
			{
//				kdDebug( 14010 ) <<  k_funcinfo << "Card seems to not support " << pixelFormatName(newformat) << " format. Fallback to it is not yet implemented." << endl;
			}

			if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
				kdDebug( 14010 ) <<  k_funcinfo << "VIDIOCGPICT failed (" << errno << ")." << endl;

//			kdDebug( 14010 ) <<  k_funcinfo << "V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth << endl;
			m_pixelformat=pixelFormatForPalette(V4L_picture.palette);
			if (m_pixelformat == newformat)
				ret = newformat;

			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			break;
	}
	return ret;
}

int VideoDevice::currentInput() const
{
	if(isOpen())
	{
		return m_current_input;
	}
	return 0;
}

int VideoDevice::selectInput(int newinput)
{
	if(m_current_input >= inputCount())
		return EXIT_FAILURE;

	if(isOpen())
	{
		switch (m_driver)
		{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
			case VIDEODEV_DRIVER_V4L2:
				if (-1 == ioctl (descriptor, VIDIOC_S_INPUT, &newinput))
				{
					perror ("VIDIOC_S_INPUT");
					return EXIT_FAILURE;
				}
				break;
#endif
			case VIDEODEV_DRIVER_V4L:
				struct video_channel V4L_input;
				V4L_input.channel=newinput;
				V4L_input.norm=4; // Hey, it's plain wrong! It should be input's signal standard!
				if (-1 == ioctl (descriptor, VIDIOCSCHAN, &V4L_input))
				{
					perror ("ioctl (VIDIOCSCHAN)");
					return EXIT_FAILURE;
				}
				break;
#endif
			case VIDEODEV_DRIVER_NONE:
			default:
				break;
		}
		kdDebug( 14010 ) <<  k_funcinfo << "Selected input " << newinput << " (" << m_inputs[newinput].name() << ")" << endl;
		m_current_input = newinput;
		setInputParameters();
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int VideoDevice::setInputParameters()
{
	if( (isOpen()) && (m_current_input < inputCount() ) )
	{
		setBrightness( brightness() );
		setContrast( contrast() );
		setSaturation( saturation() );
		setWhiteness( whiteness() );
		setHue( hue() );
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int VideoDevice::startCapturing()
{

	kdDebug( 14010 ) <<  k_funcinfo << "called." << endl;
	if(isOpen())
	{
		switch (m_io_method)
		{
			case IO_METHOD_NONE: // Card cannot capture frames
				return EXIT_FAILURE;
				break;
			case IO_METHOD_READ: // Nothing to do
				break;
			case IO_METHOD_MMAP:
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
				{
					unsigned int loop;
					for (loop = 0; loop < m_streambuffers; ++loop)
					{
						struct v4l2_buffer buf;
						CLEAR (buf);
						buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
						buf.memory = V4L2_MEMORY_MMAP;
						buf.index  = loop;
						if (-1 == xioctl (VIDIOC_QBUF, &buf))
							return errnoReturn ("VIDIOC_QBUF");
					}
					enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					if (-1 == xioctl (VIDIOC_STREAMON, &type))
						return errnoReturn ("VIDIOC_STREAMON");
				}
#endif
#endif
				break;
			case IO_METHOD_USERPTR:
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
				{
					unsigned int loop;
					for (loop = 0; loop < m_streambuffers; ++loop)
					{
						struct v4l2_buffer buf;
						CLEAR (buf);
						buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
						buf.memory    = V4L2_MEMORY_USERPTR;
						buf.m.userptr = (unsigned long) m_rawbuffers[loop].start;
						buf.length    = m_rawbuffers[loop].length;
						if (-1 == xioctl (VIDIOC_QBUF, &buf))
							return errnoReturn ("VIDIOC_QBUF");
					}
					enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					if (-1 == xioctl (VIDIOC_STREAMON, &type))
						return errnoReturn ("VIDIOC_STREAMON");
				}
#endif
#endif
				break;
		}

		kdDebug( 14010 ) <<  k_funcinfo << "exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int VideoDevice::getFrame()
{
	ssize_t bytesread;

#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
	struct v4l2_buffer v4l2buffer;
#endif
#endif
//	kdDebug( 14010 ) <<  k_funcinfo << "getFrame() called." << endl;
	if(isOpen())
	{
		switch (m_io_method)
		{
			case IO_METHOD_NONE: // Card cannot capture frames
				return EXIT_FAILURE;
				break;
			case IO_METHOD_READ:
//				kdDebug( 14010 ) <<  k_funcinfo << "Using IO_METHOD_READ.File descriptor: " << descriptor << " Buffer address: " << &m_currentbuffer.data[0] << " Size: " << m_currentbuffer.data.size() << endl;
				bytesread = read (descriptor, &m_currentbuffer.data[0], m_currentbuffer.data.size());
				if (-1 == bytesread) // must verify this point with ov511 driver.
				{
					kdDebug( 14010 ) <<  k_funcinfo << "IO_METHOD_READ failed." << endl;
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
					kdDebug( 14010 ) <<  k_funcinfo << "IO_METHOD_READ returned less bytes (" << bytesread << ") than it was asked for (" << m_currentbuffer.data.size() <<")." << endl;
				}
				break;
			case IO_METHOD_MMAP:
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
				CLEAR (v4l2buffer);
				v4l2buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				v4l2buffer.memory = V4L2_MEMORY_MMAP;
				if (-1 == xioctl (VIDIOC_DQBUF, &v4l2buffer))
				{
					kdDebug( 14010 ) <<  k_funcinfo << m_devicePath << " MMAPed getFrame failed." << endl;
					switch (errno)
					{
						case EAGAIN:
						{
							kdDebug( 14010 ) <<  k_funcinfo << m_devicePath << " MMAPed getFrame failed: EAGAIN. Pointer: " << endl;
							return EXIT_FAILURE;
						}
						case EIO: /* Could ignore EIO, see spec. fall through */
						default:
							return errnoReturn ("VIDIOC_DQBUF");
					}
				}
/*				if (v4l2buffer.index < m_streambuffers)
					return EXIT_FAILURE;*/ //it was an assert()
//kdDebug( 14010 ) << k_funcinfo << "m_rawbuffers[" << v4l2buffer.index << "].start: " << (void *)m_rawbuffers[v4l2buffer.index].start << "   Size: " << m_currentbuffer.data.size() << endl;



/*{
	unsigned long long result=0;
	unsigned long long R=0, G=0, B=0, A=0;
	int Rmax=0, Gmax=0, Bmax=0, Amax=0;
	int Rmin=255, Gmin=255, Bmin=255, Amin=0;

	for(unsigned int loop=0;loop < m_currentbuffer.data.size();loop+=4)
	{
		R+=m_rawbuffers[v4l2buffer.index].start[loop];
		G+=m_rawbuffers[v4l2buffer.index].start[loop+1];
		B+=m_rawbuffers[v4l2buffer.index].start[loop+2];
//		A+=currentbuffer.data[loop+3];
		if (m_currentbuffer.data[loop]   < Rmin) Rmin = m_currentbuffer.data[loop];
		if (m_currentbuffer.data[loop+1] < Gmin) Gmin = m_currentbuffer.data[loop+1];
		if (m_currentbuffer.data[loop+2] < Bmin) Bmin = m_currentbuffer.data[loop+2];
//		if (m_currentbuffer.data[loop+3] < Amin) Amin = m_currentbuffer.data[loop+3];
		if (m_currentbuffer.data[loop]   > Rmax) Rmax = m_currentbuffer.data[loop];
		if (m_currentbuffer.data[loop+1] > Gmax) Gmax = m_currentbuffer.data[loop+1];
		if (m_currentbuffer.data[loop+2] > Bmax) Bmax = m_currentbuffer.data[loop+2];
//		if (m_currentbuffer.data[loop+3] > Amax) Amax = m_currentbuffer.data[loop+3];
	}
	kdDebug( 14010 ) << " R: " << R << " G: " << G << " B: " << B << " A: " << A <<
		" Rmin: " << Rmin << " Gmin: " << Gmin << " Bmin: " << Bmin << " Amin: " << Amin <<
		" Rmax: " << Rmax << " Gmax: " << Gmax << " Bmax: " << Bmax << " Amax: " << Amax << endl;
}*/


memcpy(&m_currentbuffer.data[0], m_rawbuffers[v4l2buffer.index].start, m_currentbuffer.data.size());
				if (-1 == xioctl (VIDIOC_QBUF, &v4l2buffer))
					return errnoReturn ("VIDIOC_QBUF");
#endif
#endif
				break;
			case IO_METHOD_USERPTR:
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
				{
					unsigned int i;
					CLEAR (v4l2buffer);
					v4l2buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					v4l2buffer.memory = V4L2_MEMORY_USERPTR;
					if (-1 == xioctl (VIDIOC_DQBUF, &v4l2buffer))
					{
						switch (errno)
						{
							case EAGAIN:
								return EXIT_FAILURE;
							case EIO: /* Could ignore EIO, see spec. fall through */
							default:
								return errnoReturn ("VIDIOC_DQBUF");
						}
					}
					for (i = 0; i < m_streambuffers; ++i)
						if (v4l2buffer.m.userptr == (unsigned long) m_rawbuffers[i].start && v4l2buffer.length == m_rawbuffers[i].length)
							break;
					if (i < m_streambuffers)
						return EXIT_FAILURE;
					if (-1 == xioctl (VIDIOC_QBUF, &v4l2buffer))
					return errnoReturn ("VIDIOC_QBUF");
				}
#endif
#endif
				break;
		}

// Automatic color correction. Now it just swaps R and B channels in RGB24/BGR24 modes.
		if(m_inputs[m_current_input].autoColorCorrection())
		{
			switch(m_currentbuffer.pixelformat)
			{
				case PIXELFORMAT_NONE	: break;
				case PIXELFORMAT_GREY	: break;
				case PIXELFORMAT_RGB332	: break;
				case PIXELFORMAT_RGB555	: break;
				case PIXELFORMAT_RGB555X: break;
				case PIXELFORMAT_RGB565	: break;
				case PIXELFORMAT_RGB565X: break;
				case PIXELFORMAT_RGB24	:
				case PIXELFORMAT_BGR24	:
					{
						unsigned char temp;
						for(unsigned int loop=0;loop < m_currentbuffer.data.size();loop+=3)
						{
							temp = m_currentbuffer.data[loop];
							m_currentbuffer.data[loop] = m_currentbuffer.data[loop+2];
							m_currentbuffer.data[loop+2] = temp;
						}
					}
					break;
				case PIXELFORMAT_RGB32	:
				case PIXELFORMAT_BGR32	:
					{
						unsigned char temp;
						for(unsigned int loop=0;loop < m_currentbuffer.data.size();loop+=4)
						{
							temp = m_currentbuffer.data[loop];
							m_currentbuffer.data[loop] = m_currentbuffer.data[loop+2];
							m_currentbuffer.data[loop+2] = temp;
						}
					}
					break;
				case PIXELFORMAT_YUYV	: break;
				case PIXELFORMAT_UYVY	: break;
				case PIXELFORMAT_YUV420P: break;
				case PIXELFORMAT_YUV422P: break;
				case PIXELFORMAT_MJPEG:   break;
			}
		}
//kdDebug( 14010 ) <<  k_funcinfo << "10 Using IO_METHOD_READ.File descriptor: " << descriptor << " Buffer address: " << &m_currentbuffer.data[0] << " Size: " << m_currentbuffer.data.size() << endl;


// put frame copy operation here
//		kdDebug( 14010 ) <<  k_funcinfo << "exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int VideoDevice::getImage(QImage *qimage)
{
	qimage->create(width(), height(),32, QImage::IgnoreEndian);
	uchar *bits=qimage->bits();
//kdDebug( 14010 ) <<  k_funcinfo << "Capturing in " << pixelFormatName(m_currentbuffer.pixelformat) << endl;
	switch(m_currentbuffer.pixelformat)
	{
		case PIXELFORMAT_NONE	: break;
		case PIXELFORMAT_GREY	: break;
		case PIXELFORMAT_RGB332	: break;
		case PIXELFORMAT_RGB555	: break;
		case PIXELFORMAT_RGB555X: break;
		case PIXELFORMAT_RGB565	:
			{
				int step=0;
				for(int loop=0;loop < qimage->numBytes();loop+=4)
				{
					bits[loop] = (m_currentbuffer.data[step]<<3)+(m_currentbuffer.data[step]<<3>>5);
					bits[loop+1] = ((m_currentbuffer.data[step+1])<<5)|m_currentbuffer.data[step]>>5;
					bits[loop+2]   = ((m_currentbuffer.data[step+1])&248)+((m_currentbuffer.data[step+1])>>5);
					bits[loop+3] = 255;
					step+=2;
				}
			}
			break;
		case PIXELFORMAT_RGB565X: break;
		case PIXELFORMAT_RGB24	:
			{
				int step=0;
				for(int loop=0;loop < qimage->numBytes();loop+=4)
				{
					bits[loop]   = m_currentbuffer.data[step];
					bits[loop+1] = m_currentbuffer.data[step+1];
					bits[loop+2] = m_currentbuffer.data[step+2];
					bits[loop+3] = 255;
					step+=3;
				}
			}
			break;
		case PIXELFORMAT_BGR24	: break;
			{
				int step=0;
				for(int loop=0;loop < qimage->numBytes();loop+=4)
				{
					bits[loop]   = m_currentbuffer.data[step+2];
					bits[loop+1] = m_currentbuffer.data[step+1];
					bits[loop+2] = m_currentbuffer.data[step];
					bits[loop+3] = 255;
					step+=3;
				}
			}
			break;
		case PIXELFORMAT_RGB32	: memcpy(bits,&m_currentbuffer.data[0], m_currentbuffer.data.size());
			break;
		case PIXELFORMAT_BGR32	: break;
	
        	case PIXELFORMAT_MJPEG:
		  {
		    unsigned char*pic = NULL;
		    int w = 0, h = 0;
		    VideoCodec::jpeg_decode(&pic, &m_currentbuffer.data[0], &w, &h);
		    if(pic != NULL && w == m_currentwidth && h == m_currentheight)
		      {
			VideoCodec::yuyv2qimage(qimage, pic, w, h);
			free(pic);
		      }
		    else
		      {
			fprintf(stderr, "error in MJPEG decoding- expected: %d x %d; actual: %d x %d\n",
				m_currentwidth, m_currentheight, w, h);
		      }
		  }
		  break;

		case PIXELFORMAT_YUYV:
		  {
		    VideoCodec::yuyv2qimage(qimage, &m_currentbuffer.data[0], m_currentwidth, m_currentheight);
		  }
		  break;

		case PIXELFORMAT_UYVY:
		case PIXELFORMAT_YUV422P:
		case PIXELFORMAT_YUV420P:
		{
		uchar *yptr, *cbptr, *crptr;
		bool halfheight=false;
		bool packed=false;

		if (m_currentbuffer.pixelformat == PIXELFORMAT_YUV420P)
			halfheight=true;
	
		if (m_currentbuffer.pixelformat == PIXELFORMAT_YUYV)
		{
			yptr = &m_currentbuffer.data[0];
			cbptr = yptr + 1;
			crptr = yptr + 3;
			packed=true;
		}
		else if (m_currentbuffer.pixelformat == PIXELFORMAT_UYVY)
		{
			cbptr = &m_currentbuffer.data[0];
			yptr = cbptr + 1;
			crptr = cbptr + 3;
			packed=true;
		}
		else
		{
			yptr = &m_currentbuffer.data[0];
			cbptr = yptr + (width()*height());
			crptr = cbptr + (width()*height()/(halfheight ? 4:2));
		}
	
		for(int y=0; y<height(); y++)
		{
			for(int x=0; x<width(); x++)
			{
			int c,d,e;
	
			if (packed)
			{
				c = (yptr[x<<1])-16;
				d = (cbptr[x&!1])-128;
				e = (crptr[x&!1])-128;
			}
			else
			{
				c = (yptr[x])-16;
				d = (cbptr[x>>1])-128;
				e = (crptr[x>>1])-128;
			}
	
			int r = (298 * c           + 409 * e + 128)>>8;
			int g = (298 * c - 100 * d - 208 * e + 128)>>8;
			int b = (298 * c + 516 * d           + 128)>>8;
	
			if (r<0) r=0;   if (r>255) r=255;
			if (g<0) g=0;   if (g>255) g=255;
			if (b<0) b=0;   if (b>255) b=255;
	
			uint *p = (uint*)qimage->scanLine(y)+x;
			*p = qRgba(r,g,b,255);
	
			}
	
			if (packed)
			{
			yptr+=width()*2;
			cbptr+=width()*2;
			crptr+=width()*2;
			}
			else
			{
			yptr+=width();
			if (!halfheight || y&1)
			{
				cbptr+=width()/2;
				crptr+=width()/2;
			}
			}
		}
		}
		break;
	}

	return EXIT_SUCCESS;
}

int VideoDevice::getPreviewImage(QImage *qimage)
{
	if(imageAsMirror())
	{
		QImage img;
		getImage(&img);
		VideoCodec::computeMirrorImage(qimage, &img);
	}
	else
	{
		getImage(qimage);
	}
	return EXIT_SUCCESS;
}

int VideoDevice::stopCapturing()
{
    /// @todo implement me
	kdDebug( 14010 ) <<  k_funcinfo << "called." << endl;
	if(isOpen())
	{
		switch (m_io_method)
		{
			case IO_METHOD_NONE: // Card cannot capture frames
				return EXIT_FAILURE;
				break;
			case IO_METHOD_READ: // Nothing to do
				break;
			case IO_METHOD_MMAP:
			case IO_METHOD_USERPTR:
#ifdef HAVE_V4L2
				{
					enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					if (-1 == xioctl (VIDIOC_STREAMOFF, &type))
						return errnoReturn ("VIDIOC_STREAMOFF");

                    if (m_io_method == IO_METHOD_MMAP)
                    {
                        unsigned int loop;
                        for (loop = 0; loop < m_streambuffers; ++loop)
                        {
                            if (munmap(m_rawbuffers[loop].start,m_rawbuffers[loop].length) != 0)
                            {
                                kdDebug( 14010 ) <<  k_funcinfo << "unable to munmap." << endl;
                            }
                        }
                    }
				}
#endif
				break;
		}
		kdDebug( 14010 ) <<  k_funcinfo << "exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


int VideoDevice::close()
{
	kdDebug( 14010 ) <<  k_funcinfo << " called." << endl;
	if(isOpen())
	{
		kdDebug( 14010 ) << k_funcinfo << " Device is open. Trying to properly shutdown the device." << endl;
		stopCapturing();
		kdDebug( 14010 ) << k_funcinfo << "::close() returns " << ::close(descriptor) << endl;
	}
	descriptor = -1;
	return EXIT_SUCCESS;
}

float VideoDevice::brightness() const
{
  if (m_current_input < m_inputs.size() )
	return m_inputs[m_current_input].brightness();
  else
	return 0;
}

void VideoDevice::setBrightness(float newBrightness)
{
	kdDebug( 14010 ) <<  k_funcinfo << " called." << endl;
	m_inputs[m_current_input].setBrightness( newBrightness); // Just to check bounds

	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			{
				struct v4l2_control control;
				struct v4l2_queryctrl queryctrl;
				float value = brightness();
				CLEAR(queryctrl);
				queryctrl.id =  V4L2_CID_BRIGHTNESS;
				if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl)) {
					fprintf(stderr, "Brightness control not supported.\n");
				} else {
					CLEAR(control);
					control.id    = V4L2_CID_BRIGHTNESS;
					control.value = queryctrl.minimum + (queryctrl.maximum - queryctrl.minimum) * value;
					fprintf(stderr, "V4L2: setBrightness- value=%d\n", control.value);
					if (-1 == xioctl (VIDIOC_S_CTRL, &control)) {
						fprintf(stderr, "Brightness control not supported.\n");
					}
				}
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			{
				struct video_picture V4L_picture;
				if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
					kdDebug( 14010 ) <<  k_funcinfo << "VIDIOCGPICT failed (" << errno << ")." << endl;
				V4L_picture.brightness   = (65535*brightness());
				if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
					kdDebug( 14010 ) <<  k_funcinfo << "Card seems to not support adjusting image brightness. Fallback to it is not yet implemented." << endl;
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			break;
	}
}

float VideoDevice::contrast() const
{
  if (m_current_input < m_inputs.size() )
	return m_inputs[m_current_input].contrast();
  else
	return 0;
}

void VideoDevice::setContrast(float newContrast)
{
	kdDebug( 14010 ) <<  k_funcinfo << " called." << endl;
	m_inputs[m_current_input].setContrast(newContrast); // Just to check bounds

	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			{
				struct v4l2_control control;
				struct v4l2_queryctrl queryctrl;
				float value = contrast();
				CLEAR(queryctrl);
				queryctrl.id =  V4L2_CID_CONTRAST;
				if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl)) {
					fprintf(stderr, "Contrast control not supported.\n");
				} else {
					CLEAR(control);
					control.id    = V4L2_CID_CONTRAST;
					control.value = queryctrl.minimum + (queryctrl.maximum - queryctrl.minimum) * value;
					fprintf(stderr, "V4L2: setContrast- value=%d (%f) [%d:%d]\n", control.value, value, queryctrl.minimum, queryctrl.maximum);
					if (-1 == xioctl (VIDIOC_S_CTRL, &control)) {
						fprintf(stderr, "Contrast control not supported.\n");
					}
				}
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			{
				struct video_picture V4L_picture;
				if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
					kdDebug( 14010 ) <<  k_funcinfo << "VIDIOCGPICT failed (" << errno << ")." << endl;
				V4L_picture.contrast   = (65535*contrast());
				if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
					kdDebug( 14010 ) <<  k_funcinfo << "Card seems to not support adjusting image contrast. Fallback to it is not yet implemented." << endl;
			}
		break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			break;
	}
}

float VideoDevice::saturation() const
{
  if (m_current_input < m_inputs.size() )
	return m_inputs[m_current_input].saturation();
  else
	return 0;
}

void VideoDevice::setSaturation(float newSaturation)
{
	kdDebug( 14010 ) <<  k_funcinfo << " called." << endl;
	m_inputs[m_current_input].setSaturation(newSaturation); // Just to check bounds

	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			{
				struct v4l2_control control;
				struct v4l2_queryctrl queryctrl;
				float value = saturation();
				CLEAR(queryctrl);
				queryctrl.id =  V4L2_CID_SATURATION;
				if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl)) {
					fprintf(stderr, "Saturation control not supported.\n");
				} else {
					CLEAR(control);
					control.id    = V4L2_CID_SATURATION;
					control.value = queryctrl.minimum + (queryctrl.maximum - queryctrl.minimum) * value;
					fprintf(stderr, "V4L2: setSaturation- value=%d\n", control.value);
					if (-1 == xioctl (VIDIOC_S_CTRL, &control)) {
						fprintf(stderr, "Saturation control not supported.\n");
					}
				}
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			{
				struct video_picture V4L_picture;
				if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
					kdDebug( 14010 ) <<  k_funcinfo << "VIDIOCGPICT failed (" << errno << ")." << endl;
				V4L_picture.colour   = (65535*saturation());
				if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
					kdDebug( 14010 ) <<  k_funcinfo << "Card seems to not support adjusting image saturation. Fallback to it is not yet implemented." << endl;
			}
		break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			break;
	}
}

float VideoDevice::whiteness() const
{
  if (m_current_input < m_inputs.size() )
	return m_inputs[m_current_input].whiteness();
  else
	return 0;
}

void VideoDevice::setWhiteness(float newWhiteness)
{
	kdDebug( 14010 ) <<  k_funcinfo << " called." << endl;
	m_inputs[m_current_input].setWhiteness(newWhiteness); // Just to check bounds

	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			{
				struct v4l2_control control;
				struct v4l2_queryctrl queryctrl;
				float value = whiteness();
				CLEAR(queryctrl);
				queryctrl.id =  V4L2_CID_GAMMA;
				if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl)) {
					fprintf(stderr, "Whiteness control not supported.\n");
				} else {
					CLEAR(control);
					control.id    = V4L2_CID_GAMMA;
					control.value = queryctrl.minimum + (queryctrl.maximum - queryctrl.minimum) * value;
					fprintf(stderr, "V4L2: setWhiteness- value=%d\n", control.value);
					if (-1 == xioctl (VIDIOC_S_CTRL, &control)) {
						fprintf(stderr, "Whiteness control not supported.\n");
					}
				}
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			{
				struct video_picture V4L_picture;
				if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
					kdDebug( 14010 ) <<  k_funcinfo << "VIDIOCGPICT failed (" << errno << ")." << endl;
				V4L_picture.whiteness   = (65535*whiteness());
				if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
					kdDebug( 14010 ) <<  k_funcinfo << "Card seems to not support adjusting white level. Fallback to it is not yet implemented." << endl;
			}
		break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			break;
	}
}

float VideoDevice::hue() const
{
  if (m_current_input < m_inputs.size() )
	return m_inputs[m_current_input].hue();
  else
	return 0;
}

void VideoDevice::setHue(float newHue)
{
	kdDebug( 14010 ) <<  k_funcinfo << " called." << endl;
	m_inputs[m_current_input].setHue(newHue); // Just to check bounds

	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			{
				struct v4l2_control control;
				struct v4l2_queryctrl queryctrl;
				float value = hue();
				CLEAR(queryctrl);
				queryctrl.id =  V4L2_CID_HUE;
				if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl)) {
					fprintf(stderr, "Hue control not supported.\n");
				} else {
					CLEAR(control);
					control.id    = V4L2_CID_HUE;
					control.value = queryctrl.minimum + (queryctrl.maximum - queryctrl.minimum) * value;
					fprintf(stderr, "V4L2: setHue- value=%d\n", control.value);
					if (-1 == xioctl (VIDIOC_S_CTRL, &control)) {
						fprintf(stderr, "Hue control not supported.\n");
					}
				}
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			{
				struct video_picture V4L_picture;
				if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
					kdDebug( 14010 ) <<  k_funcinfo << "VIDIOCGPICT failed (" << errno << ")." << endl;
				V4L_picture.hue   = (65535*hue());
				if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
					kdDebug( 14010 ) <<  k_funcinfo << "Card seems to not support adjusting image hue. Fallback to it is not yet implemented." << endl;
			}
		break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			break;
	}
}


bool VideoDevice::autoBrightnessContrast() const
{
  if (m_current_input < m_inputs.size() )
	return m_inputs[m_current_input].autoBrightnessContrast();
  else
	return false;
}

void VideoDevice::setAutoBrightnessContrast(bool brightnesscontrast)
{
	kdDebug( 14010 ) <<  k_funcinfo << "VideoDevice::setAutoBrightnessContrast(" << brightnesscontrast << ") called." << endl;
	if (m_current_input < m_inputs.size() ) 
	  {
		m_inputs[m_current_input].setAutoBrightnessContrast( brightnesscontrast );
	  }
}

bool VideoDevice::autoColorCorrection() const
{
  if (m_current_input < m_inputs.size() )
	return m_inputs[m_current_input].autoColorCorrection();
  else
	return false;
}

void VideoDevice::setAutoColorCorrection(bool colorcorrection)
{
	kdDebug( 14010 ) <<  k_funcinfo << "VideoDevice::setAutoColorCorrection(" << colorcorrection << ") called." << endl;
	if (m_current_input < m_inputs.size() )
	  {
		m_inputs[m_current_input].setAutoColorCorrection(colorcorrection);
	  }
}

bool VideoDevice::imageAsMirror() const
{
  if (m_current_input < m_inputs.size() )
	return m_inputs[m_current_input].imageAsMirror();
  else
	return false;
}

void VideoDevice::setImageAsMirror(bool imageasmirror)
{
	kdDebug( 14010 ) <<  k_funcinfo << "VideoDevice::setImageAsMirror(" << imageasmirror << ") called." << endl;
	if (m_current_input < m_inputs.size() ) 
	  {
		m_inputs[m_current_input].setImageAsMirror(imageasmirror);
	  }
}

bool VideoDevice::mmapDisabled() const
{
	return m_disablemmap;
}

void VideoDevice::setDisableMMap(bool disablemmap)
{
	kdDebug( 14010 ) <<  k_funcinfo << "VideoDevice::setDisableMMap(" << disablemmap << ") called." << endl;
	m_disablemmap = disablemmap;
}

pixel_format VideoDevice::pixelFormatForPalette( int palette ) const
{
	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			switch(palette)
			{
				case 0 				: return PIXELFORMAT_NONE;	break;
				case V4L2_PIX_FMT_GREY		: return PIXELFORMAT_GREY;	break;
				case V4L2_PIX_FMT_RGB332	: return PIXELFORMAT_RGB332;	break;
				case V4L2_PIX_FMT_RGB555	: return PIXELFORMAT_RGB555;	break;
				case V4L2_PIX_FMT_RGB555X	: return PIXELFORMAT_RGB555X;	break;
				case V4L2_PIX_FMT_RGB565	: return PIXELFORMAT_RGB565;	break;
				case V4L2_PIX_FMT_RGB565X	: return PIXELFORMAT_RGB565X;	break;
				case V4L2_PIX_FMT_RGB24		: return PIXELFORMAT_RGB24;	break;
				case V4L2_PIX_FMT_BGR24		: return PIXELFORMAT_BGR24;	break;
				case V4L2_PIX_FMT_RGB32		: return PIXELFORMAT_RGB32;	break;
				case V4L2_PIX_FMT_BGR32		: return PIXELFORMAT_BGR32;	break;
				case V4L2_PIX_FMT_YUYV		: return PIXELFORMAT_YUYV;	break;
				case V4L2_PIX_FMT_UYVY		: return PIXELFORMAT_UYVY;	break;
				case V4L2_PIX_FMT_YUV420	: return PIXELFORMAT_YUV420P;	break;
				case V4L2_PIX_FMT_YUV422P	: return PIXELFORMAT_YUV422P;	break;
         			case V4L2_PIX_FMT_MJPEG         : return PIXELFORMAT_MJPEG;     break;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			switch(palette)
			{
				case 0				: return PIXELFORMAT_NONE;	break;
				case VIDEO_PALETTE_GREY		: return PIXELFORMAT_GREY;	break;
				case VIDEO_PALETTE_HI240	: return PIXELFORMAT_RGB332;	break;
				case VIDEO_PALETTE_RGB555	: return PIXELFORMAT_RGB555;	break;
				case VIDEO_PALETTE_RGB565	: return PIXELFORMAT_RGB565;	break;
				case VIDEO_PALETTE_RGB24	: return PIXELFORMAT_RGB24;	break;
				case VIDEO_PALETTE_RGB32	: return PIXELFORMAT_RGB32;	break;
				case VIDEO_PALETTE_YUYV		: return PIXELFORMAT_YUYV;	break;
				case VIDEO_PALETTE_UYVY		: return PIXELFORMAT_UYVY;	break;
				case VIDEO_PALETTE_YUV420	: return PIXELFORMAT_YUV420P;	break;
				case VIDEO_PALETTE_YUV422P	: return PIXELFORMAT_YUV422P;	break;
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			return PIXELFORMAT_NONE;	break;
	}
	return PIXELFORMAT_NONE;
}

int VideoDevice::pixelFormatCode(pixel_format pixelformat) const
{
	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			switch(pixelformat)
			{
				case PIXELFORMAT_NONE	: return 0;			break;
				case PIXELFORMAT_GREY	: return V4L2_PIX_FMT_GREY;	break;
				case PIXELFORMAT_RGB332	: return V4L2_PIX_FMT_RGB332;	break;
				case PIXELFORMAT_RGB555	: return V4L2_PIX_FMT_RGB555;	break;
				case PIXELFORMAT_RGB555X: return V4L2_PIX_FMT_RGB555X;	break;
				case PIXELFORMAT_RGB565	: return V4L2_PIX_FMT_RGB565;	break;
				case PIXELFORMAT_RGB565X: return V4L2_PIX_FMT_RGB565X;	break;
				case PIXELFORMAT_RGB24	: return V4L2_PIX_FMT_RGB24;	break;
				case PIXELFORMAT_BGR24	: return V4L2_PIX_FMT_BGR24;	break;
				case PIXELFORMAT_RGB32	: return V4L2_PIX_FMT_RGB32;	break;
				case PIXELFORMAT_BGR32	: return V4L2_PIX_FMT_BGR32;	break;
				case PIXELFORMAT_YUYV	: return V4L2_PIX_FMT_YUYV;	break;
				case PIXELFORMAT_UYVY	: return V4L2_PIX_FMT_UYVY;	break;
				case PIXELFORMAT_YUV420P: return V4L2_PIX_FMT_YUV420;	break;
				case PIXELFORMAT_YUV422P: return V4L2_PIX_FMT_YUV422P;	break;
         			case PIXELFORMAT_MJPEG  : return V4L2_PIX_FMT_MJPEG;    break;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			switch(pixelformat)
			{
				case PIXELFORMAT_NONE	: return 0;			break;
				case PIXELFORMAT_GREY	: return VIDEO_PALETTE_GREY;	break;
				case PIXELFORMAT_RGB332	: return VIDEO_PALETTE_HI240;	break;
				case PIXELFORMAT_RGB555	: return VIDEO_PALETTE_RGB555;	break;
				case PIXELFORMAT_RGB555X: return PIXELFORMAT_NONE;	break;
				case PIXELFORMAT_RGB565	: return VIDEO_PALETTE_RGB565;	break;
				case PIXELFORMAT_RGB565X: return PIXELFORMAT_NONE;	break;
				case PIXELFORMAT_RGB24	: return VIDEO_PALETTE_RGB24;	break;
				case PIXELFORMAT_BGR24	: return PIXELFORMAT_NONE;	break;
				case PIXELFORMAT_RGB32	: return VIDEO_PALETTE_RGB32;	break;
				case PIXELFORMAT_BGR32	: return PIXELFORMAT_NONE;	break;
				case PIXELFORMAT_YUYV	: return VIDEO_PALETTE_YUYV;	break;
				case PIXELFORMAT_UYVY	: return VIDEO_PALETTE_UYVY;	break;
				case PIXELFORMAT_YUV420P: return VIDEO_PALETTE_YUV420;	break;
				case PIXELFORMAT_YUV422P: return VIDEO_PALETTE_YUV422P;	break;
         			case PIXELFORMAT_MJPEG  : return PIXELFORMAT_NONE;      break;
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			return PIXELFORMAT_NONE;	break;
	}
	return PIXELFORMAT_NONE;
}

int VideoDevice::pixelFormatDepth(pixel_format pixelformat) const
{
	switch(pixelformat)
	{
		case PIXELFORMAT_NONE	: return 0;	break;
		case PIXELFORMAT_GREY	: return 8;	break;
		case PIXELFORMAT_RGB332	: return 8;	break;
		case PIXELFORMAT_RGB555	: return 16;	break;
		case PIXELFORMAT_RGB555X: return 16;	break;
		case PIXELFORMAT_RGB565	: return 16;	break;
		case PIXELFORMAT_RGB565X: return 16;	break;
		case PIXELFORMAT_RGB24	: return 24;	break;
		case PIXELFORMAT_BGR24	: return 24;	break;
		case PIXELFORMAT_RGB32	: return 32;	break;
		case PIXELFORMAT_BGR32	: return 32;	break;
		case PIXELFORMAT_YUYV	: return 16;	break;
		case PIXELFORMAT_UYVY	: return 16;	break;
		case PIXELFORMAT_YUV420P: return 16;	break;
		case PIXELFORMAT_YUV422P: return 16;	break;
		case PIXELFORMAT_MJPEG  : return 16;	break;
	}
	return 0;
}

QString VideoDevice::pixelFormatName(pixel_format pixelformat) const
{
	QString returnvalue;
	switch(pixelformat)
	{
		case PIXELFORMAT_NONE	: returnvalue = QString::fromLatin1("None");			break;
		case PIXELFORMAT_GREY	: returnvalue = QString::fromLatin1("8-bit Grayscale");	break;
		case PIXELFORMAT_RGB332	: returnvalue = QString::fromLatin1("8-bit RGB332");		break;
		case PIXELFORMAT_RGB555	: returnvalue = QString::fromLatin1("16-bit RGB555");	break;
		case PIXELFORMAT_RGB555X: returnvalue = QString::fromLatin1("16-bit RGB555X");	break;
		case PIXELFORMAT_RGB565	: returnvalue = QString::fromLatin1("16-bit RGB565");	break;
		case PIXELFORMAT_RGB565X: returnvalue = QString::fromLatin1("16-bit RGB565X");	break;
		case PIXELFORMAT_RGB24	: returnvalue = QString::fromLatin1("24-bit RGB24");		break;
		case PIXELFORMAT_BGR24	: returnvalue = QString::fromLatin1("24-bit BGR24");		break;
		case PIXELFORMAT_RGB32	: returnvalue = QString::fromLatin1("32-bit RGB32");		break;
		case PIXELFORMAT_BGR32	: returnvalue = QString::fromLatin1("32-bit BGR32");		break;
		case PIXELFORMAT_YUYV	: returnvalue = QString::fromLatin1("Packed YUV 4:2:2");	break;
		case PIXELFORMAT_UYVY	: returnvalue = QString::fromLatin1("Packed YVU 4:2:2");	break;
		case PIXELFORMAT_YUV420P: returnvalue = QString::fromLatin1("Planar YUV 4:2:0");	break;
		case PIXELFORMAT_YUV422P: returnvalue = QString::fromLatin1("Planar YUV 4:2:2");	break;
		case PIXELFORMAT_MJPEG:   returnvalue = QString::fromLatin1("M-JPEG");	        break;
		default:
			returnvalue = QString::fromLatin1("None");
	}
	return returnvalue;
}

QString VideoDevice::pixelFormatName(int pixelformat) const
{
	QString returnvalue;
	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			switch(pixelformat)
			{
				case V4L2_PIX_FMT_GREY		: returnvalue = pixelFormatName(PIXELFORMAT_GREY);	break;
				case V4L2_PIX_FMT_RGB332	: returnvalue = pixelFormatName(PIXELFORMAT_RGB332);	break;
				case V4L2_PIX_FMT_RGB555	: returnvalue = pixelFormatName(PIXELFORMAT_RGB555);	break;
				case V4L2_PIX_FMT_RGB555X	: returnvalue = pixelFormatName(PIXELFORMAT_RGB555X);	break;
				case V4L2_PIX_FMT_RGB565	: returnvalue = pixelFormatName(PIXELFORMAT_RGB565);	break;
				case V4L2_PIX_FMT_RGB565X	: returnvalue = pixelFormatName(PIXELFORMAT_RGB565X);	break;
				case V4L2_PIX_FMT_RGB24		: returnvalue = pixelFormatName(PIXELFORMAT_RGB24);	break;
				case V4L2_PIX_FMT_BGR24		: returnvalue = pixelFormatName(PIXELFORMAT_BGR24);	break;
				case V4L2_PIX_FMT_RGB32		: returnvalue = pixelFormatName(PIXELFORMAT_RGB32);	break;
				case V4L2_PIX_FMT_BGR32		: returnvalue = pixelFormatName(PIXELFORMAT_BGR32);	break;
				case V4L2_PIX_FMT_YUYV		: returnvalue = pixelFormatName(PIXELFORMAT_YUYV);	break;
				case V4L2_PIX_FMT_UYVY		: returnvalue = pixelFormatName(PIXELFORMAT_UYVY);	break;
				case V4L2_PIX_FMT_YUV420	: returnvalue = pixelFormatName(PIXELFORMAT_YUV420P);	break;
				case V4L2_PIX_FMT_YUV422P	: returnvalue = pixelFormatName(PIXELFORMAT_YUV422P);	break;
				case V4L2_PIX_FMT_MJPEG	        : returnvalue = pixelFormatName(PIXELFORMAT_MJPEG);	break;
				default:
					returnvalue = QString::fromLatin1("None");

			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			switch(pixelformat)
			{
				case VIDEO_PALETTE_GREY		: returnvalue = pixelFormatName(PIXELFORMAT_GREY);	break;
				case VIDEO_PALETTE_HI240	: returnvalue = pixelFormatName(PIXELFORMAT_RGB332);	break;
				case VIDEO_PALETTE_RGB555	: returnvalue = pixelFormatName(PIXELFORMAT_RGB555);	break;
				case VIDEO_PALETTE_RGB565	: returnvalue = pixelFormatName(PIXELFORMAT_RGB565);	break;
				case VIDEO_PALETTE_RGB24	: returnvalue = pixelFormatName(PIXELFORMAT_RGB24);	break;
				case VIDEO_PALETTE_RGB32	: returnvalue = pixelFormatName(PIXELFORMAT_RGB32);	break;
				case VIDEO_PALETTE_YUYV		: returnvalue = pixelFormatName(PIXELFORMAT_YUYV);	break;
				case VIDEO_PALETTE_UYVY		: returnvalue = pixelFormatName(PIXELFORMAT_UYVY);	break;
				case VIDEO_PALETTE_YUV420	: returnvalue = pixelFormatName(PIXELFORMAT_YUV420P);	break;
				case VIDEO_PALETTE_YUV422P	: returnvalue = pixelFormatName(PIXELFORMAT_YUV422P);	break;
				default:
					returnvalue = QString::fromLatin1("None");
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
			default:
				returnvalue = QString::fromLatin1("None");
			break;
	}
	return returnvalue;
}

__u64 VideoDevice::signalStandardCode(signal_standard standard)
{
	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			switch(standard)
			{
				case STANDARD_NONE	: return V4L2_STD_UNKNOWN;	break;
				case STANDARD_PAL_B	: return V4L2_STD_PAL_B;	break;
				case STANDARD_PAL_B1	: return V4L2_STD_PAL_B1;	break;
				case STANDARD_PAL_G	: return V4L2_STD_PAL_G;	break;
				case STANDARD_PAL_H	: return V4L2_STD_PAL_H;	break;
				case STANDARD_PAL_I	: return V4L2_STD_PAL_I;	break;
				case STANDARD_PAL_D	: return V4L2_STD_PAL_D;	break;
				case STANDARD_PAL_D1	: return V4L2_STD_PAL_D1;	break;
				case STANDARD_PAL_K	: return V4L2_STD_PAL_K;	break;
				case STANDARD_PAL_M	: return V4L2_STD_PAL_M;	break;
				case STANDARD_PAL_N	: return V4L2_STD_PAL_N;	break;
				case STANDARD_PAL_Nc	: return V4L2_STD_PAL_Nc;	break;
				case STANDARD_PAL_60	: return V4L2_STD_PAL_60;	break;
				case STANDARD_NTSC_M	: return V4L2_STD_NTSC_M;	break;
				case STANDARD_NTSC_M_JP	: return V4L2_STD_NTSC_M_JP;	break;
				case STANDARD_NTSC_443	: return V4L2_STD_NTSC;		break; // Using workaround value because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
				case STANDARD_SECAM_B	: return V4L2_STD_SECAM_B;	break;
				case STANDARD_SECAM_D	: return V4L2_STD_SECAM_D;	break;
				case STANDARD_SECAM_G	: return V4L2_STD_SECAM_G;	break;
				case STANDARD_SECAM_H	: return V4L2_STD_SECAM_H;	break;
				case STANDARD_SECAM_K	: return V4L2_STD_SECAM_K;	break;
				case STANDARD_SECAM_K1	: return V4L2_STD_SECAM_K1;	break;
				case STANDARD_SECAM_L	: return V4L2_STD_SECAM_L;	break;
				case STANDARD_SECAM_LC	: return V4L2_STD_SECAM;	break; // Using workaround value because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
				case STANDARD_ATSC_8_VSB	: return V4L2_STD_ATSC_8_VSB;	break; // ATSC/HDTV Standard officially not supported by V4L2 but exists in videodev2.h
				case STANDARD_ATSC_16_VSB	: return V4L2_STD_ATSC_16_VSB;	break; // ATSC/HDTV Standard officially not supported by V4L2 but exists in videodev2.h
				case STANDARD_PAL_BG	: return V4L2_STD_PAL_BG;	break;
				case STANDARD_PAL_DK	: return V4L2_STD_PAL_DK;	break;
				case STANDARD_PAL	: return V4L2_STD_PAL;		break;
				case STANDARD_NTSC	: return V4L2_STD_NTSC;		break;
				case STANDARD_SECAM_DK	: return V4L2_STD_SECAM_DK;	break;
				case STANDARD_SECAM	: return V4L2_STD_SECAM;	break;
				case STANDARD_525_60	: return V4L2_STD_525_60;	break;
				case STANDARD_625_50	: return V4L2_STD_625_50;	break;
				case STANDARD_ALL	: return V4L2_STD_ALL;		break;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			switch(standard)
			{
				case STANDARD_NONE	: return VIDEO_MODE_AUTO;	break;
				case STANDARD_PAL_B	: return VIDEO_MODE_PAL;	break;
				case STANDARD_PAL_B1	: return VIDEO_MODE_PAL;	break;
				case STANDARD_PAL_G	: return VIDEO_MODE_PAL;	break;
				case STANDARD_PAL_H	: return VIDEO_MODE_PAL;	break;
				case STANDARD_PAL_I	: return VIDEO_MODE_PAL;	break;
				case STANDARD_PAL_D	: return VIDEO_MODE_PAL;	break;
				case STANDARD_PAL_D1	: return VIDEO_MODE_PAL;	break;
				case STANDARD_PAL_K	: return VIDEO_MODE_PAL;	break;
				case STANDARD_PAL_M	: return 5;	break;	// Undocumented value found to be compatible with V4L bttv driver
				case STANDARD_PAL_N	: return 6;	break;	// Undocumented value found to be compatible with V4L bttv driver
				case STANDARD_PAL_Nc	: return 4;	break;	// Undocumented value found to be compatible with V4L bttv driver
				case STANDARD_PAL_60	: return VIDEO_MODE_PAL;	break;
				case STANDARD_NTSC_M	: return VIDEO_MODE_NTSC;	break;
				case STANDARD_NTSC_M_JP	: return 7;	break;	// Undocumented value found to be compatible with V4L bttv driver
				case STANDARD_NTSC_443	: return VIDEO_MODE_NTSC;	break; // Using workaround value because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
				case STANDARD_SECAM_B	: return VIDEO_MODE_SECAM;	break;
				case STANDARD_SECAM_D	: return VIDEO_MODE_SECAM;	break;
				case STANDARD_SECAM_G	: return VIDEO_MODE_SECAM;	break;
				case STANDARD_SECAM_H	: return VIDEO_MODE_SECAM;	break;
				case STANDARD_SECAM_K	: return VIDEO_MODE_SECAM;	break;
				case STANDARD_SECAM_K1	: return VIDEO_MODE_SECAM;	break;
				case STANDARD_SECAM_L	: return VIDEO_MODE_SECAM;	break;
				case STANDARD_SECAM_LC	: return VIDEO_MODE_SECAM;	break; // Using workaround value because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
				case STANDARD_ATSC_8_VSB	: return VIDEO_MODE_AUTO;	break; // ATSC/HDTV Standard officially not supported by V4L2 but exists in videodev2.h
				case STANDARD_ATSC_16_VSB	: return VIDEO_MODE_AUTO;	break; // ATSC/HDTV Standard officially not supported by V4L2 but exists in videodev2.h
				case STANDARD_PAL_BG	: return VIDEO_MODE_PAL;	break;
				case STANDARD_PAL_DK	: return VIDEO_MODE_PAL;	break;
				case STANDARD_PAL	: return VIDEO_MODE_PAL;	break;
				case STANDARD_NTSC	: return VIDEO_MODE_NTSC;	break;
				case STANDARD_SECAM_DK	: return VIDEO_MODE_SECAM;	break;
				case STANDARD_SECAM	: return VIDEO_MODE_SECAM;	break;
				case STANDARD_525_60	: return VIDEO_MODE_PAL;	break;
				case STANDARD_625_50	: return VIDEO_MODE_SECAM;	break;
				case STANDARD_ALL	: return VIDEO_MODE_AUTO;	break;
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			return STANDARD_NONE;	break;
	}
	return STANDARD_NONE;
}

QString VideoDevice::signalStandardName(signal_standard standard)
{
	QString returnvalue;
	returnvalue = "None";
	switch(standard)
	{
		case STANDARD_NONE	: returnvalue = "None";		break;
		case STANDARD_PAL_B	: returnvalue = "PAL-B";	break;
		case STANDARD_PAL_B1	: returnvalue = "PAL-B1"; 	break;
		case STANDARD_PAL_G	: returnvalue = "PAL-G";	break;
		case STANDARD_PAL_H	: returnvalue = "PAL-H";	break;
		case STANDARD_PAL_I	: returnvalue = "PAL-I";	break;
		case STANDARD_PAL_D	: returnvalue = "PAL-D";	break;
		case STANDARD_PAL_D1	: returnvalue = "PAL-D1";	break;
		case STANDARD_PAL_K	: returnvalue = "PAL-K";	break;
		case STANDARD_PAL_M	: returnvalue = "PAL-M";	break;
		case STANDARD_PAL_N	: returnvalue = "PAL-N";	break;
		case STANDARD_PAL_Nc	: returnvalue = "PAL-Nc";	break;
		case STANDARD_PAL_60	: returnvalue = "PAL-60";	break;
		case STANDARD_NTSC_M	: returnvalue = "NTSC-M";	break;
		case STANDARD_NTSC_M_JP	: returnvalue = "NTSC-M(JP)";	break;
		case STANDARD_NTSC_443	: returnvalue = "NTSC-443";	break;
		case STANDARD_SECAM_B	: returnvalue = "SECAM-B";	break;
		case STANDARD_SECAM_D	: returnvalue = "SECAM-D";	break;
		case STANDARD_SECAM_G	: returnvalue = "SECAM-G";	break;
		case STANDARD_SECAM_H	: returnvalue = "SECAM-H";	break;
		case STANDARD_SECAM_K	: returnvalue = "SECAM-K";	break;
		case STANDARD_SECAM_K1	: returnvalue = "SECAM-K1";	break;
		case STANDARD_SECAM_L	: returnvalue = "SECAM-L";	break;
		case STANDARD_SECAM_LC	: returnvalue = "SECAM-LC";	break;
		case STANDARD_ATSC_8_VSB	: returnvalue = "ATSC-8-VSB";	break; // ATSC/HDTV Standard officially not supported by V4L2 but exists in videodev2.h
		case STANDARD_ATSC_16_VSB	: returnvalue = "ATSC-16-VSB";	break; // ATSC/HDTV Standard officially not supported by V4L2 but exists in videodev2.h
		case STANDARD_PAL_BG	: returnvalue = "PAL-BG";	break;
		case STANDARD_PAL_DK	: returnvalue = "PAL-DK";	break;
		case STANDARD_PAL	: returnvalue = "PAL";		break;
		case STANDARD_NTSC	: returnvalue = "NTSC";		break;
		case STANDARD_SECAM_DK  : returnvalue = "SECAM-DK";	break;
		case STANDARD_SECAM	: returnvalue = "SECAM";	break;
		case STANDARD_525_60	: returnvalue = "525 lines 60Hz";	break;
		case STANDARD_625_50	: returnvalue = "625 lines 50Hz"; 	break;
		case STANDARD_ALL	: returnvalue = "All";		break;
	}
	return returnvalue;
}

QString VideoDevice::signalStandardName(int standard)
{
	QString returnvalue;
	returnvalue = "None";
	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			switch(standard)
			{
				case V4L2_STD_PAL_B	: returnvalue = signalStandardName(STANDARD_PAL_B);	break;
				case V4L2_STD_PAL_B1	: returnvalue = signalStandardName(STANDARD_PAL_B1);	break;
				case V4L2_STD_PAL_G	: returnvalue = signalStandardName(STANDARD_PAL_G);	break;
				case V4L2_STD_PAL_H	: returnvalue = signalStandardName(STANDARD_PAL_H);	break;
				case V4L2_STD_PAL_I	: returnvalue = signalStandardName(STANDARD_PAL_I);	break;
				case V4L2_STD_PAL_D	: returnvalue = signalStandardName(STANDARD_PAL_D);	break;
				case V4L2_STD_PAL_D1	: returnvalue = signalStandardName(STANDARD_PAL_D1);	break;
				case V4L2_STD_PAL_K	: returnvalue = signalStandardName(STANDARD_PAL_K);	break;
				case V4L2_STD_PAL_M	: returnvalue = signalStandardName(STANDARD_PAL_M);	break;
				case V4L2_STD_PAL_N	: returnvalue = signalStandardName(STANDARD_PAL_N);	break;
				case V4L2_STD_PAL_Nc	: returnvalue = signalStandardName(STANDARD_PAL_Nc);	break;
				case V4L2_STD_PAL_60	: returnvalue = signalStandardName(STANDARD_PAL_60);	break;
				case V4L2_STD_NTSC_M	: returnvalue = signalStandardName(STANDARD_NTSC_M);	break;
				case V4L2_STD_NTSC_M_JP	: returnvalue = signalStandardName(STANDARD_NTSC_M_JP);	break;
//				case V4L2_STD_NTSC_443	: returnvalue = signalStandardName(STANDARD_NTSC_443);	break; // Commented out because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
				case V4L2_STD_SECAM_B	: returnvalue = signalStandardName(STANDARD_SECAM_B);	break;
				case V4L2_STD_SECAM_D	: returnvalue = signalStandardName(STANDARD_SECAM_D);	break;
				case V4L2_STD_SECAM_G	: returnvalue = signalStandardName(STANDARD_SECAM_G);	break;
				case V4L2_STD_SECAM_H	: returnvalue = signalStandardName(STANDARD_SECAM_H);	break;
				case V4L2_STD_SECAM_K	: returnvalue = signalStandardName(STANDARD_SECAM_K);	break;
				case V4L2_STD_SECAM_K1	: returnvalue = signalStandardName(STANDARD_SECAM_K1);	break;
				case V4L2_STD_SECAM_L	: returnvalue = signalStandardName(STANDARD_SECAM_L);	break;
//				case V4L2_STD_SECAM_LC	: returnvalue = signalStandardName(STANDARD_SECAM_LC);	break; // Commented out because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
				case V4L2_STD_ATSC_8_VSB	: returnvalue = signalStandardName(STANDARD_ATSC_8_VSB);	break; // ATSC/HDTV Standard officially not supported by V4L2 but exists in videodev2.h
				case V4L2_STD_ATSC_16_VSB	: returnvalue = signalStandardName(STANDARD_ATSC_16_VSB);	break; // ATSC/HDTV Standard officially not supported by V4L2 but exists in videodev2.h
				case V4L2_STD_PAL_BG	: returnvalue = signalStandardName(STANDARD_PAL_BG);	break;
				case V4L2_STD_PAL_DK	: returnvalue = signalStandardName(STANDARD_PAL_DK);	break;
				case V4L2_STD_PAL	: returnvalue = signalStandardName(STANDARD_PAL);	break;
				case V4L2_STD_NTSC	: returnvalue = signalStandardName(STANDARD_NTSC);	break;
				case V4L2_STD_SECAM_DK	: returnvalue = signalStandardName(STANDARD_SECAM_DK);	break;
				case V4L2_STD_SECAM	: returnvalue = signalStandardName(STANDARD_SECAM);	break;
				case V4L2_STD_525_60	: returnvalue = signalStandardName(STANDARD_525_60);	break;
				case V4L2_STD_625_50	: returnvalue = signalStandardName(STANDARD_625_50);	break;
				case V4L2_STD_ALL	: returnvalue = signalStandardName(STANDARD_ALL);	break;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			switch(standard)
			{
				case VIDEO_MODE_PAL	: returnvalue = signalStandardName(STANDARD_PAL);	break;
				case VIDEO_MODE_NTSC	: returnvalue = signalStandardName(STANDARD_NTSC);	break;
				case VIDEO_MODE_SECAM	: returnvalue = signalStandardName(STANDARD_SECAM);	break;
				case VIDEO_MODE_AUTO	: returnvalue = signalStandardName(STANDARD_ALL);	break;	// It must be disabled until I find a correct way to handle those non-standard bttv modes
//				case VIDEO_MODE_PAL_Nc	: returnvalue = signalStandardName(STANDARD_PAL_Nc);	break;	// Undocumented value found to be compatible with V4L bttv driver
				case VIDEO_MODE_PAL_M	: returnvalue = signalStandardName(STANDARD_PAL_M);	break;	// Undocumented value found to be compatible with V4L bttv driver
				case VIDEO_MODE_PAL_N	: returnvalue = signalStandardName(STANDARD_PAL_N);	break;	// Undocumented value found to be compatible with V4L bttv driver
				case VIDEO_MODE_NTSC_JP	: returnvalue = signalStandardName(STANDARD_NTSC_M_JP);	break;	// Undocumented value found to be compatible with V4L bttv driver
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			break;
	}
	return returnvalue;
}

/*!
    \fn VideoDevice::detectSignalStandards()
 */
int VideoDevice::detectSignalStandards()
{
	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			break;
	}
	//FIXME: return a real value
	return 0;
}

/*!
    \fn VideoDevice::initRead()
 */
int VideoDevice::initRead()
{
    /// @todo implement me

	kdDebug( 14010 ) <<  k_funcinfo << "called." << endl;
	if(isOpen())
	{
		m_rawbuffers.resize(1);
		if (m_rawbuffers.size()==0)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}
		kdDebug( 14010 ) <<  k_funcinfo << "m_buffer_size: " << m_buffer_size << endl;

//		m_rawbuffers[0].pixelformat=m_pixelformat;
		m_rawbuffers[0].length = m_buffer_size;
		m_rawbuffers[0].start = (uchar *)malloc (m_buffer_size);

		if (!m_rawbuffers[0].start)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}
		kdDebug( 14010 ) <<  k_funcinfo << "exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


/*!
    \fn VideoDevice::initMmap()
 */
int VideoDevice::initMmap()
{
    /// @todo implement me
#define BUFFERS 2
	if(isOpen())
	{
		kdDebug( 14010 ) <<  k_funcinfo << m_devicePath << " Trying to MMAP" << endl;
#ifdef HAVE_V4L2
		struct v4l2_requestbuffers req;

		CLEAR (req);

		req.count  = BUFFERS;
		req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl (VIDIOC_REQBUFS, &req))
		{
			if (EINVAL == errno)
			{
				kdDebug( 14010 ) <<  k_funcinfo << m_devicePath << " does not support memory mapping" << endl;
				return EXIT_FAILURE;
			}
			else
			{
				return errnoReturn ("VIDIOC_REQBUFS");
			}
		}

		if (req.count < BUFFERS)
		{
			kdDebug( 14010 ) <<  k_funcinfo << "Insufficient buffer memory on " << m_devicePath << endl;
			return EXIT_FAILURE;
		}

		m_rawbuffers.resize(req.count);

		if (m_rawbuffers.size()==0)
		{
			kdDebug( 14010 ) <<  k_funcinfo <<  "Out of memory" << endl;
			return EXIT_FAILURE;
		}

		for (m_streambuffers = 0; m_streambuffers < req.count; ++m_streambuffers)
		{
			struct v4l2_buffer v4l2buffer;

			CLEAR (v4l2buffer);

			v4l2buffer.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			v4l2buffer.memory = V4L2_MEMORY_MMAP;
			v4l2buffer.index  = m_streambuffers;

			if (-1 == xioctl (VIDIOC_QUERYBUF, &v4l2buffer))
				return errnoReturn ("VIDIOC_QUERYBUF");

			m_rawbuffers[m_streambuffers].length = v4l2buffer.length;
			m_rawbuffers[m_streambuffers].start = (uchar *) mmap (NULL /* start anywhere */, v4l2buffer.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, descriptor, v4l2buffer.m.offset);

			if (MAP_FAILED == m_rawbuffers[m_streambuffers].start)
			return errnoReturn ("mmap");
		}
#endif
		m_currentbuffer.data.resize(m_rawbuffers[0].length); // Makes the imagesize.data buffer size equal to the rawbuffer size
		kdDebug( 14010 ) <<  k_funcinfo << m_devicePath << " m_currentbuffer.data.size(): " << m_currentbuffer.data.size() << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


/*!
    \fn VideoDevice::initUserptr()
 */
int VideoDevice::initUserptr()
{
    /// @todo implement me
	if(isOpen())
	{
#ifdef HAVE_V4L2
		struct v4l2_requestbuffers req;

		CLEAR (req);

		req.count  = 2;
		req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl (VIDIOC_REQBUFS, &req))
		{
			if (EINVAL == errno)
			{
				kdDebug( 14010 ) <<  k_funcinfo << m_devicePath << " does not support memory mapping" << endl;
				return EXIT_FAILURE;
			}
			else
			{
				return errnoReturn ("VIDIOC_REQBUFS");
			}
		}

		m_rawbuffers.resize(4);

		if (m_rawbuffers.size()==0)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}

		for (m_streambuffers = 0; m_streambuffers < 4; ++m_streambuffers)
		{
			m_rawbuffers[m_streambuffers].length = m_buffer_size;
			m_rawbuffers[m_streambuffers].start = (uchar *) malloc (m_buffer_size);

			if (!m_rawbuffers[m_streambuffers].start)
			{
				kdDebug( 14010 ) <<  k_funcinfo <<  "Out of memory" << endl;
				return EXIT_FAILURE;
			}
		}
#endif
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

bool VideoDevice::canCapture()
{
	return m_videocapture;
}

bool VideoDevice::canChromakey()
{
	return m_videochromakey;
}

bool VideoDevice::canScale()
{
	return m_videoscale;
}

bool VideoDevice::canOverlay()
{
	return m_videooverlay;
}

bool VideoDevice::canRead()
{
	return m_videoread;
}

bool VideoDevice::canAsyncIO()
{
	return m_videoasyncio;
}

bool VideoDevice::canStream()
{
	return m_videostream;
}

size_t VideoDevice::getModelIndex()
{
	return m_modelindex;
}
void VideoDevice::setModelIndex(size_t modelindex)
{
	m_modelindex = modelindex;
}

} // namespace AV

} // namespace Kopete
