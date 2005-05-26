/*
    videodevice.cpp  -  Kopete Video Device Low-level Support

    Copyright (c) 2005 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

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

#include <assert.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <kdebug.h>
#include <klocale.h>
#include <qdir.h>

#include "videodevice.h"
#include "videodevicepool.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

VideoDevicePool *VideoDevicePool::s_self = NULL;

VideoDevicePool* VideoDevicePool::self()
{
//	kdDebug() << "libkopete (avdevice): self() called" << endl;
	if (s_self == NULL)
	{
		s_self = new VideoDevicePool;
	}
//	kdDebug() << "libkopete (avdevice): self() exited successfuly" << endl;
	return s_self;
}

VideoDevicePool::VideoDevicePool()
{
}


VideoDevicePool::~VideoDevicePool()
{
}




/*!
    \fn VideoDevicePool::open()
 */
int VideoDevicePool::open()
{
    /// @todo implement me

	return m_videodevice[currentDevice()].open();
}

/*!
    \fn VideoDevicePool::open(int device)
 */
int VideoDevicePool::open(unsigned int device)
{
    /// @todo implement me
	kdDebug() << "libkopete (avdevice): open(" << device << ") called." << endl;
	if(device >= m_videodevice.size())
	{
		kdDebug() << "libkopete (avdevice): open(" << device <<"): Device does not exist." << endl;
		return EXIT_FAILURE;
	}
	close();
	kdDebug() << "libkopete (avdevice): open(" << device << ") Setting m_current_Device to " << device << endl;
	m_current_device = device;
	kdDebug() << "libkopete (avdevice): open(" << device << ") Calling open()." << endl;
	open();
//	m_videodevice[currentDevice()].initDevice();
	kdDebug() << "libkopete (avdevice): open(" << device << ") exited successfuly." << endl;
	return EXIT_SUCCESS;
}

bool VideoDevicePool::isOpen()
{
	return m_videodevice[currentDevice()].isOpen();
}

/*!
    \fn VideoDevicePool::processImage(const void *p)
 */
int VideoDevicePool::processImage(const void * p)
{
	return m_videodevice[currentDevice()].processImage(p);
}


/*!
    \fn VideoDevicePool::getFrame()
 */
int VideoDevicePool::getFrame()
{
    /// @todo implement me
	readFrame();
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevicePool::showDeviceCapabilities(int device)
 */
int VideoDevicePool::showDeviceCapabilities(unsigned int device)
{
	return m_videodevice[device].showDeviceCapabilities();
}

int VideoDevicePool::width()
{
	return m_videodevice[currentDevice()].width();
}

int VideoDevicePool::minWidth()
{
	return m_videodevice[currentDevice()].minWidth();
}

int VideoDevicePool::maxWidth()
{
	return m_videodevice[currentDevice()].maxWidth();
}

int VideoDevicePool::height()
{
	return m_videodevice[currentDevice()].height();
}

int VideoDevicePool::minHeight()
{
	return m_videodevice[currentDevice()].minHeight();
}

int VideoDevicePool::maxHeight()
{
	return m_videodevice[currentDevice()].maxHeight();
}

int VideoDevicePool::setSize( int newwidth, int newheight)
{
	return m_videodevice[currentDevice()].setSize(newwidth, newheight);
}

/*!
    \fn VideoDevicePool::close()
 */
int VideoDevicePool::close()
{
    /// @todo implement me
	if(currentDevice() < m_videodevice.size())
		return m_videodevice[currentDevice()].close();
	kdDebug() << "libkopete (avdevice): VideoDevicePool::close() Current device out of range." << endl;
	return EXIT_FAILURE;
}

/*!
    \fn VideoDevicePool::startCapturing()
 */
int VideoDevicePool::startCapturing()
{
	return m_videodevice[currentDevice()].startCapturing();
}


/*!
    \fn VideoDevicePool::stopCapturing()
 */
int VideoDevicePool::stopCapturing()
{
	return m_videodevice[currentDevice()].stopCapturing();
}


/*!
    \fn VideoDevicePool::readFrame()
 */
int VideoDevicePool::readFrame()
{
	return m_videodevice[currentDevice()].readFrame();
}

/*!
    \fn VideoDevicePool::getQImage(QImage *qimage)
 */
int VideoDevicePool::getImage(QImage *qimage)
{
	return m_videodevice[currentDevice()].getImage(qimage);
}

/*!
    \fn Kopete::AV::VideoDevicePool::selectInput(int input)
 */
int VideoDevicePool::selectInput(int newinput)
{
	return m_videodevice[currentDevice()].selectInput(newinput);
}

/*!
    \fn Kopete::AV::VideoDevicePool::fillInputKComboBox(KComboBox *combobox)
 */
int VideoDevicePool::fillDeviceKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kdDebug() << "libkopete (avdevice): fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_videodevice.size()>0)
	{
		for (unsigned int loop=0; loop < m_videodevice.size(); loop++)
		{
			combobox->insertItem(m_videodevice[loop].name);
			kdDebug() << "libkopete (avdevice): DeviceKCombobox: Added device " << loop << ": " << m_videodevice[loop].name << endl;
		}
		combobox->setCurrentItem(currentDevice());
	}
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevicePool::fillInputKComboBox(KComboBox *combobox)
 */
int VideoDevicePool::fillInputKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kdDebug() << "libkopete (avdevice): fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_videodevice[currentDevice()].inputs()>0)
		for (unsigned int loop=0; loop < m_videodevice[currentDevice()].inputs(); loop++)
		{
			combobox->insertItem(m_videodevice[currentDevice()].input[loop].name);
			kdDebug() << "libkopete (avdevice): InputKCombobox: Added input " << loop << ": " << m_videodevice[currentDevice()].input[loop].name << " (tuner: " << m_videodevice[currentDevice()].input[loop].hastuner << ")" << endl;
		}
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevicePool::scanDevices()
 */
int VideoDevicePool::scanDevices()
{
    /// @todo implement me

	kdDebug() << "libkopete (avdevice): scanDevices() called" << endl;
	QDir videodevice_dir;
	const QString videodevice_dir_path=QString::fromLocal8Bit("/dev/v4l/");
	const QString videodevice_dir_filter=QString::fromLocal8Bit("video*");
	VideoDevice videodevice;

	videodevice_dir.setPath(videodevice_dir_path);
	videodevice_dir.setNameFilter(videodevice_dir_filter);
        videodevice_dir.setFilter( QDir::System | QDir::NoSymLinks | QDir::Readable | QDir::Writable );
        videodevice_dir.setSorting( QDir::Name );

	const QFileInfoList *list = videodevice_dir.entryInfoList();

	if (!list)
		return EXIT_FAILURE;

	QFileInfoListIterator fileiterator ( *list );
	QFileInfo *fileinfo;
//	QFile file;

	m_videodevice.clear();
	kdDebug() << "libkopete (avdevice): scanDevices() called" << endl;
	while ( (fileinfo = fileiterator.current()) != 0 )
	{
		videodevice.setFileName(fileinfo->absFilePath());
		kdDebug() << "libkopete (avdevice): Found device " << videodevice.full_filename << endl;
		videodevice.open(); // It should be opened with O_NONBLOCK (it's a FIFO) but I dunno how to do it using QFile
		if(videodevice.isOpen())
		{
			kdDebug() << "libkopete (avdevice): File " << videodevice.full_filename << " was opened successfuly" << endl;
			videodevice.close();
			m_videodevice.push_back(videodevice);
		}
		++fileiterator;
	}
	kdDebug() << "libkopete (avdevice): scanDevices() exited successfuly" << endl;
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevicePool::currentDevice()
 */
unsigned int VideoDevicePool::currentDevice()
{
    /// @todo implement me
	return m_current_device;
}

/*!
    \fn Kopete::AV::VideoDevicePool::currentInput()
 */
int VideoDevicePool::currentInput()
{
    /// @todo implement me
	return m_videodevice[currentDevice()].currentInput();
}

/*!
    \fn Kopete::AV::VideoDevicePool::currentInput()
 */
unsigned int VideoDevicePool::inputs()
{
    /// @todo implement me
	return m_videodevice[currentDevice()].inputs();
}



}

}
