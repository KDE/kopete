 /*
  * jinglecallsmanager.cpp - A manager which manages all Jingle sessions.
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
#include <QMessageBox>

#include <ortp/ortp.h>

#include "jinglecallsmanager.h"
#include "jinglecallsgui.h"
#include "jinglecontentdialog.h"
#include "jinglesessionmanager.h"
#include "jinglemediamanager.h"

#include "jabberaccount.h"
#include "jabberresource.h"
#include "jabberresourcepool.h"
#include "jabberjinglesession.h"

#include <KDebug>

//using namespace XMPP;

class JingleCallsManager::Private
{
public:
	JabberAccount *jabberAccount;
	JingleCallsGui *gui;
	QList<JabberJingleSession*> sessions;
	XMPP::Client *client;
	JingleMediaManager *mediaManager;
	QList<QDomElement> audioPayloads;
	QList<QDomElement> videoPayloads;
	JingleContentDialog *contentDialog;
};

JingleCallsManager::JingleCallsManager(JabberAccount* acc)
: d(new Private)
{
	d->jabberAccount = acc;
	d->client = d->jabberAccount->client()->client();
	
	init();
	qDebug() << " ********** JingleCallsManager::JingleCallsManager created. ************* ";
}

JingleCallsManager::~JingleCallsManager()
{
	ortp_exit();

	delete d->mediaManager;

	//TODO:delete the other fields in Private...
}

void JingleCallsManager::init()
{
	//Initialize oRTP library.
	ortp_init();
	//ortp_scheduler_init(); // Check the utility of the scheduler.
	
	d->gui = 0L;
	QStringList transports;
	transports << "urn:xmpp:tmp:jingle:transports:ice-udp";
	transports << "urn:xmpp:tmp:jingle:transports:raw-udp";
	d->client->jingleSessionManager()->setSupportedTransports(transports);

	QStringList profiles;
	profiles << "RTP/AVP";
	d->client->jingleSessionManager()->setSupportedProfiles(profiles);
	
	// The Media Manager should be able to give xml tags for the supported payloads.
	QDomDocument doc("");
	
	// Audio payloads
	QDomElement aPayload = doc.createElement("payload-type");
	aPayload.setAttribute("id", "97");
	aPayload.setAttribute("name", "speex");
	aPayload.setAttribute("clockrate", "8000");
	d->audioPayloads << aPayload;
	d->client->jingleSessionManager()->setSupportedAudioPayloads(d->audioPayloads);
	
	//Video payloads
	QDomElement vPayload = doc.createElement("payload-type");
	vPayload.setAttribute("id", "96");
	vPayload.setAttribute("name", "theora");
	vPayload.setAttribute("clockrate", "90000");
	QStringList pNames;
	pNames  << "height" << "width" << "delivery-method" << "configuration" << "sampling";
	QStringList pValues;
	pValues << "288"    << "352"   << "inline"	    << "?" 	       << "YCbCr-4:2:0";// Thoses data are from the file http://www.polycrystal.org/lego/movies/Swim.ogg
	for (int i = 0; i < pNames.count(); i++)
	{
		QDomElement parameter = doc.createElement("parameter");
		parameter.setAttribute("name", pNames[i]);
		parameter.setAttribute("value", pValues[i]);
		vPayload.appendChild(parameter);
	}
	d->videoPayloads << vPayload;

	d->mediaManager = new JingleMediaManager();
	
	d->client->jingleSessionManager()->setSupportedVideoPayloads(d->videoPayloads);
	connect((const QObject*) d->client->jingleSessionManager(), SIGNAL(newJingleSession(XMPP::JingleSession*)),
		this, SLOT(slotNewSession(XMPP::JingleSession*)));
	connect((const QObject*) d->client->jingleSessionManager(), SIGNAL(sessionTerminate(XMPP::JingleSession*)),
		this, SLOT(slotSessionTerminate(XMPP::JingleSession*)));
}

bool JingleCallsManager::startNewSession(const XMPP::Jid& toJid)
{
	//TODO:There should be a way to start a video-only or audio-only session.
	qDebug() << "Starting Jingle session for : " << toJid.full();
	
	//Here, we parse the features to have a list of which transports can be used with the responder.
	bool iceUdp = false, rawUdp = false;
	JabberResource *bestResource = d->jabberAccount->resourcePool()->bestJabberResource( toJid );
	if (!bestResource)
	{
		// Would be better to show this message in the chat dialog.
		//QMessageBox::warning((QWidget*)this, tr("Jingle Calls Manager"), tr("The contact is not online, unable to start a Jingle session"), QMessageBox::Ok);
		qDebug() << "The contact is not online, unable to start a Jingle session";
		return false;
	}
	QStringList fList = bestResource->features().list();
	for (int i = 0; i < fList.count(); i++)
	{
		if (fList[i] == "urn:xmpp:tmp:jingle:transports:ice-udp")
			iceUdp = true;
		if (fList[i] == "urn:xmpp:tmp:jingle:transports:raw-udp")
			rawUdp = true;
	}
	QList<JingleContent*> contents;
	QDomDocument doc("");
	if (iceUdp)
	{
		// Create ice-udp contents.
		QDomElement iceAudioTransport = doc.createElement("transport");
		iceAudioTransport.setAttribute("xmlns", "urn:xmpp:tmp:jingle:transports:ice-udp");
		JingleContent *iceAudio = new JingleContent();
		iceAudio->setName("audio-content");
		iceAudio->addPayloadTypes(d->audioPayloads);
		iceAudio->setTransport(iceAudioTransport);
		iceAudio->setDescriptionNS("urn:xmpp:tmp:jingle:apps:audio-rtp");
		iceAudio->setProfile("RTP/AVP");
		iceAudio->setCreator(d->jabberAccount->client()->jid().full());

		QDomElement iceVideoTransport = doc.createElement("transport");
		iceVideoTransport.setAttribute("xmlns", "urn:xmpp:tmp:jingle:transports:ice-udp");
		JingleContent *iceVideo = new JingleContent();
		iceVideo->setName("video-content");
		iceVideo->addPayloadTypes(d->videoPayloads);
		iceVideo->setTransport(iceVideoTransport);
		iceVideo->setDescriptionNS("urn:xmpp:tmp:jingle:apps:video-rtp");
		iceVideo->setProfile("RTP/AVP");
		iceVideo->setCreator(d->jabberAccount->client()->jid().full());
		
		contents << iceAudio << iceVideo;
		qDebug() << "ICE-UDP supported !!!!!!!!!!!!!!!!!!";
	}
	else if (rawUdp)
	{
		// Create raw-udp contents.
		QDomElement rawAudioTransport = doc.createElement("transport");
		rawAudioTransport.setAttribute("xmlns", "urn:xmpp:tmp:jingle:transports:raw-udp");
		JingleContent *rawAudio = new JingleContent();
		rawAudio->setName("audio-content");
		rawAudio->addPayloadTypes(d->audioPayloads);
		rawAudio->setTransport(rawAudioTransport);
		rawAudio->setDescriptionNS("urn:xmpp:tmp:jingle:apps:audio-rtp");
		rawAudio->setProfile("RTP/AVP");
		rawAudio->setCreator(d->jabberAccount->client()->jid().full());

		QDomElement rawVideoTransport = doc.createElement("transport");
		rawVideoTransport.setAttribute("xmlns", "urn:xmpp:tmp:jingle:transports:raw-udp");
		JingleContent *rawVideo = new JingleContent();
		rawVideo->setName("video-content");
		rawVideo->addPayloadTypes(d->videoPayloads);
		rawVideo->setTransport(rawVideoTransport);
		rawVideo->setDescriptionNS("urn:xmpp:tmp:jingle:apps:video-rtp");
		rawVideo->setProfile("RTP/AVP");
		rawVideo->setCreator(d->jabberAccount->client()->jid().full());
		
		contents << rawAudio << rawVideo;
		qDebug() << "ICE-UDP not supported, falling back to RAW-UDP !";
	}
	else
	{
		qDebug() << "No protocol supported, terminating.....";
		// Would be better to show this message in the chat dialog.
		//QMessageBox::warning((QWidget*) this, tr("Jingle Calls Manager"), tr("The contact does not support any transport method that kopete does."), QMessageBox::Ok);
		qDebug() << "The contact does not support any transport method that kopete does.";
		return false;
	}
	
	JingleSession* newSession = d->client->jingleSessionManager()->startNewSession(toJid, contents);
	JabberJingleSession *jabberSess = new JabberJingleSession(this);
	jabberSess->setMediaManager(d->mediaManager); //Could be done directly in the constructor
	jabberSess->setJingleSession(newSession); //Could be done directly in the constructor
	d->sessions << jabberSess;
	if(d->gui)
		d->gui->addSession(jabberSess);
	return true;
}

void JingleCallsManager::slotNewSession(XMPP::JingleSession *newSession)
{
	qDebug() << "New session incoming, showing the dialog.";
	
	JabberJingleSession *jabberSess = new JabberJingleSession(this);
	jabberSess->setMediaManager(d->mediaManager); //Could be done directly in the constructor
	jabberSess->setJingleSession(newSession); //Could be done directly in the constructor
	d->sessions << jabberSess;
	if (d->gui)
		d->gui->addSession(jabberSess);
	
	d->contentDialog = new JingleContentDialog();
	d->contentDialog->setSession(newSession);
	connect(d->contentDialog, SIGNAL(accepted()), this, SLOT(slotUserAccepted()));
	connect(d->contentDialog, SIGNAL(rejected()), this, SLOT(slotUserRejected()));
	d->contentDialog->show();
}

void JingleCallsManager::slotUserAccepted()
{
	qDebug() << "The user accepted the session, checking accepted contents :";
	JingleContentDialog *contentDialog = (JingleContentDialog*) sender();
	if (contentDialog == NULL)
	{
		qDebug() << "Fatal Error : sender is NULL !!!!";
		return;
	}
	if (contentDialog->unChecked().count() == 0)
	{
		qDebug() << "Accept all contents !";
		contentDialog->session()->acceptSession();
	}
	else if (contentDialog->checked().count() == 0)
	{
		qDebug() << "Terminate the session, no contents accepted.";
		contentDialog->session()->terminate(JingleReason(JingleReason::Decline));
	}
	else
	{
		qDebug() << "Accept only some contents, removing some unaccepted.";
		contentDialog->session()->removeContent(contentDialog->unChecked());
	}
}

void JingleCallsManager::slotUserRejected()
{
	JingleContentDialog *contentDialog = (JingleContentDialog*) sender();
	if (contentDialog == NULL)
	{
		qDebug() << "Fatal Error : sender is NULL !!!!";
		return;
	}
	contentDialog->session()->terminate(JingleReason(JingleReason::Decline));
}

void JingleCallsManager::showCallsGui()
{
	if (d->gui == 0L)
	{
		d->gui = new JingleCallsGui(this);
		d->gui->setSessions(d->sessions);
	}
	d->gui->show();
}

void JingleCallsManager::hideCallsGui()
{
	if (d->gui == 0L)
		return;
	d->gui->hide();
}

QList<JabberJingleSession*> JingleCallsManager::jabberSessions()
{
	return d->sessions;
}

void JingleCallsManager::slotSessionTerminate(XMPP::JingleSession* sess)
{
	for (int i = 0; i < d->sessions.count(); i++)
	{
		if (d->sessions[i]->jingleSession() == sess)
		{
			d->gui->removeSession(d->sessions[i]);
			delete d->sessions[i];
			d->sessions.removeAt(i);
		}
	}

}

#include "jinglecallsmanager.moc"
