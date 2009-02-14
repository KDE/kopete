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
#include <QTcpSocket>

#include <ortp/ortp.h>

#include "jinglerawcontent.h"
#include "jingleicecontent.h"

#include "jinglecallsmanager.h"
#include "jinglecallsgui.h"
#include "jinglecontentdialog.h"
#include "jinglesessionmanager.h"
#include "mediamanager.h"
#include "mediasession.h"

#include "jabberaccount.h"
#include "jabberresource.h"
#include "jabberresourcepool.h"
#include "jabberjinglesession.h"

#include <KDebug>
#include <KUrl>
#include <kio/netaccess.h>

//using namespace XMPP;

class JingleCallsManager::Private
{
public:
	JabberAccount *jabberAccount;
	JingleCallsGui *gui;
	QList<JabberJingleSession*> sessions;
	XMPP::Client *client;
	MediaManager *mediaManager;
	QList<QDomElement> audioPayloads;
	QList<QDomElement> videoPayloads;
	JingleContentDialog *contentDialog;
	QTcpSocket *stunSocket;
};

JingleCallsManager::JingleCallsManager(JabberAccount* acc)
: d(new Private)
{
	d->jabberAccount = acc;
	d->client = d->jabberAccount->client()->client();
	
	init();
	kDebug() << " ********** JingleCallsManager::JingleCallsManager created. ************* ";
}

JingleCallsManager::~JingleCallsManager()
{
	ortp_exit();

	delete d->gui;
	
	for (int i = 0; i < d->sessions.count(); i++)
	{
		delete d->sessions[i];
	}

	delete d->mediaManager;

        delete d;
}

void JingleCallsManager::init()
{
	d->client->jingleSessionManager()->setFirstPort(d->jabberAccount->configGroup()->readEntry("JingleFirstPort", QString("9000")).toInt());

	//Initialize oRTP library.
	ortp_init();
	ortp_scheduler_init();
	ortp_set_log_file(0);
	ortp_log_level_enabled(ORTP_ERROR | ORTP_FATAL);
	
	d->gui = 0L;
	QStringList transports;
	transports << NS_JINGLE_TRANSPORTS_ICE;
	//transports << NS_JINGLE_TRANSPORTS_RAW;
	d->client->jingleSessionManager()->setSupportedTransports(transports);

	// TODO:The Media Manager should be able to give xml elements for the supported payloads.
	QDomDocument doc("");
	
	// Audio payloads
	//Speex
	QDomElement sPayload = doc.createElement("payload-type");
	sPayload.setAttribute("id", "96");
	sPayload.setAttribute("name", "speex");
	sPayload.setAttribute("clockrate", "8000");
	d->audioPayloads << sPayload;
	
	//A-Law
	//Do I still have implementation for that ?
	//To remove anyway
	QDomElement aPayload = doc.createElement("payload-type");
	aPayload.setAttribute("id", "8");
	aPayload.setAttribute("name", "PCMA");
	//aPayload.setAttribute("clockrate", "8000");
	d->audioPayloads << aPayload;

	d->client->jingleSessionManager()->setSupportedAudioPayloads(d->audioPayloads);
	
	//Video payloads
	/*QDomElement vPayload = doc.createElement("payload-type");
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
	d->videoPayloads << vPayload;*/

	QString inputDev = d->jabberAccount->configGroup()->readEntry("JingleInputDevice", QString());
	QString outputDev = d->jabberAccount->configGroup()->readEntry("JingleOutputDevice", QString());
	d->mediaManager = new MediaManager(inputDev, outputDev);
	
	d->client->jingleSessionManager()->setSupportedVideoPayloads(d->videoPayloads);
	
	// psa provides a TURN/STUN server : eu.turn.xmpp.org
	d->stunSocket = new QTcpSocket();
	connect(d->stunSocket, SIGNAL(connected()), this, SLOT(stunSocketConnected()));
	d->stunSocket->connectToHost("eu.turn.xmpp.org", 3478);

	connect((const QObject*) d->client->jingleSessionManager(), SIGNAL(newJingleSession(XMPP::JingleSession*)),
		this, SLOT(slotNewSession(XMPP::JingleSession*)));
	connect((const QObject*) d->client->jingleSessionManager(), SIGNAL(sessionTerminate(XMPP::JingleSession*)),
		this, SLOT(slotSessionTerminate(XMPP::JingleSession*)));
}

void JingleCallsManager::stunSocketConnected()
{
	d->client->jingleSessionManager()->setStunServiceAddress(d->stunSocket->peerAddress(), 3478);
	
	d->stunSocket->deleteLater();
}

bool JingleCallsManager::startNewSession(const XMPP::Jid& toJid)
{
	kDebug() << "Starting Jingle session for : " << toJid.full();
	
	// Create the session.
	JingleSession* newSession = d->client->jingleSessionManager()->startNewSession(toJid);
	
	//Here, we parse the features to have a list of which transports can be used with the responder.
	bool iceUdp = false, rawUdp = false;
	JabberResource *bestResource = d->jabberAccount->resourcePool()->bestJabberResource( toJid );
	if (!bestResource)
	{
		// Would be better to show this message in the chat dialog.
		//QMessageBox::warning((QWidget*)this, tr("Jingle Calls Manager"), tr("The contact is not online, unable to start a Jingle session"), QMessageBox::Ok);
		kDebug() << "The contact is not online, unable to start a Jingle session";
		return false;
	}
	QStringList fList = bestResource->features().list();
	for (int i = 0; i < fList.count(); i++)
	{
		if (fList[i] == NS_JINGLE_TRANSPORTS_ICE)
			iceUdp = true;
		if (fList[i] == NS_JINGLE_TRANSPORTS_RAW)
			rawUdp = true;
	}
	QList<JingleContent*> contents;

	kDebug() << iceUdp << rawUdp;
	

	QDomDocument doc("");
	if (iceUdp)
	{
		kDebug() << "ICE-UDP supported !!!!!!!!!!!!!!!!!!";

		// Create ice-udp contents.
		JingleContent *iceAudio = new JingleIceContent(JingleContent::Initiator, newSession);
		iceAudio->setName("audio-content");
		iceAudio->addLocalPayloads(d->audioPayloads);
		iceAudio->setDescriptionNS(NS_JINGLE_APPS_RTP);
		iceAudio->setType(JingleContent::Audio);
		iceAudio->setCreator("initiator");

		/*JingleContent *iceVideo = new JingleIceContent(JingleContent::Initiator, newSession);
		iceVideo->setName("video-content");
		iceVideo->addLocalPayloads(d->videoPayloads);
		iceVideo->setDescriptionNS(NS_JINGLE_APPS_RTP);
		iceVideo->setType(JingleContent::Video);
		iceVideo->setCreator("initiator");*/
		
		//contents << iceAudio << iceVideo;
		contents << iceAudio;
	}
	else if (rawUdp)
	{
		kDebug() << "ICE-UDP not supported, falling back to RAW-UDP !";

		// Create raw-udp contents.
		JingleContent *rawAudio = new JingleRawContent(JingleContent::Initiator, newSession);
		rawAudio->setName("audio-content");
		rawAudio->addLocalPayloads(d->audioPayloads);
		rawAudio->setDescriptionNS(NS_JINGLE_APPS_RTP);
		rawAudio->setType(JingleContent::Audio);
		rawAudio->setCreator("initiator");

		/*JingleContent *rawVideo = new JingleRawContent(JingleContent::Initiator, newSession);
		rawVideo->setName("video-content");
		rawVideo->addLocalPayloads(d->videoPayloads);
		rawVideo->setDescriptionNS(NS_JINGLE_APPS_RTP);
		rawVideo->setType(JingleContent::Video);
		rawVideo->setCreator("initiator");*/
		
		//contents << rawAudio << rawVideo;
		contents << rawAudio;
	}
	else
	{
		kDebug() << "No protocol supported, terminating.....";
		// Would be better to show this message in the chat dialog.
		//QMessageBox::warning((QWidget*) this, tr("Jingle Calls Manager"), tr("The contact does not support any transport method that kopete does."), QMessageBox::Ok);
		kDebug() << "The contact does not support any transport method that kopete does.";
		return false;
	}
	
	newSession->addContents(contents);
	newSession->start();

	JabberJingleSession *jabberSess = new JabberJingleSession(this);
	connect(jabberSess, SIGNAL(terminated()), this, SLOT(slotSessionTerminated()));
	connect(jabberSess, SIGNAL(stateChanged()), this, SLOT(slotStateChanged()));
	jabberSess->setJingleSession(newSession); //Could be done directly in the constructor
	d->sessions << jabberSess;
	
	if(d->gui)
		d->gui->addSession(jabberSess);
	
	return true;
}

void JingleCallsManager::slotStateChanged()
{
	JabberJingleSession *sess = (JabberJingleSession*) sender();

	d->gui->changeState(sess);
}

void JingleCallsManager::slotNewSession(XMPP::JingleSession *newSession)
{
	showCallsGui();
	kDebug() << "New session incoming, showing the dialog.";
	
	JabberJingleSession *jabberSess = new JabberJingleSession(this);
	jabberSess->setJingleSession(newSession); //Could be done directly in the constructor
	connect(jabberSess, SIGNAL(terminated()), this, SLOT(slotSessionTerminated()));
	connect(jabberSess, SIGNAL(stateChanged()), this, SLOT(slotStateChanged()));

	d->sessions << jabberSess;
	if (d->gui)
		d->gui->addSession(jabberSess);
	
	d->contentDialog = new JingleContentDialog(d->gui);
	d->contentDialog->setSession(newSession);
	connect(d->contentDialog, SIGNAL(accepted()), this, SLOT(slotUserAccepted()));
	connect(d->contentDialog, SIGNAL(rejected()), this, SLOT(slotUserRejected()));
	d->contentDialog->show();
}

void JingleCallsManager::slotSessionTerminated()
{
	JabberJingleSession *sess = (JabberJingleSession*) sender();
	d->gui->removeSession(sess);
	
	for (int i = 0; i < d->sessions.count(); i++)
	{
		if (sess == d->sessions[i])
			d->sessions.removeAt(i);
	}

	delete sess;
}

void JingleCallsManager::slotUserAccepted()
{
	kDebug() << "The user accepted the session, checking accepted contents :";
	JingleContentDialog *contentDialog = (JingleContentDialog*) sender();
	if (contentDialog == NULL)
	{
		kDebug() << "Fatal Error : sender is NULL !!!!";
		return;
	}
	if (contentDialog->unChecked().count() == 0)
	{
		kDebug() << "Accept all contents !";
		//We must not actually accept the session, we must accept the session if everything has been established
		//if not, we must wait for everything to be established.
		//For Raw-UDP, as soon as a content is connected, stream must be terminated and we must wait for the user
		//to accept the session before we actually send multimedia stream and play it.
		contentDialog->session()->acceptSession();
	}
	else if (contentDialog->checked().count() == 0)
	{
		kDebug() << "Terminate the session, no contents accepted.";
		contentDialog->session()->sessionTerminate(JingleReason(JingleReason::Decline));
	}
	else
	{
		kDebug() << "Accept only some contents, removing some unaccepted.";
		contentDialog->session()->removeContent(contentDialog->unChecked());
	}
	kDebug() << "end";
	contentDialog->close();
	contentDialog->deleteLater();
}

void JingleCallsManager::slotUserRejected()
{
	JingleContentDialog *contentDialog = (JingleContentDialog*) sender();
	if (contentDialog == NULL)
	{
		kDebug() << "Fatal Error : sender is NULL !!!!";
		return;
	}
	contentDialog->session()->sessionTerminate(JingleReason(JingleReason::Decline));
	kDebug() << "end";
	contentDialog->close();
	contentDialog->deleteLater();
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

MediaManager* JingleCallsManager::mediaManager()
{
	return d->mediaManager;
}

#include "jinglecallsmanager.moc"
