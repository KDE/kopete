/*
    dummyvideodevice.cpp  -  Dummy Video Device

    Copyright (c) 2009 by Alan Jones <skyphyr@gmail.com>

    Kopete    (c) 2002-2009      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    * This program is distributed in the hope that it will be useful,       *
    * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
    * GNU General Public License for more details.                          *
    *                                                                       *
    * You should have received a copy of the GNU Lesser General Public      *
    * License along with this program. If not, see                          *
    * <http://www.gnu.org/licenses/>.                                       *
    *************************************************************************
*/

#include "dummyvideodevice.h"
#include "videoinput.h"

namespace Kopete {

namespace AV {

DummyVideoDevice::DummyVideoDevice()
	{
	//need to set a bunch of defaults for members
	m_pixelformat = V4L2_PIX_FMT_RGB32;
	m_input.append(VideoInput());
	
	m_frameSizes << QSize(160, 120);
	m_frameSizes << QSize(1280, 960);
	setSize(QSize(320, 240));
	//let's be opened by default seeing this is a fallback 
	opened = true;
	}

int DummyVideoDevice::open()
	{
	//wow - we always succeed in opening our dummy device
	opened = true;
	return EXIT_SUCCESS;
	}
	
bool DummyVideoDevice::isOpen()
	{
	//we'll track it just incase there is situations where it's expected not to be open
	return opened;
	}
	
int DummyVideoDevice::checkDevice()
	{
	//again this is just incase anything ever relies on it failing when closed
	if(isOpen())
		{
		//need to check which, if any, of these need to be enabled for this to function properly
		m_videocapture=false;
		m_videochromakey=false;
		m_videoscale=false;
		m_videooverlay=false;
		m_videoread=false;
		m_videoasyncio=false;
		m_videostream=false;

		return EXIT_SUCCESS;
		}
	return EXIT_FAILURE;
	}
	
int DummyVideoDevice::initDevice()
	{
	//again this is just incase anything ever relies on it failing when closed
	return isOpen() ? EXIT_SUCCESS : EXIT_FAILURE;
	}
	
int DummyVideoDevice::setSize(QSize newSize)
	{
	if (isOpen())
		{
		if (newSize.width() > m_frameSizes.last().width())
			newSize.setWidth(m_frameSizes.last().width());
		if (newSize.height() > m_frameSizes.last().height())
			newSize.setHeight(m_frameSizes.last().height());

		if (newSize.width() < m_frameSizes.first().width())
			newSize.setWidth(m_frameSizes.first().width());
		if (newSize.height() < m_frameSizes.first().height())
			newSize.setHeight(m_frameSizes.first().height());

		currentFrameSize = newSize;

		m_currentbuffer.width = currentFrameSize.width();
		m_currentbuffer.height = currentFrameSize.height();
		m_currentbuffer.pixelformat = m_pixelformat;
		
		//let's fill the buffer up with something
		//TODO: replace this with an .svg that says there is no camera available
		m_currentbuffer.data.resize(currentFrameSize.width() * currentFrameSize.height() * 4);
		for (int i=0; i<m_currentbuffer.data.size(); i++)
			{
			m_currentbuffer.data[i] = 255;
			}
		
		return EXIT_SUCCESS;
		}
	return EXIT_FAILURE;
	}
	
unsigned int DummyVideoDevice::setPixelFormat(unsigned int newformat)
	{
	int ret = 0;
	
	if (newformat == V4L2_PIX_FMT_RGB32){
		m_pixelformat = newformat;
		ret = m_pixelformat;
		}
	
	return ret;
	}
	
int DummyVideoDevice::startCapturing()
	{
	//again this is just incase anything ever relies on it failing when closed
	return isOpen() ? EXIT_SUCCESS : EXIT_FAILURE;
	}
	
int DummyVideoDevice::getFrame()
	{
	//again this is just incase anything ever relies on it failing when closed
	return isOpen() ? EXIT_SUCCESS : EXIT_FAILURE;
	}
	
int DummyVideoDevice::getFrame(imagebuffer *imgbuffer)
	{
	if(imgbuffer)
	{
		imgbuffer->height      = m_currentbuffer.height;
		imgbuffer->width       = m_currentbuffer.width;
		imgbuffer->pixelformat = m_currentbuffer.pixelformat;
		imgbuffer->data        = m_currentbuffer.data;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
	}
	
int DummyVideoDevice::getImage(QImage *qimage)
	{
	// do NOT delete qimage here, as it is received as a parameter
	if (qimage->size() != frameSize())
		*qimage = QImage(frameSize(), QImage::Format_RGB32);

	uchar *bits=qimage->bits();
	memcpy(bits,&m_currentbuffer.data[0], m_currentbuffer.data.size());
	
	return EXIT_SUCCESS;
	}
	
int DummyVideoDevice::stopCapturing()
	{
	//again this is just incase anything ever relies on it failing when closed
	return isOpen() ? EXIT_SUCCESS : EXIT_FAILURE;
	}
	
int DummyVideoDevice::close()
	{
	//we always manage to close it too - amazing :)
	opened = false;
	return EXIT_SUCCESS;
	}

}

}
