//
// C++ Implementation: videodevicelistitem
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <iostream>
#include <ostream>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <kdebug.h>

#include "videoinput.h"
#include "videodevice.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

VideoDevice::VideoDevice()
{
//	kdDebug() << "libkopete (avdevice): VideoDevice() called" << endl;
	descriptor = -1;
	m_streambuffers  = 0;
	m_autobrightcontrast = false;
	m_autocolorcorrection = false;
//	kdDebug() << "libkopete (avdevice): VideoDevice() exited successfuly" << endl;
}


VideoDevice::~VideoDevice()
{
}

/*!
    \fn VideoDevice::xioctl(int fd, int request, void *arg)
 */
int VideoDevice::xioctl(int request, void *arg)
{
	int r;

	do r = ioctl (descriptor, request, arg);
	while (-1 == r && EINTR == errno);
	return r;
}

/*!
    \fn VideoDevice::errnoReturn(const char* s)
 */
int VideoDevice::errnoReturn(const char* s)
{
    /// @todo implement me
	fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
	return EXIT_FAILURE;
}

/*!
    \fn VideoDevice::setFileName(QString name)
 */
int VideoDevice::setFileName(QString filename)
{
    /// @todo implement me
	full_filename=filename;
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevice::open()
 */
int VideoDevice::open()
{
    /// @todo implement me

	kdDebug() <<  k_funcinfo << "called" << endl;
	if(-1 != descriptor)
	{
		kdDebug() <<  k_funcinfo << "Device is already open" << endl;
		return EXIT_SUCCESS;
	}
	descriptor = ::open (full_filename.local8Bit(), O_RDWR /* required */ | O_NONBLOCK, 0);
	if(isOpen())
	{
		kdDebug() <<  k_funcinfo << "File " << full_filename << " was opened successfuly" << endl;
		if(EXIT_FAILURE==checkDevice())
		{
			kdDebug() <<  k_funcinfo << "File " << full_filename << " could not be opened" << endl;
			close();
			return EXIT_FAILURE;
		}
	}
	initDevice();
	kdDebug() <<  k_funcinfo << "exited successfuly" << endl;
	return EXIT_SUCCESS;
}

bool VideoDevice::isOpen()
{
	if(-1 == descriptor)
	{
//		kdDebug() <<  k_funcinfo << "VideoDevice::isOpen() File is not open" << endl;
		return false;
	}
//	kdDebug() <<  k_funcinfo << "VideoDevice::isOpen() File is open" << endl;
	return true;
}

int VideoDevice::checkDevice()
{
	kdDebug() <<  k_funcinfo << "checkDevice() called." << endl;
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
		memset(&V4L2_capabilities, 0, sizeof(V4L2_capabilities));

		if (-1 != xioctl (VIDIOC_QUERYCAP, &V4L2_capabilities))
		{
			if (!(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE))
			{
				kdDebug() <<  k_funcinfo << "checkDevice(): " << full_filename << " is not a video capture device." << endl;
				m_driver = VIDEODEV_DRIVER_NONE;
				return EXIT_FAILURE;
			}
			m_videocapture=true;
			kdDebug() <<  k_funcinfo << "checkDevice(): " << full_filename << " is a V4L2 device." << endl;
			m_driver = VIDEODEV_DRIVER_V4L2;
			name=QString::fromLocal8Bit((const char*)V4L2_capabilities.card);



// Detect maximum and minimum resolution supported by the V4L2 device
			CLEAR (fmt);
			fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    			if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
				kdDebug() <<  k_funcinfo << "VIDIOC_G_FMT failed (" << errno << ")." << endl;
			fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			fmt.fmt.pix.width       = 32767;
			fmt.fmt.pix.height      = 32767;
			fmt.fmt.pix.field       = V4L2_FIELD_ANY;
			if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
			{
				kdDebug() << k_funcinfo << "Detecting maximum size with VIDIOC_S_FMT failed (" << errno << ").Returned maxwidth: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
				// Note VIDIOC_S_FMT may change width and height.
			}
			else
			{
				maxwidth  = fmt.fmt.pix.width;
				maxheight = fmt.fmt.pix.height;
			}
			if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
				kdDebug() << k_funcinfo << "VIDIOC_G_FMT failed (" << errno << ")." << endl;
			fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			fmt.fmt.pix.width       = 1;
			fmt.fmt.pix.height      = 1;
			fmt.fmt.pix.field       = V4L2_FIELD_ANY;
			if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
			{
				kdDebug() << k_funcinfo << "Detecting minimum size with VIDIOC_S_FMT failed (" << errno << ").Returned maxwidth: " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
				// Note VIDIOC_S_FMT may change width and height.
			}
			else
			{
				minwidth  = fmt.fmt.pix.width;
				minheight = fmt.fmt.pix.height;
			}

// Buggy driver paranoia
/*				min = fmt.fmt.pix.width * 2;
				if (fmt.fmt.pix.bytesperline < min)
					fmt.fmt.pix.bytesperline = min;
				min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
				if (fmt.fmt.pix.sizeimage < min)
					fmt.fmt.pix.sizeimage = min;
				m_buffer_size=fmt.fmt.pix.sizeimage ;*/

			int inputisok=EXIT_SUCCESS;
			input.clear();
			for(unsigned int loop=0; inputisok==EXIT_SUCCESS; loop++)
			{
				struct v4l2_input videoinput;
				memset(&videoinput, 0, sizeof(videoinput));
				videoinput.index = loop;
				inputisok=xioctl(VIDIOC_ENUMINPUT, &videoinput);
				if(inputisok==EXIT_SUCCESS)
				{
					VideoInput tempinput;
					tempinput.name = QString::fromLocal8Bit((const char*)videoinput.name);
					tempinput.hastuner=videoinput.type & V4L2_INPUT_TYPE_TUNER;
					input.push_back(tempinput);
					kdDebug() <<  k_funcinfo << "Input " << loop << ": " << tempinput.name << " (tuner: " << ((videoinput.type & V4L2_INPUT_TYPE_TUNER) != 0) << ")" << endl;
					if((videoinput.type & V4L2_INPUT_TYPE_TUNER) != 0)
					{
//						_tunerForInput[name] = desc.tuner;
//						_isTuner = true;
					}
					else
					{
//						_tunerForInput[name] = -1;
					}
				}
			}


		}
		else
		{
			kdDebug() <<  k_funcinfo << "checkDevice(): " << full_filename << " is no V4L2 device." << endl;
		}
#endif

		memset(&V4L_capabilities, 0, sizeof(V4L_capabilities));

		if(m_driver==VIDEODEV_DRIVER_NONE)
		{
			kdDebug() <<  k_funcinfo << "checkDevice(): " << full_filename << " Trying V4L API." << endl;
			if (-1 == xioctl (VIDIOCGCAP, &V4L_capabilities))
			{
				perror ("ioctl (VIDIOCGCAP)");
				m_driver = VIDEODEV_DRIVER_NONE;
				return EXIT_FAILURE;
			}
			else
			{
				kdDebug() <<  k_funcinfo << full_filename << " is a V4L device." << endl;
				m_driver = VIDEODEV_DRIVER_V4L;
				name=QString::fromLocal8Bit((const char*)V4L_capabilities.name);

				if(V4L_capabilities.type & VID_TYPE_CAPTURE)
					m_videocapture=true;
				if(V4L_capabilities.type & VID_TYPE_CHROMAKEY)
					m_videochromakey=true;
				if(V4L_capabilities.type & VID_TYPE_SCALES)
					m_videoscale=true;	
				if(V4L_capabilities.type & VID_TYPE_OVERLAY)
					m_videooverlay=true;
//				kdDebug() << "libkopete (avdevice):     Inputs : " << V4L_capabilities.channels << endl;
//				kdDebug() << "libkopete (avdevice):     Audios : " << V4L_capabilities.audios << endl;
				minwidth  = V4L_capabilities.minwidth;
				maxwidth  = V4L_capabilities.maxwidth;
				minheight = V4L_capabilities.minheight;
				maxheight = V4L_capabilities.maxheight;


				int inputisok=EXIT_SUCCESS;
				input.clear();
				for(int loop=0; loop < V4L_capabilities.channels; loop++)
				{
					struct video_channel videoinput;
					memset(&videoinput, 0, sizeof(videoinput));
					videoinput.channel = loop;
					videoinput.norm    = 1;
					inputisok=xioctl(VIDIOCGCHAN, &videoinput);
					if(inputisok==EXIT_SUCCESS)
					{
						VideoInput tempinput;
						tempinput.name = QString::fromLocal8Bit((const char*)videoinput.name);
						tempinput.hastuner=videoinput.flags & VIDEO_VC_TUNER;
						input.push_back(tempinput);
//						kdDebug() << "libkopete (avdevice): Input " << loop << ": " << tempinput.name << " (tuner: " << ((videoinput.flags & VIDEO_VC_TUNER) != 0) << ")" << endl;
/*						if((input.type & V4L2_INPUT_TYPE_TUNER) != 0)
						{
//							_tunerForInput[name] = desc.tuner;
//							_isTuner = true;
						}
						else
						{
//							_tunerForInput[name] = -1;
						}
*/					}
				}

			}
		}
#endif
		// Now we must execute the proper initialization according to the type of the driver.
		kdDebug() <<  k_funcinfo << "checkDevice() exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


/*!
    \fn VideoDevice::showDeviceCapabilities()
 */
int VideoDevice::showDeviceCapabilities()
{
	kdDebug() <<  k_funcinfo << "showDeviceCapabilities() called." << endl;
	if(isOpen())
	{
/*		kdDebug() << "libkopete (avdevice): Driver: " << (const char*)V4L2_capabilities.driver << " "
			<< ((V4L2_capabilities.version>>16) & 0xFF) << "."
			<< ((V4L2_capabilities.version>> 8) & 0xFF) << "."
			<< ((V4L2_capabilities.version    ) & 0xFF) << endl;
		kdDebug() << "libkopete (avdevice): Card: " << name << endl;
		kdDebug() << "libkopete (avdevice): Capabilities:" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE)
			kdDebug() << "libkopete (avdevice):     Video capture" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_OUTPUT)
			kdDebug() << "libkopete (avdevice):     Video output" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_OVERLAY)
			kdDebug() << "libkopete (avdevice):     Video overlay" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_VBI_CAPTURE)
			kdDebug() << "libkopete (avdevice):     VBI capture" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_VBI_OUTPUT)
			kdDebug() << "libkopete (avdevice):     VBI output" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_RDS_CAPTURE)
			kdDebug() << "libkopete (avdevice):     RDS capture" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_TUNER)
			kdDebug() << "libkopete (avdevice):     Tuner IO" << endl;
		if(V4L2_capabilities.capabilities & V4L2_CAP_AUDIO)
			kdDebug() << "libkopete (avdevice):     Audio IO" << endl;
;*/
		kdDebug() <<  k_funcinfo << "Card: " << name << endl;
		kdDebug() <<  k_funcinfo << "Capabilities:" << endl;
		if(canCapture())
			kdDebug() <<  k_funcinfo << "    Video capture" << endl;
		if(canRead())
			kdDebug() <<  k_funcinfo << "        Read" << endl;
		if(canAsyncIO())
			kdDebug() <<  k_funcinfo << "        Asynchronous input/output" << endl;
		if(canStream())
			kdDebug() <<  k_funcinfo << "        Streaming" << endl;
		if(canChromakey())
			kdDebug() <<  k_funcinfo << "    Video chromakey" << endl;
		if(canScale())
			kdDebug() <<  k_funcinfo << "    Video scales" << endl;
		if(canOverlay())
			kdDebug() <<  k_funcinfo << "    Video overlay" << endl;
//		kdDebug() << "libkopete (avdevice):     Audios : " << V4L_capabilities.audios << endl;
		kdDebug() <<  k_funcinfo << "    Max res: " << maxWidth() << " x " << maxHeight() << endl;
		kdDebug() <<  k_funcinfo << "    Min res: " << minWidth() << " x " << minHeight() << endl;
		kdDebug() <<  k_funcinfo << "    Inputs : " << inputs() << endl;
		for (unsigned int loop=0; loop < inputs(); loop++)
			kdDebug() <<  k_funcinfo << "Input " << loop << ": " << input[loop].name << " (tuner: " << input[loop].hastuner << ")" << endl;
		kdDebug() <<  k_funcinfo << "showDeviceCapabilities() exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/*!
    \fn VideoDevicePool::initDevice()
 */
int VideoDevice::initDevice()
{
    /// @todo implement me
	kdDebug() <<  k_funcinfo << "initDevice() started" << endl;
	if(-1 == descriptor)
	{
		kdDebug() <<  k_funcinfo << "initDevice() Device is not open" << endl;
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
				kdDebug() <<  k_funcinfo << "    Read/Write interface" << endl;
			}
			if(V4L2_capabilities.capabilities & V4L2_CAP_ASYNCIO)
			{
				m_videoasyncio=true;
				kdDebug() <<  k_funcinfo << "    Async IO interface" << endl;
			}
			if(V4L2_capabilities.capabilities & V4L2_CAP_STREAMING)
			{
				m_videostream=true;
				m_io_method = IO_METHOD_MMAP;
//				m_io_method = IO_METHOD_USERPTR;
				kdDebug() <<  k_funcinfo << "    Streaming interface" << endl;
			}
			if(m_io_method==IO_METHOD_NONE)
			{
				kdDebug() <<  k_funcinfo << "initDevice() Found no suitable input/output method for " << full_filename << endl;
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
				kdDebug() <<  k_funcinfo << "    Streaming interface" << endl;
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
	kdDebug() <<  k_funcinfo << "initDevice() exited successfuly" << endl;
	return EXIT_SUCCESS;
}

unsigned int VideoDevice::inputs()
{
	return input.size();
}


int VideoDevice::width()
{
	return currentwidth;
}

int VideoDevice::minWidth()
{
	return minwidth;
}

int VideoDevice::maxWidth()
{
	return maxwidth;
}

int VideoDevice::height()
{
	return currentheight;
}

int VideoDevice::minHeight()
{
	return minheight;
}

int VideoDevice::maxHeight()
{
	return maxheight;
}

int VideoDevice::setSize( int newwidth, int newheight)
{
kdDebug() <<  k_funcinfo << "setSize(" << newwidth << ", " << newheight << ") called." << endl;
	if(isOpen())
	{
// It should not be there. It must remain in a completely distict place, cause this method should not change the pixelformat.
		if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_RGB24))
		{
			kdDebug() <<  k_funcinfo << "Card seems to not support RGB24 format. Trying BGR24." << endl;
			if(PIXELFORMAT_NONE == setPixelFormat(PIXELFORMAT_BGR24))
				kdDebug() <<  k_funcinfo << "Card seems to not support BGR24 format. Fallback to it is not yet implemented." << endl;
		}

		if(newwidth  > maxwidth ) newwidth  = maxwidth;
		if(newheight > maxheight) newheight = maxheight;
		if(newwidth  < minwidth ) newwidth  = minwidth;
		if(newheight < minheight) newheight = minheight;

		currentwidth  = newwidth;
		currentheight = newheight;

//kdDebug() << k_funcinfo << "width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << width() << "x" << height() << endl;
// Change resolution for the video device
		switch(m_driver)
		{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
			case VIDEODEV_DRIVER_V4L2:
//				CLEAR (fmt);
				if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
					kdDebug() << k_funcinfo << "VIDIOC_G_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
				fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				fmt.fmt.pix.width       = width();
				fmt.fmt.pix.height      = height();
				fmt.fmt.pix.field       = V4L2_FIELD_ANY;
				if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
				{
					kdDebug() << k_funcinfo << "VIDIOC_S_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
					// Note VIDIOC_S_FMT may change width and height.
				}
				else
				{
// Buggy driver paranoia.
kdDebug() << k_funcinfo << "VIDIOC_S_FMT worked (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
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

kdDebug() << "------------- width: " << V4L_videowindow.width << " Height: " << V4L_videowindow.height << " Clipcount: " << V4L_videowindow.clipcount << " -----------------" << endl;

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
kdDebug() << "------------- width: " << V4L_videowindow.width << " Height: " << V4L_videowindow.height << " Clipcount: " << V4L_videowindow.clipcount << " -----------------" << endl;

//				kdDebug() << "libkopete (avdevice): V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth << endl;

/*				if(-1 == xioctl(VIDIOCGFBUF,&V4L_videobuffer))
					kdDebug() << "libkopete (avdevice): VIDIOCGFBUF failed (" << errno << "): Card cannot stream" << endl;*/

				}
				break;
#endif
			case VIDEODEV_DRIVER_NONE:
			default:
				break;
		}
		m_buffer_size = width() * height() * pixelFormatDepth(m_pixelformat) / 8;
kdDebug() << "------------------------- ------- -- m_buffer_size: " << m_buffer_size << " !!! -- ------- -----------------------------------------" << endl;

		m_currentbuffer.pixelformat=m_pixelformat;
		m_currentbuffer.data.resize(m_buffer_size);

		switch (m_io_method)
		{
			case IO_METHOD_NONE:                    break;
			case IO_METHOD_READ:    initRead ();    break;
			case IO_METHOD_MMAP:    initMmap ();    break;
			case IO_METHOD_USERPTR: initUserptr (); break;
		}

kdDebug() <<  k_funcinfo << "setSize(" << newwidth << ", " << newheight << ") exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
kdDebug() <<  k_funcinfo << "setSize(" << newwidth << ", " << newheight << ") Device is not open." << endl;
	return EXIT_FAILURE;
}













pixel_format VideoDevice::setPixelFormat(pixel_format newformat)
{
kdDebug() <<  k_funcinfo << "called." << endl;
// Change the pixel format for the video device
	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
//			CLEAR (fmt);
			if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
//				return errnoReturn ("VIDIOC_S_FMT");
				kdDebug() << k_funcinfo << "VIDIOC_G_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
			fmt.fmt.pix.pixelformat = pixelFormatCode(newformat);
			if (-1 != xioctl (VIDIOC_S_FMT, &fmt))
			{
				m_pixelformat=newformat;
				return newformat;
			}
			kdDebug() << k_funcinfo << "VIDIOC_S_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << endl;
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			{
			struct video_picture V4L_picture;
			if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
				kdDebug() <<  k_funcinfo << "VIDIOCGPICT failed (" << errno << ")." << endl;
			kdDebug() <<  k_funcinfo << "V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth << endl;
			V4L_picture.palette = pixelFormatCode(newformat);
			V4L_picture.depth   = pixelFormatDepth(newformat);
			if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
			{
				kdDebug() <<  k_funcinfo << "Card seems to not support " << pixelFormatName(newformat) << " format. Fallback to it is not yet implemented." << endl;
			}
			if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
				kdDebug() <<  k_funcinfo << "VIDIOCGPICT failed (" << errno << ")." << endl;

			kdDebug() <<  k_funcinfo << "V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth << endl;
				m_pixelformat=newformat;
				return newformat;
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			break;
	}
	return PIXELFORMAT_NONE;
}

















/*!
    \fn Kopete::AV::VideoDevice::selectInput(int input)
 */
int VideoDevice::selectInput(int newinput)
{
    /// @todo implement me
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
				V4L_input.norm=1;
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
		kdDebug() <<  k_funcinfo << "Selected input " << newinput << " (" << input[newinput].name << ")" << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/*!
    \fn VideoDevice::startCapturing()
 */
int VideoDevice::startCapturing()
{

	kdDebug() <<  k_funcinfo << "called." << endl;
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

		kdDebug() <<  k_funcinfo << "exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/*!
    \fn VideoDevice::getFrame()
 */
int VideoDevice::getFrame()
{
    /// @todo implement me
	ssize_t bytesread;

#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
	struct v4l2_buffer v4l2buffer;
#endif
#endif
	kdDebug() <<  k_funcinfo << "getFrame() called." << endl;
	if(isOpen())
	{
		switch (m_io_method)
		{
			case IO_METHOD_NONE: // Card cannot capture frames
				return EXIT_FAILURE;
				break;
			case IO_METHOD_READ:
				kdDebug() <<  k_funcinfo << "Using IO_METHOD_READ.File descriptor: " << descriptor << " Buffer address: " << &m_currentbuffer.data[0] << " Size: " << m_currentbuffer.data.size() << endl;
				bytesread = read (descriptor, &m_currentbuffer.data[0], m_currentbuffer.data.size());
				if (-1 == bytesread)
				{
					kdDebug() <<  k_funcinfo << "IO_METHOD_READ failed." << endl;
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
					kdDebug() <<  k_funcinfo << "IO_METHOD_READ returned less bytes (" << bytesread << ") than it was asked for (" << m_currentbuffer.data.size() <<")." << endl;
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
					kdDebug() <<  k_funcinfo << full_filename << " MMAPed getFrame failed." << endl;
					switch (errno)
					{
						case EAGAIN:
						{
							kdDebug() <<  k_funcinfo << full_filename << " MMAPed getFrame failed: EAGAIN. Pointer: " << endl;
							return EXIT_FAILURE;
						}
						case EIO: /* Could ignore EIO, see spec. fall through */
						default:
							return errnoReturn ("VIDIOC_DQBUF");
					}
				}
/*				if (v4l2buffer.index < m_streambuffers)
					return EXIT_FAILURE;*/ //it was an assert()
kdDebug() << k_funcinfo << "m_rawbuffers[" << v4l2buffer.index << "].start: " << (void *)m_rawbuffers[v4l2buffer.index].start << "   Size: " << m_currentbuffer.data.size() << endl;



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
	kdDebug() << " R: " << R << " G: " << G << " B: " << B << " A: " << A <<
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
		if(m_autocolorcorrection)
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
			}
		}


// put frame copy operation here
		kdDebug() <<  k_funcinfo << "exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/*!
    \fn VideoDevice::getFrame(imagebuffer *imgbuffer)
 */
int VideoDevice::getFrame(imagebuffer *imgbuffer)
{
	if(imgbuffer)
	{
		getFrame();
		imgbuffer->height      = m_currentbuffer.height;
		imgbuffer->width       = m_currentbuffer.width;
		imgbuffer->pixelformat = m_currentbuffer.pixelformat;
		imgbuffer->data        = m_currentbuffer.data;
	}
}

/*!
    \fn Kopete::AV::VideoDevice::getImage(const QImage *qimage)
 */
int VideoDevice::getImage(QImage *qimage)
{
    /// @todo implement me
	qimage->create(width(), height(),32, QImage::IgnoreEndian);
	uchar *bits=qimage->bits();
kdDebug() <<  k_funcinfo << "Capturing in " << pixelFormatName(m_currentbuffer.pixelformat) << endl;
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
	}

	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevice::stopCapturing()
 */
int VideoDevice::stopCapturing()
{
    /// @todo implement me
	kdDebug() <<  k_funcinfo << "called." << endl;
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
				}
#endif
				break;
		}
		kdDebug() <<  k_funcinfo << "exited successfuly." << endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


/*!
    \fn VideoDevice::close()
 */
int VideoDevice::close()
{
    /// @todo implement me
	if(isOpen())
	{
		stopCapturing();
		::close(descriptor);
	}
	descriptor = -1;
	return EXIT_SUCCESS;
}

bool VideoDevice::getAutoColorCorrection()
{
	return m_autocolorcorrection;
}

bool VideoDevice::setAutoColorCorrection(bool colorcorrection)
{
	kdDebug() <<  k_funcinfo << "VideoDevice::setAutoColorCorrection(" << colorcorrection << ") called." << endl;
	m_autocolorcorrection = colorcorrection;
	return m_autocolorcorrection;
}


int VideoDevice::pixelFormatCode(pixel_format pixelformat)
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
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
		default:
			return PIXELFORMAT_NONE;	break;
	}
	return PIXELFORMAT_NONE;
}

int VideoDevice::pixelFormatDepth(pixel_format pixelformat)
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
	}
	return 0;
}

QString VideoDevice::pixelFormatName(pixel_format pixelformat)
{
	QString returnvalue;
	switch(pixelformat)
	{
		case PIXELFORMAT_NONE	: returnvalue = "None";			break;
		case PIXELFORMAT_GREY	: returnvalue = "8-bit Grayscale";	break;
		case PIXELFORMAT_RGB332	: returnvalue = "8-bit RGB332";		break;
		case PIXELFORMAT_RGB555	: returnvalue = "16-bit RGB555";	break;
		case PIXELFORMAT_RGB555X: returnvalue = "16-bit RGB555X";	break;
		case PIXELFORMAT_RGB565	: returnvalue = "16-bit RGB565";	break;
		case PIXELFORMAT_RGB565X: returnvalue = "16-bit RGB565X";	break;
		case PIXELFORMAT_RGB24	: returnvalue = "24-bit RGB24";		break;
		case PIXELFORMAT_BGR24	: returnvalue = "24-bit BGR24";		break;
		case PIXELFORMAT_RGB32	: returnvalue = "32-bit RGB32";		break;
		case PIXELFORMAT_BGR32	: returnvalue = "32-bit BGR32";		break;
	}
	return returnvalue;
}

QString VideoDevice::pixelFormatName(int pixelformat)
{
	QString returnvalue;
	returnvalue = "None";
	switch(m_driver)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			switch(pixelformat)
			{
				case V4L2_PIX_FMT_GREY		: returnvalue = "8-bit Grayscale";	break;
				case V4L2_PIX_FMT_RGB332	: returnvalue = "8-bit RGB332";		break;
				case V4L2_PIX_FMT_RGB555	: returnvalue = "16-bit RGB555";	break;
				case V4L2_PIX_FMT_RGB555X	: returnvalue = "16-bit RGB555X";	break;
				case V4L2_PIX_FMT_RGB565	: returnvalue = "16-bit RGB565";	break;
				case V4L2_PIX_FMT_RGB565X	: returnvalue = "16-bit RGB565X";	break;
				case V4L2_PIX_FMT_RGB24		: returnvalue = "24-bit RGB24";		break;
				case V4L2_PIX_FMT_BGR24		: returnvalue = "24-bit BGR24";		break;
				case V4L2_PIX_FMT_RGB32		: returnvalue = "32-bit RGB32";		break;
				case V4L2_PIX_FMT_BGR32		: returnvalue = "32-bit BGR32";		break;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			switch(pixelformat)
			{
				case VIDEO_PALETTE_GREY		: returnvalue = "8-bit Grayscale";	break;
				case VIDEO_PALETTE_HI240	: returnvalue = "8-bit RGB332";		break;
				case VIDEO_PALETTE_RGB555	: returnvalue = "16-bit RGB555";	break;
				case VIDEO_PALETTE_RGB565	: returnvalue = "16-bit RGB565";	break;
				case VIDEO_PALETTE_RGB24	: returnvalue = "24-bit RGB24";		break;
				case VIDEO_PALETTE_RGB32	: returnvalue = "32-bit RGB32";		break;
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
    \fn VideoDevice::initRead()
 */
int VideoDevice::initRead()
{
    /// @todo implement me

	kdDebug() <<  k_funcinfo << "called." << endl;
	if(isOpen())
	{
		m_rawbuffers.resize(1);
		if (m_rawbuffers.size()==0)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}
		kdDebug() <<  k_funcinfo << "m_buffer_size: " << m_buffer_size << endl;

//		m_rawbuffers[0].pixelformat=m_pixelformat;
		m_rawbuffers[0].length = m_buffer_size;
		m_rawbuffers[0].start = (uchar *)malloc (m_buffer_size);

		if (!m_rawbuffers[0].start)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}
		kdDebug() <<  k_funcinfo << "exited successfuly." << endl;
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
		kdDebug() <<  k_funcinfo << full_filename << " Trying to MMAP" << endl;
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
				kdDebug() <<  k_funcinfo << full_filename << " does not support memory mapping" << endl;
				return EXIT_FAILURE;
			}
			else
			{
				return errnoReturn ("VIDIOC_REQBUFS");
			}
		}

		if (req.count < BUFFERS)
		{
			kdDebug() <<  k_funcinfo << "Insufficient buffer memory on " << full_filename << endl;
			return EXIT_FAILURE;
		}

		m_rawbuffers.resize(req.count);

		if (m_rawbuffers.size()==0)
		{
			kdDebug() <<  k_funcinfo <<  "Out of memory" << endl;
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
		kdDebug() <<  k_funcinfo << full_filename << " m_currentbuffer.data.size(): " << m_currentbuffer.data.size() << endl;
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
				kdDebug() <<  k_funcinfo << full_filename << " does not support memory mapping" << endl;
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
				kdDebug() <<  k_funcinfo <<  "Out of memory" << endl;
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



}

}
