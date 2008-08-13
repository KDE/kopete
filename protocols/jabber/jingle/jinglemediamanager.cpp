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

JingleMediaSession::JingleMediaSession(JingleMediaManager *parent)
 : m_mediaManager(parent)
{
	kDebug() << "created";
	ts = 0;
	tsValue = 0;
}

JingleMediaSession::~JingleMediaSession()
{
	kDebug() << "deleted";
}

void JingleMediaSession::setPayloadType(const QDomElement& payload)
{
	kDebug() << "called";
	//Maybe we will use it, maybe we won't...
	int pID = payload.attribute("id").toInt();
	if (pID >= 0 && pID <= 23) // This is audio payload.
		m_type = Audio;
	else /*if (pID >= 24 && pID <= 34)*/// Let's say other ones are video... // This is video payload.
		m_type = Video;
	
	m_payload = payload;
}

void JingleMediaSession::setInputDevice(Solid::Device& device)
{
	if (m_type == Audio)
	{
		audioInputDevice = device.as<Solid::AudioInterface>();
		kDebug() << "Using" << audioInputDevice->name() << "audio card for INPUT";
	}
}

void JingleMediaSession::setOutputDevice(Solid::Device& device)
{
	Q_UNUSED(device)
}

void JingleMediaSession::setCaptureMediaPlugin(AlsaALaw *p)
{
	kDebug() << "Setting the capturePlugin";
	//Here we should be able to configure the capturePlugin and tell it which souncard it should use.
	capturePlugin = p;

	if (capturePlugin->isReady())
		tsValue = capturePlugin->timeStamp();
	else
		tsValue = 168;
		// This value should be set to the default depending on the "blank" stream that will be sent.
		// In this case, it will be blank ALaw (only zeros) with a timestamp increment of 168.
	connect(capturePlugin, SIGNAL(readyRead()), this, SLOT(slotInReadyRead()));
}

void JingleMediaSession::setPlaybackMediaPlugin(AlsaALaw *p)
{
	kDebug() << "Setting playbackPlugin";
	playbackPlugin = p;
}

void JingleMediaSession::start()
{
	capturePlugin->start();
	playbackPlugin->start();
}

void JingleMediaSession::playData(const QByteArray& data)
{
	playbackPlugin->write(data);
}

void JingleMediaSession::slotInReadyRead()
{
	emit readyRead(ts);
	ts += tsValue;
}

QByteArray JingleMediaSession::data()
{
	return capturePlugin->data();
}

/*********************
 * JingleMediaManager
 *********************/

JingleMediaManager::JingleMediaManager()
{
	kDebug() << "Created";
	findDevices();
	timer = 0;
	alawCapture = 0;
	alawPlayback = 0;
}

JingleMediaManager::~JingleMediaManager()
{
	kDebug() << "Media manager destroyed";
	if (alawCapture)
		delete alawCapture;
	if (alawPlayback)
		delete alawPlayback;
	if (timer)
		delete timer;
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
			m_microphones << deviceList[i];
		}

		if (device->deviceType() == Solid::AudioInterface::AudioOutput)
		{
			kDebug() << "Sound card found. The driver used is " << device->driver();
			m_audioOutputs << deviceList[i];
		}
	}

	deviceList = Solid::Device::listFromType(Solid::DeviceInterface::Video, QString());
	for (int i = 0; i < deviceList.count(); i++)
		m_videoInputs << deviceList[i];
}

void JingleMediaManager::startVideoStreaming()
{

}

QList<Solid::Device> JingleMediaManager::videoDevices()
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
	QDomDocument doc("");
	QDomElement pcma = doc.createElement("payload-type");
	pcma.setAttribute("name", "PCMA");
	pcma.setAttribute("id", "8");
	//pcma.setAttribute("");
	ret << pcma;
	return ret;
}

void JingleMediaManager::startStreaming(const QDomElement& payloadType)
{
	Q_UNUSED(payloadType)
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

JingleMediaSession *JingleMediaManager::createNewSession(const QDomElement& payload, Solid::Device inputDevice, Solid::Device outputDevice)
{
	kDebug() << (payload.isNull() ? "Payload is NULL !! tagname =" : "payload tagname =") << payload.tagName();
	if (payload.isNull() || (payload.tagName() != "payload-type"))
		return NULL;
	
	JingleMediaSession *mediaSession = new JingleMediaSession(this);
	
	mediaSession->setPayloadType(payload);
	if (!inputDevice.isValid())
		; //set default device depending on the payload-type
	else
		mediaSession->setInputDevice(inputDevice);
	if (!outputDevice.isValid())
		; //set default device depending on the payload-type
	else
		mediaSession->setOutputDevice(outputDevice);
	
	if (payload.attribute("id") == "8")
	{
		if ((payload.hasAttribute("name") && payload.attribute("name") == "PCMA") || !payload.hasAttribute("name"))
		{
			//Start a-law streaming. Currently, it is the only one supported, more to come.
			//The problem is that Phonon does not support audio input yet...
			if (alawCapture == 0)
			{
				alawCapture = new AlsaALaw(AlsaALaw::Capture);
			}
			mediaSession->setCaptureMediaPlugin(alawCapture);
			
			if (alawPlayback == 0)
			{
				alawPlayback = new AlsaALaw(AlsaALaw::Playback);
			}
			mediaSession->setPlaybackMediaPlugin(alawPlayback);

			//FIXME:I call it a Media Plugin, maybe those can be loaded as plugins for each formats.
			//	It Depends on what Phonon will provide (will I have to encode audio or will Phonon take care of that ?)
		}
	}

	connect(mediaSession, SIGNAL(incomingData()), this, SLOT(slotIncomingData()));
	connect(mediaSession, SIGNAL(terminated()), this, SLOT(slotSessionTerminated()));
	
	m_sessions << mediaSession;

	qDebug() << "Returning media session" << mediaSession;
	return mediaSession;
}

void JingleMediaManager::slotIncomingData()
{

}

void JingleMediaManager::slotSessionTerminated()
{
	JingleMediaSession *session = (JingleMediaSession*) sender();

	m_sessions.removeOne(session);
	delete session; //This will delete signals to and from this session.
}
