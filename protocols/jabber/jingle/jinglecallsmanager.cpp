#include <QMessageBox>

#include <ortp/ortp.h>

#include "jinglecallsmanager.h"
#include "jinglecallsgui.h"
#include "contentdialog.h"
#include "jinglesessionmanager.h"
#include "jinglemediamanager.h"

#include "jabberaccount.h"
#include "jabberresource.h"
#include "jabberresourcepool.h"
#include "jabberjinglesession.h"

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
	// For example : d->client->jingleSessionManager()->setSupportedPayloads(m_mediaManager->payloads());
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
	
	connect((const QObject*) d->client->jingleSessionManager(), SIGNAL(newJingleSession(XMPP::JingleSession*)), this, SLOT(slotNewSession(XMPP::JingleSession*)));
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
	// TODO:Connect all session signals here...
	JabberJingleSession *jabberSess = new JabberJingleSession(this);
	jabberSess->setJingleSession(newSession); //Could be done directly in the constructor
	jabberSess->setMediaManager(d->mediaManager); //Could be done directly in the constructor
	d->sessions << jabberSess;
	return true;
}

void JingleCallsManager::slotNewSession(XMPP::JingleSession *newSession)
{
	qDebug() << "New session incoming, showing the dialog.";
	
	// TODO:Connect all session signals here...
	JabberJingleSession *jabberSess = new JabberJingleSession(this);
	jabberSess->setJingleSession(newSession); //Could be done directly in the constructor
	jabberSess->setMediaManager(d->mediaManager); //Could be done directly in the constructor
	d->sessions << jabberSess;
	
	ContentDialog *cd = new ContentDialog();
	//cd->setContents(sess->contents());
	cd->setSession(newSession);
	connect(cd, SIGNAL(accepted()), this, SLOT(slotUserAccepted()));
	connect(cd, SIGNAL(rejected()), this, SLOT(slotUserRejected()));
	cd->show();
}

void JingleCallsManager::slotUserAccepted()
{
	qDebug() << "The user accepted the session, checking accepted contents :";
	ContentDialog *cd = (ContentDialog*) sender();
	if (cd == NULL)
	{
		qDebug() << "Fatal Error : sender is NULL !!!!";
		return;
	}
	/*
	 * TODO:
	 *	Check the contents not accepted (cd->child() or sth like that)
	 *	If no contents are removed, cd->session()->contentAccept().
	 *	If contents are removed, cd->session()->contentRemove(contents).
	 *	If all contents are removed, slotUserRejected() (or so)
	 */
	if (cd->unChecked().count() == 0)
	{
		qDebug() << "Accept all contents !";
		cd->session()->acceptSession();
	}
	else if (cd->checked().count() == 0)
	{
		qDebug() << "Terminate the session, no contents accepted.";
		cd->session()->terminate(JingleSession::Decline);
	}
	else
	{
		qDebug() << "Accept only some contents, removing some unaccepted.";
		cd->session()->removeContent(cd->unChecked());
	}
}

void JingleCallsManager::slotUserRejected()
{
	ContentDialog *cd = (ContentDialog*) sender();
	if (cd == NULL)
	{
		qDebug() << "Fatal Error : sender is NULL !!!!";
		return;
	}
	cd->session()->terminate(JingleSession::Decline);
}

void JingleCallsManager::showCallsGui()
{
	if (d->gui == 0L)
		d->gui = new JingleCallsGui(this);
	d->gui->show();
}

void JingleCallsManager::hideCallsGui()
{
	if (d->gui == 0L)
		return;
	d->gui->hide();
}
#include "jinglecallsmanager.moc"
