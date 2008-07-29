 /*
  * jinglemediamanager.cpp - A media manager to use with jingle.
  *
  * Copyright (c) 2008 by Detlev Casanova <detlev.casanova@gmail.com>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */
#include "jinglemediamanager.h"

#include <QList>
#include <QDomElement>
#include <KDebug>

JingleMediaManager::JingleMediaManager()
{
	//findDevice();
	timer = 0;
}

JingleMediaManager::~JingleMediaManager()
{
	kDebug() << "MEDIA MANAGER DESTROYED !!";
}

/*
 * Find audio input and output devices.
 */
int JingleMediaManager::findDevice()
{
	m_inputDevice = 0L;
	m_outputDevice = 0L;
	QList<Solid::Device> devicesList = Solid::Device::listFromType(Solid::DeviceInterface::AudioInterface, QString());
        int i = 0, ret = 0;
        while (i <= devicesList.count())
        {
                Solid::AudioInterface *device = devicesList[i].as<Solid::AudioInterface>();
                if (device->deviceType() == Solid::AudioInterface::AudioInput)
                {
			if (m_inputDevice != 0L)
				delete m_inputDevice;
			kDebug() << "Microphone found. The driver used is " << device->driver();
			m_inputDevice = device;
                	i++;
			ret++;
			continue;
                }
                
		if (device->deviceType() == Solid::AudioInterface::AudioOutput)
                {
			if (m_outputDevice != 0L)
				delete m_outputDevice;
			kDebug() << "Sound card found. The driver used is " << device->driver();
			m_outputDevice = device;
                	i++;
			ret++;
			continue;
                }
                
		delete device;
                i++;
        }

	return ret;

}

QList<QDomElement> JingleMediaManager::payloads()
{
	QList<QDomElement> ret;
	return ret;
}

void JingleMediaManager::startAudioStreaming()
{
	if (timer == 0)
	{
		timer = new QTimer();
		timer->setInterval(2000);
		connect(timer, SIGNAL(timeout()), this, SIGNAL(audioReadyRead()));
	}
	if (!timer->isActive())
		timer->start();
	
}

QByteArray JingleMediaManager::data()
{
	return QByteArray("Data for 2000 ms, you should not try to play this !!");
}

