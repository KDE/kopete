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
#include "videodevice.h"

#include "bayer.h"
#include "sonix_compress.h"

namespace Kopete {

namespace AV {

VideoDevice::VideoDevice()
{
	kDebug() << "Create a basic video device.";
	descriptor = -1;
	m_streambuffers  = 0;
	m_current_input = 0;
//	kDebug() << "libkopete (avdevice): V4l1Device() exited successfuly";
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

	do
	{
		r = ioctl (descriptor, request, arg);
	}
	while (-1 == r && EINTR == errno);
	return r;
}

/*!
    \fn VideoDevice::errnoReturn(const char* s)
 */
int VideoDevice::errnoReturn(const char* s)
{
	fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
	return EXIT_FAILURE;
}

/*!
    \fn VideoDevice::setFileName(QString name)
 */
int VideoDevice::setFileName(QString filename)
{
	full_filename=filename;
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevice::open()
 */
int VideoDevice::open()
{
	if (isOpen())
	{
		kDebug() << "Device is already open";
		return EXIT_SUCCESS;
	}
	
	descriptor = ::open (QFile::encodeName(full_filename), O_RDWR);

	if (!isOpen())
	{
		kDebug() << "Unable to open file " << full_filename << "Err: "<< errno;
		return EXIT_FAILURE;
	}
	
	kDebug() << "File" << full_filename << "was opened successfuly";

	// Check the device.
	if (EXIT_FAILURE == checkDevice())
	{
		kDebug() << "File " << full_filename << " could not be opened";
		close();
		return EXIT_FAILURE;
	}

	initDevice();
	selectInput(m_current_input);
	
	kDebug() << "exited successfuly";
	
	return EXIT_SUCCESS;
}

bool VideoDevice::isOpen()
{
	return descriptor != -1;
}

int VideoDevice::checkDevice()
{
	return EXIT_SUCCESS;
}

DeviceType VideoDevice::checkDevice(const QString& dev)
{
	kDebug() << "called.";

#ifdef V4L2_CAP_VIDEO_CAPTURE
	int fd = ::open (dev.toUtf8().constData(), O_RDWR);
	if (!fd)
		return UnknownType;

	struct v4l2_capability V4L2_capabilities;
	memset(&V4L2_capabilities, 0, sizeof (V4L2_capabilities));
	
	if (-1 == ioctl (fd, VIDIOC_QUERYCAP, &V4L2_capabilities))
	{
		::close(fd);
		return V4l1;
	}
	else
	{
		::close(fd);
		return V4l2;
	}
#else
	return V4l1;
#endif
}


/*!
    \fn VideoDevice::showDeviceCapabilities()
 */
int VideoDevice::showDeviceCapabilities()
{
	kDebug() << "called.";
	if(!isOpen())
	{
		return EXIT_FAILURE;
	}

	kDebug() << "Device model: " << m_model;

	kDebug() << "Device name : " << m_name;

	kDebug() << "Capabilities:";

	if(canCapture())
		kDebug() << "    Video capture";

	if(canRead())
		kDebug() << "        Read";

	if(canAsyncIO())
		kDebug() << "        Asynchronous input/output";

	if(canStream())
		kDebug() << "        Streaming";

	if(canChromakey())
		kDebug() << "    Video chromakey";

	if(canScale())
		kDebug() << "    Video scales";

	if(canOverlay())
		kDebug() << "    Video overlay";

	kDebug() << "Supported sizes :";
	for (int i = 0; i < m_frameSizes.count(); ++i)
		kDebug() << "    " <<  m_frameSizes.at(i).width() << " x " << m_frameSizes.at(i).height();

	kDebug() << "    Inputs : " << inputs();
	for (int loop = 0; loop < inputs(); loop++)
		kDebug() << "Input " << loop << ": " << m_input[loop].name << " (tuner: " << m_input[loop].hastuner << ")";

	kDebug() << "showDeviceCapabilities() exited successfuly.";

	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevicePool::initDevice()
 */
int VideoDevice::initDevice()
{
	kDebug() << "called.";
	return EXIT_SUCCESS;
}

int VideoDevice::inputs()
{
	return m_input.size();
}

QSize VideoDevice::frameSize()
{
	return currentFrameSize;
}

int VideoDevice::setSize(QSize newSize)
{
	Q_UNUSED(newSize)
	kDebug() << "called.";
	return EXIT_SUCCESS;
}

unsigned int VideoDevice::setPixelFormat(unsigned int newformat)
{
	Q_UNUSED(newformat)
	return 0;
}

/*!
    \fn Kopete::AV::VideoDevice::currentInput()
 */
int VideoDevice::currentInput()
{
	if(isOpen())
	{
		return m_current_input;
	}
	return 0;
}

/*!
    \fn Kopete::AV::VideoDevice::selectInput(int input)
 */
int VideoDevice::selectInput(int newinput)
{
	Q_UNUSED(newinput)
	kDebug() << "called.";
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevice::setInputParameters()
 */
int VideoDevice::setInputParameters()
{
	if( (isOpen()) && (m_current_input < inputs() ) )
	{
		setBrightness( getBrightness() );
		setContrast( getContrast() );
		setSaturation( getSaturation() );
		setWhiteness( getWhiteness() );
		setHue( getHue() );
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/*!
    \fn VideoDevice::startCapturing()
 */
int VideoDevice::startCapturing()
{
	kDebug() << "called.";
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevice::getFrame()
 */
int VideoDevice::getFrame()
{
	kDebug() << "called.";
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevice::getFrame(imagebuffer *imgbuffer)
 */
int VideoDevice::getFrame(imagebuffer *imgbuffer)
{
	if(imgbuffer)
	{
		getFrame(); //Which getFrame will be called ?
		imgbuffer->height      = m_currentbuffer.height;
		imgbuffer->width       = m_currentbuffer.width;
		imgbuffer->pixelformat = m_currentbuffer.pixelformat;
		imgbuffer->data        = m_currentbuffer.data;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/*!
    \fn Kopete::AV::VideoDevice::getImage(const QImage *qimage)
 */
int VideoDevice::getImage(QImage *qimage)
{
	// do NOT delete qimage here, as it is received as a parameter
	if (qimage->size() != frameSize()) 
		*qimage = QImage(frameSize(), QImage::Format_RGB32);

	if (!m_currentbuffer.data.size())
	{
		//there is no data so if we continue something will try access it (as in bug 161536) and crash kopete
		//perhaps we should look at having the interface reflect when the camera isn't available? as it thinks 
		//it is for some reason, though the data size seems to be an ok check
		return EXIT_FAILURE;
	}

	uchar *bits=qimage->bits();
	
//FIXME:shouldn't this be in another class ?
//	It's image processing and conversion, not a video device operation.
	switch(m_currentbuffer.pixelformat)
	{
		case 0	: break;

// Packed RGB formats
		case V4L2_PIX_FMT_RGB332	:
			{
				int step=0;
				for(int loop=0;loop < qimage->numBytes();loop+=4)
				{
					bits[loop]   = (m_currentbuffer.data[step]>>5<<5)+(m_currentbuffer.data[step]>>5<<2)+(m_currentbuffer.data[step]>>6);
					bits[loop+1] = (m_currentbuffer.data[step]>>2<<5)+(m_currentbuffer.data[step]<<3>>5<<2)+(m_currentbuffer.data[step]<<3>>6);
					bits[loop+2] = (m_currentbuffer.data[step]<<6)+(m_currentbuffer.data[step]<<6>>2)+(m_currentbuffer.data[step]<<6>>4)+(m_currentbuffer.data[step]<<6>>6);
					bits[loop+3] = 255;
					step++;
				}
			}
			break;
		case V4L2_PIX_FMT_RGB444	: break;
		case V4L2_PIX_FMT_RGB555	: break;
		case V4L2_PIX_FMT_RGB565	:
			{
				int step=0;
				for(int loop=0;loop < qimage->numBytes();loop+=4)
				{
					bits[loop]   = (m_currentbuffer.data[step]<<3)+(m_currentbuffer.data[step]<<3>>5);
					bits[loop+1] = ((m_currentbuffer.data[step+1])<<5)|m_currentbuffer.data[step]>>5;
					bits[loop+2] = ((m_currentbuffer.data[step+1])&248)+((m_currentbuffer.data[step+1])>>5);
					bits[loop+3] = 255;
					step+=2;
				}
			}
			break;
		case V4L2_PIX_FMT_RGB555X: break;
		case V4L2_PIX_FMT_RGB565X: break;
		case V4L2_PIX_FMT_BGR24	:
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
		case V4L2_PIX_FMT_RGB24	:
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
		case V4L2_PIX_FMT_BGR32	:
			{
				int step=0;
				for(int loop=0;loop < qimage->numBytes();loop+=4)
				{
					bits[loop]   = m_currentbuffer.data[step+2];
					bits[loop+1] = m_currentbuffer.data[step+1];
					bits[loop+2] = m_currentbuffer.data[step];
					bits[loop+3] = m_currentbuffer.data[step+3];
					step+=4;
				}
			}
			break;
		case V4L2_PIX_FMT_RGB32	: memcpy(bits,&m_currentbuffer.data[0], m_currentbuffer.data.size());
			break;

// Bayer RGB format
		case V4L2_PIX_FMT_SBGGR8	:
		{
			unsigned char *d = (unsigned char *) malloc (frameSize().width() * frameSize().height() * 3);
			bayer2rgb24(d, &m_currentbuffer.data.first(), frameSize().width(), frameSize().height());
			int step=0;
			for(int loop=0;loop < qimage->numBytes();loop+=4)
			{
				bits[loop]   = d[step+2];
				bits[loop+1] = d[step+1];
				bits[loop+2] = d[step];
				bits[loop+3] = 255;
				step+=3;
			}
			free(d);
		}
		break;

// YUV formats
		case V4L2_PIX_FMT_GREY	:
			{
				int step=0;
				for(int loop=0;loop < qimage->numBytes();loop+=4)
				{
					bits[loop]   = m_currentbuffer.data[step];
					bits[loop+1] = m_currentbuffer.data[step];
					bits[loop+2] = m_currentbuffer.data[step];
					bits[loop+3] = 255;
					step++;
				}
			}
			break;
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_YUV420:
		case V4L2_PIX_FMT_YUV422P:
		{
			uchar *yptr, *cbptr, *crptr;
			bool halfheight=false;
			bool packed=false;
// Adjust algorythm to specific YUV data arrangements.
			if (m_currentbuffer.pixelformat == V4L2_PIX_FMT_YUV420)
				halfheight=true;
			if (m_currentbuffer.pixelformat == V4L2_PIX_FMT_YUYV)
			{
				yptr = &m_currentbuffer.data[0];
				cbptr = yptr + 1;
				crptr = yptr + 3;
				packed=true;
			}
			else if (m_currentbuffer.pixelformat == V4L2_PIX_FMT_UYVY)
			{
				cbptr = &m_currentbuffer.data[0];
				yptr = cbptr + 1;
				crptr = cbptr + 2;
				packed=true;
			}
			else
			{
				yptr = &m_currentbuffer.data[0];
				cbptr = yptr + (frameSize().width() * frameSize().height());
				crptr = cbptr + (frameSize().width() * frameSize().height() / (halfheight ? 4:2));
			}
	
			for(int y=0; y<frameSize().height(); y++)
			{
// Decode scanline
				for(int x = 0; x < frameSize().width(); x++)
				{
					int c,d,e;

					if (packed)
					{
						c = (yptr[x<<1])-16;
						d = (cbptr[x>>1<<2])-128;
						e = (crptr[x>>1<<2])-128;
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
// Jump to next line
				if (packed)
				{
					yptr += frameSize().width() * 2;
					cbptr += frameSize().width() * 2;
					crptr += frameSize().width() * 2;
				}
				else
				{
					yptr += frameSize().width();
					if (!halfheight || y&1)
					{
						cbptr += frameSize().width() / 2;
						crptr += frameSize().width() / 2;
					}
				}
			}
		}
		break;

// Compressed formats
		case V4L2_PIX_FMT_JPEG	: break;
		case V4L2_PIX_FMT_MPEG	: break;

// Reserved formats
		case V4L2_PIX_FMT_DV	: break;
		case V4L2_PIX_FMT_ET61X251:break;
		case V4L2_PIX_FMT_HI240	: break;
		case V4L2_PIX_FMT_HM12	: break;
		case V4L2_PIX_FMT_MJPEG	: break;
		case V4L2_PIX_FMT_PWC1	: break;
		case V4L2_PIX_FMT_PWC2	: break;
		case V4L2_PIX_FMT_SN9C10X:
		{
			unsigned char *s = new unsigned char [frameSize().width() * frameSize().height()];
			unsigned char *d = new unsigned char [frameSize().width() * frameSize().height() * 3];
			sonix_decompress_init();
			sonix_decompress(frameSize().width(), frameSize().height(), &m_currentbuffer.data.first(), s);
			bayer2rgb24(d, s, frameSize().width(), frameSize().height());
			int step=0;
			for(int loop=0;loop < qimage->numBytes();loop+=4)
			{
				bits[loop]   = d[step+2];
				bits[loop+1] = d[step+1];
				bits[loop+2] = d[step];
				bits[loop+3] = 255;
				step+=3;
			}
			delete[] s;
			delete[] d;
		}
		case V4L2_PIX_FMT_WNVA	: break;
		case V4L2_PIX_FMT_YYUV	: break;
	}

// Proccesses image for automatic Brightness/Contrast/Color correction
	if (getAutoBrightnessContrast()||getAutoColorCorrection())
	{
		unsigned long long R=0, G=0, B=0, A=0, global=0;
		int Rmax=0, Gmax=0, Bmax=0, Amax=0, globalmax=0;
		int Rmin=255, Gmin=255, Bmin=255, Amin=255, globalmin=255;
		int Rrange=255, Grange=255, Brange=255;

// Finds minimum and maximum intensity for each color component
		for(int loop=0;loop < qimage->numBytes();loop+=4)
		{
			R+=bits[loop];
			G+=bits[loop+1];
			B+=bits[loop+2];
//			A+=bits[loop+3];
			if (bits[loop]   < Rmin) Rmin = bits[loop];
			if (bits[loop+1] < Gmin) Gmin = bits[loop+1];
			if (bits[loop+2] < Bmin) Bmin = bits[loop+2];
//			if (bits[loop+3] < Amin) Amin = bits[loop+3];
			if (bits[loop]   > Rmax) Rmax = bits[loop];
			if (bits[loop+1] > Gmax) Gmax = bits[loop+1];
			if (bits[loop+2] > Bmax) Bmax = bits[loop+2];
//			if (bits[loop+3] > Amax) Amax = bits[loop+3];
		}
		global = R + G + B;
// Finds overall minimum and maximum intensity
		if (Rmin > Gmin) globalmin = Gmin; else globalmin = Rmin; if (Bmin < globalmin) globalmin = Bmin;
		if (Rmax > Gmax) globalmax = Rmax; else globalmax = Gmax; if (Bmax > globalmax) globalmax = Bmax;
// If no color correction should be performed, simply level all the intensities so they're just the same.
// In fact color correction should use the R, G and B variables to detect color deviation and "bump up" the saturation,
// but it's computationally more expensive and the current way returns better results to the user.
		if(!getAutoColorCorrection())
		{
			Rmin = globalmin ; Rmax = globalmax;
			Gmin = globalmin ; Gmax = globalmax;
			Bmin = globalmin ; Bmax = globalmax;
//			Amin = globalmin ; Amax = globalmax;
		}
// Calculates ranges and prevent a division by zero later on.
			Rrange = Rmax - Rmin; if (Rrange == 0) Rrange = 255;
			Grange = Gmax - Gmin; if (Grange == 0) Grange = 255;
			Brange = Bmax - Bmin; if (Brange == 0) Brange = 255;
//			Arange = Amax - Amin; if (Arange == 0) Arange = 255;

		kDebug() << " R: " << R << " G: " << G << " B: " << B << " A: " << A << " global: " << global <<
			" Rmin: " << Rmin << " Gmin: " << Gmin << " Bmin: " << Bmin << " Amin: " << Amin << " globalmin: " << globalmin <<
			" Rmax: " << Rmax << " Gmax: " << Gmax << " Bmax: " << Bmax << " Amax: " << Amax << " globalmax: " << globalmax ;

		for(int loop=0;loop < qimage->numBytes();loop+=4)
		{
			bits[loop]   = (bits[loop]   - Rmin) * 255 / (Rrange);
			bits[loop+1] = (bits[loop+1] - Gmin) * 255 / (Grange);
			bits[loop+2] = (bits[loop+2] - Bmin) * 255 / (Brange);
//			bits[loop+3] = (bits[loop+3] - Amin) * 255 / (Arange);
		}
	}
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevice::stopCapturing()
 */
int VideoDevice::stopCapturing()
{
	kDebug() << "called.";
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::close()
 */
int VideoDevice::close()
{
	kDebug() << " called.";
	if(isOpen())
	{
		kDebug() << " Device is open. Trying to properly shutdown the device.";
		stopCapturing();
		int ret = ::close(descriptor);
		kDebug() << "::close() returns " << ret;
	}
	m_frameSizes.clear();
	descriptor = -1;
	return EXIT_SUCCESS;
}

float VideoDevice::getBrightness()
{
	if (m_current_input < m_input.size() )
		return m_input[m_current_input].getBrightness();
	else
		return 0;
}

float VideoDevice::setBrightness(float brightness)
{
	Q_UNUSED(brightness)
	kDebug() << "called.";
	return getBrightness();
}

float VideoDevice::getContrast()
{
	if (m_current_input < m_input.size() )
		return m_input[m_current_input].getContrast();
	else
		return 0;
}

float VideoDevice::setContrast(float contrast)
{
	Q_UNUSED(contrast)
	kDebug() << "called.";
	return getContrast();
}

float VideoDevice::getSaturation()
{
	if (m_current_input < m_input.size() )
		return m_input[m_current_input].getSaturation();
	else
		return 0;
}

float VideoDevice::setSaturation(float saturation)
{
	Q_UNUSED(saturation)
	kDebug() << "called.";
	return getSaturation();
}

float VideoDevice::getWhiteness()
{
  if (m_current_input < m_input.size() )
	return m_input[m_current_input].getWhiteness();
  else
	return 0;
}

float VideoDevice::setWhiteness(float whiteness)
{
	Q_UNUSED(whiteness)
	kDebug() << "called.";
	return getWhiteness();
}

float VideoDevice::getHue()
{
  if (m_current_input < m_input.size() )
	return m_input[m_current_input].getHue();
  else
	return 0;
}

float VideoDevice::setHue(float hue)
{
	Q_UNUSED(hue)
	kDebug() << "called.";
	return getHue();
}


bool VideoDevice::getAutoBrightnessContrast()
{
  if (m_current_input < m_input.size() )
	return m_input[m_current_input].getAutoBrightnessContrast();
  else
	return false;
}

bool VideoDevice::setAutoBrightnessContrast(bool brightnesscontrast)
{
	kDebug() << "VideoDevice::setAutoBrightnessContrast(" << brightnesscontrast << ") called.";
	if (m_current_input < m_input.size() ) 
	  {
		m_input[m_current_input].setAutoBrightnessContrast(brightnesscontrast);
		return m_input[m_current_input].getAutoBrightnessContrast();
	  }
	else
	  return false;
   
}

bool VideoDevice::getAutoColorCorrection()
{
  if (m_current_input < m_input.size() )
	return m_input[m_current_input].getAutoColorCorrection();
  else
	return false;
}

bool VideoDevice::setAutoColorCorrection(bool colorcorrection)
{
	kDebug() << "VideoDevice::setAutoColorCorrection(" << colorcorrection << ") called.";
	if (m_current_input < m_input.size() )
	  {
		m_input[m_current_input].setAutoColorCorrection(colorcorrection);
		return m_input[m_current_input].getAutoColorCorrection();
	  }
	else
	  return false;
}

bool VideoDevice::getImageAsMirror()
{
  if (m_current_input < m_input.size() )
	return m_input[m_current_input].getImageAsMirror();
  else
	return false;
}

bool VideoDevice::setImageAsMirror(bool imageasmirror)
{
	kDebug() << "VideoDevice::setImageAsMirror(" << imageasmirror << ") called.";
	if (m_current_input < m_input.size() ) 
	  {
		m_input[m_current_input].setImageAsMirror(imageasmirror);
		return m_input[m_current_input].getImageAsMirror();
	  }
	else
	  return false;
}

int VideoDevice::pixelFormatDepth(unsigned int pixelformat)
{
	switch(pixelformat)
	{
		case 0			: return 0;	break;

// Packed RGB formats
		case V4L2_PIX_FMT_RGB332	: return 8;	break;
		case V4L2_PIX_FMT_RGB444	: return 16;	break;
		case V4L2_PIX_FMT_RGB555	: return 16;	break;
		case V4L2_PIX_FMT_RGB565	: return 16;	break;
		case V4L2_PIX_FMT_RGB555X: return 16;	break;
		case V4L2_PIX_FMT_RGB565X: return 16;	break;
		case V4L2_PIX_FMT_BGR24	: return 24;	break;
		case V4L2_PIX_FMT_RGB24	: return 24;	break;
		case V4L2_PIX_FMT_BGR32	: return 32;	break;
		case V4L2_PIX_FMT_RGB32	: return 32;	break;

// Bayer RGB format
		case V4L2_PIX_FMT_SBGGR8	: return 0;	break;

// YUV formats
		case V4L2_PIX_FMT_GREY	: return 8;	break;
		case V4L2_PIX_FMT_YUYV	: return 16;	break;
		case V4L2_PIX_FMT_UYVY	: return 16;	break;
		case V4L2_PIX_FMT_YUV420: return 16;	break;
		case V4L2_PIX_FMT_YUV422P: return 16;	break;

// Compressed formats
		case V4L2_PIX_FMT_JPEG	: return 0;	break;
		case V4L2_PIX_FMT_MPEG	: return 0;	break;

// Reserved formats
		case V4L2_PIX_FMT_DV	: return 0;	break;
		case V4L2_PIX_FMT_ET61X251:return 0;	break;
		case V4L2_PIX_FMT_HI240	: return 8;	break;
		case V4L2_PIX_FMT_HM12	: return 0;	break;
		case V4L2_PIX_FMT_MJPEG	: return 0;	break;
		case V4L2_PIX_FMT_PWC1	: return 0;	break;
		case V4L2_PIX_FMT_PWC2	: return 0;	break;
		case V4L2_PIX_FMT_SN9C10X: return 0;	break;
		case V4L2_PIX_FMT_WNVA	: return 0;	break;
		case V4L2_PIX_FMT_YYUV	: return 0;	break;
	}
	return 0;
}

QString VideoDevice::pixelFormatName(unsigned int pixelformat)
{
	QString returnvalue;
	returnvalue = "None";
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef V4L2_CAP_VIDEO_CAPTURE
	switch(pixelformat)
	{
		case 0				: returnvalue = "None";			break;

// Packed RGB formats
		case V4L2_PIX_FMT_RGB332	: returnvalue = "8-bit RGB332";		break;
#if defined( V4L2_PIX_FMT_RGB444 )
		case V4L2_PIX_FMT_RGB444	: returnvalue = "8-bit RGB444";		break;
#endif
		case V4L2_PIX_FMT_RGB555	: returnvalue = "16-bit RGB555";	break;
		case V4L2_PIX_FMT_RGB565	: returnvalue = "16-bit RGB565";	break;
		case V4L2_PIX_FMT_RGB555X	: returnvalue = "16-bit RGB555X";	break;
		case V4L2_PIX_FMT_RGB565X	: returnvalue = "16-bit RGB565X";	break;
		case V4L2_PIX_FMT_BGR24		: returnvalue = "24-bit BGR24";		break;
		case V4L2_PIX_FMT_RGB24		: returnvalue = "24-bit RGB24";		break;
		case V4L2_PIX_FMT_BGR32		: returnvalue = "32-bit BGR32";		break;
		case V4L2_PIX_FMT_RGB32		: returnvalue = "32-bit RGB32";		break;

// Bayer RGB format
		case V4L2_PIX_FMT_SBGGR8	: returnvalue = "Bayer RGB format";	break;

// YUV formats
		case V4L2_PIX_FMT_GREY		: returnvalue = "8-bit Grayscale";	break;
		case V4L2_PIX_FMT_YUYV		: returnvalue = "Packed YUV 4:2:2";	break;
		case V4L2_PIX_FMT_UYVY		: returnvalue = "Packed YVU 4:2:2";	break;
		case V4L2_PIX_FMT_YUV420	: returnvalue = "Planar YUV 4:2:0";	break;
		case V4L2_PIX_FMT_YUV422P	: returnvalue = "Planar YUV 4:2:2";	break;

// Compressed formats
		case V4L2_PIX_FMT_JPEG		: returnvalue = "JPEG image";		break;
		case V4L2_PIX_FMT_MPEG		: returnvalue = "MPEG stream";		break;

// Reserved formats
		case V4L2_PIX_FMT_DV		: returnvalue = "DV Unknown";		break;
		case V4L2_PIX_FMT_ET61X251	: returnvalue = "ET61X251";		break;
		case V4L2_PIX_FMT_HI240		: returnvalue = "8-bit HI240 (RGB332)";	break;
#if defined( V4L2_PIX_FMT_HM12 )
		case V4L2_PIX_FMT_HM12		: returnvalue = "Packed YUV 4:2:2";	break;
#endif
		case V4L2_PIX_FMT_MJPEG		: returnvalue = "MJPEG";		break;
		case V4L2_PIX_FMT_PWC1		: returnvalue = "PWC1";			break;
		case V4L2_PIX_FMT_PWC2		: returnvalue = "PWC2";			break;
		case V4L2_PIX_FMT_SN9C10X	: returnvalue = "SN9C102";		break;
		case V4L2_PIX_FMT_WNVA		: returnvalue = "Winnov Videum";	break;
		case V4L2_PIX_FMT_YYUV		: returnvalue = "YYUV (unknown)";	break;
#endif
		case VIDEO_PALETTE_GREY		: returnvalue = "8-bit Grayscale";	break;
		case VIDEO_PALETTE_HI240	: returnvalue = "RGB332";		break;
		case VIDEO_PALETTE_RGB555	: returnvalue = "RGB555";		break;
		case VIDEO_PALETTE_RGB565	: returnvalue = "RGB565";		break;
		case VIDEO_PALETTE_RGB24	: returnvalue = "RGB24";		break;
		case VIDEO_PALETTE_RGB32	: returnvalue = "RGB32";		break;
		case VIDEO_PALETTE_YUYV		: returnvalue = "YUYV";			break;
		case VIDEO_PALETTE_UYVY		: returnvalue = "UYVY";			break;
		case VIDEO_PALETTE_YUV420	:
		case VIDEO_PALETTE_YUV420P	: returnvalue = "YUV420P";		break;
		case VIDEO_PALETTE_YUV422P	: returnvalue = "YUV422P";		break;
#endif
		default:
			break;
	}
	return returnvalue;
}

int VideoDevice::detectPixelFormats()
{
	return 0;
}

QString VideoDevice::signalStandardName(int standard)
{
	QString returnvalue;
	returnvalue = "None";
	switch(standard)
	{
#if defined(__linux__) && defined(ENABLE_AV)
#ifdef V4L2_CAP_VIDEO_CAPTURE
		case V4L2_STD_PAL_B	: returnvalue = "PAL-B";	break;
		case V4L2_STD_PAL_B1	: returnvalue = "PAL_B1";	break;
		case V4L2_STD_PAL_G	: returnvalue = "PAL_G";	break;
		case V4L2_STD_PAL_H	: returnvalue = "PAL_H";	break;
		case V4L2_STD_PAL_I	: returnvalue = "PAL_I";	break;
		case V4L2_STD_PAL_D	: returnvalue = "PAL_D";	break;
		case V4L2_STD_PAL_D1	: returnvalue = "PAL_D1";	break;
		case V4L2_STD_PAL_K	: returnvalue = "PAL_K";	break;
		case V4L2_STD_PAL_M	: returnvalue = "PAL_M";	break;
		case V4L2_STD_PAL_N	: returnvalue = "PAL_N";	break;
		case V4L2_STD_PAL_Nc	: returnvalue = "PAL_Nc";	break;
		case V4L2_STD_PAL_60	: returnvalue = "PAL_60";	break;
		case V4L2_STD_NTSC_M	: returnvalue = "NTSC_M";	break;
		case V4L2_STD_NTSC_M_JP	: returnvalue = "NTSC_M_JP";	break;
		case V4L2_STD_NTSC_443	: returnvalue = "NTSC_443";	break; // Commented out because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
		case V4L2_STD_NTSC_M_KR	: returnvalue = "NTSC_M_KR";	break; // Commented out because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
		case V4L2_STD_SECAM_B	: returnvalue = "SECAM_B";	break;
		case V4L2_STD_SECAM_D	: returnvalue = "SECAM_D";	break;
		case V4L2_STD_SECAM_G	: returnvalue = "SECAM_G";	break;
		case V4L2_STD_SECAM_H	: returnvalue = "SECAM_H";	break;
		case V4L2_STD_SECAM_K	: returnvalue = "SECAM_K";	break;
		case V4L2_STD_SECAM_K1	: returnvalue = "SECAM_K1";	break;
		case V4L2_STD_SECAM_L	: returnvalue = "SECAM_L";	break;
		case V4L2_STD_SECAM_LC	: returnvalue = "SECAM_LC";	break;

		case V4L2_STD_ATSC_8_VSB: returnvalue = "ATSC_8_VSB";break;
		case V4L2_STD_ATSC_16_VSB:returnvalue = "ATSC_16_VSB";break;

		case V4L2_STD_PAL_BG	: returnvalue = "PAL_BG";	break;
		case V4L2_STD_PAL_DK	: returnvalue = "PAL_DK";	break;
		case V4L2_STD_PAL	: returnvalue = "PAL";	break;
		case V4L2_STD_NTSC	: returnvalue = "NTSC";	break;
		case V4L2_STD_SECAM_DK	: returnvalue = "SECAM_DK";	break;
		case V4L2_STD_SECAM	: returnvalue = "SECAM";	break;

		case V4L2_STD_MN	: returnvalue = "MN";	break;
		case V4L2_STD_B		: returnvalue = "B";		break;
		case V4L2_STD_GH	: returnvalue = "GH";	break;
		case V4L2_STD_DK	: returnvalue = "DK";	break;

		case V4L2_STD_525_60	: returnvalue = "525_60";	break;
		case V4L2_STD_625_50	: returnvalue = "625_50";	break;
		case V4L2_STD_ATSC	: returnvalue = "ATSC";	break;

		case V4L2_STD_UNKNOWN	: returnvalue = "UNKNOWN";	break;
		case V4L2_STD_ALL	: returnvalue = "ALL";	break;
#endif
/*		case VIDEO_MODE_PAL	: returnvalue = "PAL";	break;
		case VIDEO_MODE_NTSC	: returnvalue = "NTSC";	break;
		case VIDEO_MODE_SECAM	: returnvalue = "SECAM";	break;
		case VIDEO_MODE_AUTO	: returnvalue = "ALL";	break;	// It must be disabled until I find a correct way to handle those non-standard bttv modes
		case VIDEO_MODE_PAL_M	: returnvalue = "PAL_M";	break;	// Undocumented value found to be compatible with V4L bttv driver
		case VIDEO_MODE_PAL_N	: returnvalue = "PAL_N";	break;	// Undocumented value found to be compatible with V4L bttv driver
		case VIDEO_MODE_NTSC_JP	: returnvalue = "NTSC_M_JP";	break;	// Undocumented value found to be compatible with V4L bttv driver*/

#endif
	}
	return returnvalue;
}

/*!
    \fn VideoDevice::detectSignalStandards()
// this must be done once for each _input_.
 */
int VideoDevice::detectSignalStandards()
{
	return EXIT_SUCCESS;
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

void VideoDevice::setUdi( const QString & udi )
{
    m_udi = udi;
}

QString VideoDevice::udi() const
{
    return m_udi;
}

}

}
