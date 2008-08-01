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
#include <QStringList>
#include <QDomElement>
#include <KDebug>

/*********************
 * JingleMediaSession
 *********************/

JingleMediaSession::JingleMediaSession()
{

}

JingleMediaSession::~JingleMediaSession()
{

}

void JingleMediaSession::setPayloadType(const QDomElement& payload)
{
	int pID = payload.attribute("id").toInt();
	if (pID >= 0 && pID <= 23) // This is audio payload.
		m_type = Audio;
	else /*if (pID >= 24 && pID <= 34)*/// Let's say other ones are video... // This is video payload.
		m_type = Video;
	
	m_payload = payload;
}

void JingleMediaSession::setInputDevice(const Solid::DeviceInterface* device)
{
	Q_UNUSED(device)
	if (m_type == Audio)
	{
		
	}
}

void JingleMediaSession::setOutputDevice(const Solid::DeviceInterface* device)
{
	Q_UNUSED(device)
}

/*********************
 * JingleMediaManager
 *********************/

JingleMediaManager::JingleMediaManager()
{
	findDevices();
	timer = 0;
}

JingleMediaManager::~JingleMediaManager()
{
	kDebug() << "Media manager destroyed";
}

/*
 * Find audio input and output devices.
 */
void JingleMediaManager::findDevices()
{
	QList<Solid::Device> deviceList = Solid::Device::listFromType(Solid::DeviceInterface::AudioInterface, QString());
	for (int i = 0; i < deviceList.count(); i++)
        {
                Solid::AudioInterface *device = deviceList[i].as<Solid::AudioInterface>();
                if (device->deviceType() == Solid::AudioInterface::AudioInput)
		{
			kDebug() << "Microphone found. The driver used is " << device->driver();
			m_microphones << device;
		}

		if (device->deviceType() == Solid::AudioInterface::AudioOutput)
		{
			kDebug() << "Sound card found. The driver used is " << device->driver();
			m_audioOutputs << device;
		}
	}

	deviceList = Solid::Device::listFromType(Solid::DeviceInterface::Video, QString());
	for (int i = 0; i < deviceList.count(); i++)
		m_videoInputs << deviceList[i].as<Solid::Video>();
}

void JingleMediaManager::startVideoStreaming()
{

}

QList<Solid::Video*> JingleMediaManager::videoDevices()
{
	// TODO:returns a list with devices
	//	The content of each item should be the name of the device (/dev/video0)
	//	But it could also be Phonon objects (I still don't know which ones)
	//	But it could also be Solid objects (I still don't know which ones) <-- This one will be used.
	return m_videoInputs;
}

QList<QDomElement> JingleMediaManager::payloads()
{
	QList<QDomElement> ret;
	return ret;
}

void JingleMediaManager::startAudioStreaming()
{
/*	if (timer == 0)
	{
		timer = new QTimer();
		timer->setInterval(2000);
		connect(timer, SIGNAL(timeout()), this, SIGNAL(audioReadyRead()));
	}
	if (!timer->isActive())
		timer->start();
*/	
}

QByteArray JingleMediaManager::data()
{
	return QByteArray("Data for 2000 ms, you should not try to play this !!");
}

JingleMediaSession *JingleMediaManager::createNewSession(const QDomElement& payload, const Solid::DeviceInterface *inputDevice, const Solid::DeviceInterface *outputDevice)
{
	if ((!payload.isNull()) || (payload.tagName() != "payload-type"))
		return NULL;
	
	JingleMediaSession *mediaSession = new JingleMediaSession();
	
	mediaSession->setPayloadType(payload);
	mediaSession->setInputDevice(inputDevice);
	mediaSession->setOutputDevice(outputDevice);

	connect(mediaSession, SIGNAL(incomingData()), this, SLOT(slotIncomingData()));
	connect(mediaSession, SIGNAL(terminated()), this, SLOT(slotSessionTerminated()));
	
	m_sessions << mediaSession;

	return mediaSession;
}

void JingleMediaManager::slotSessionTerminated()
{
	JingleMediaSession *session = (JingleMediaSession*) sender();

	m_sessions.removeOne(session);
	delete session; //This will delete signals to and from this session.
}
