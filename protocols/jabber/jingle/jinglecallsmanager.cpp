#include "jinglecallsmanager.h"
#include "contentdialog.h"
#include "jinglesessionmanager.h"

#include "jabberaccount.h"
#include "jabberresource.h"
#include "jabberresourcepool.h"

//using namespace XMPP;

class JingleCallsManager::Private
{
public:
	JabberAccount *jabberAccount;
	XMPP::Client *client;
	QList<JingleSession*> sessions;
	XMPP::JingleContent *rawAudio;
	XMPP::JingleContent *iceAudio;
	XMPP::JingleContent *rawVideo;
	XMPP::JingleContent *iceVideo;
};

JingleCallsManager::JingleCallsManager(JabberAccount* acc)
: d(new Private)
{
	qDebug() << " ********** JingleCallsManager::JingleCallsManager created. ************* ";

	d->jabberAccount = acc;
	d->client = d->jabberAccount->client()->client();
	init();

	connect((const QObject*) d->client->jingleSessionManager(), SIGNAL(newJingleSession(XMPP::JingleSession*)), this, SLOT(slotNewSession(XMPP::JingleSession*)));
}

JingleCallsManager::~JingleCallsManager()
{

}

void JingleCallsManager::init()
{
	QStringList transports;
	transports << "urn:xmpp:tmp:jingle:transports:ice-udp";
	transports << "urn:xmpp:tmp:jingle:transports:raw-udp";
	d->client->jingleSessionManager()->setSupportedTransports(transports);

	QStringList profiles;
	profiles << "RTP/AVP";
	d->client->jingleSessionManager()->setSupportedProfiles(profiles);
	
	// The Media Manager should be able to give xml tags for the supported payloads.
	// For example : d->client->jingleSessionManager()->setSupportedPayloads(m_mediaManager->payloads());
	QList<QDomElement> audioPayloads;
	QList<QDomElement> videoPayloads;
	QDomDocument doc("");
	QDomElement payload = doc.createElement("payload-type");
	payload.setAttribute("id", "97");
	payload.setAttribute("name", "speex");
	payload.setAttribute("clockrate", "8000");
	audioPayloads << payload;
	d->client->jingleSessionManager()->setSupportedAudioPayloads(audioPayloads);
	// We must create possible contents :
	// NOTE:Iris wil manage the other attributes and candidates
	QDomElement iceTransport = doc.createElement("transport");
	iceTransport.setAttribute("xmlns", "urn:xmpp:tmp:jingle:transports:ice-udp");
	QDomElement rawTransport = doc.createElement("transport");
	rawTransport.setAttribute("xmlns", "urn:xmpp:tmp:jingle:transports:raw-udp");
	// rawAudio
	d->rawAudio = new JingleContent();
	d->rawAudio->setName("audio-content");
	d->rawAudio->addPayloadTypes(audioPayloads);
	d->rawAudio->setTransport(rawTransport);
	d->rawAudio->setDescriptionNS("urn:xmpp:tmp:jingle:apps:audio-rtp");
	d->rawAudio->setProfile("RTP/AVP");
	d->rawAudio->setCreator(d->jabberAccount->client()->jid().full());
	
	// iceAudio
	d->iceAudio = new JingleContent();
	d->iceAudio->setName("audio-content");
	d->iceAudio->addPayloadTypes(audioPayloads);
	d->iceAudio->setTransport(iceTransport);
	d->iceAudio->setDescriptionNS("urn:xmpp:tmp:jingle:apps:audio-rtp");
	d->iceAudio->setProfile("RTP/AVP");
	d->iceAudio->setCreator(d->jabberAccount->client()->jid().full());
	
	// rawVideo
	d->rawVideo = new JingleContent();
	d->rawVideo->setName("video-content");
	d->rawVideo->addPayloadTypes(videoPayloads);
	d->rawVideo->setTransport(rawTransport);
	d->rawVideo->setDescriptionNS("urn:xmpp:tmp:jingle:apps:video-rtp");
	d->rawVideo->setProfile("RTP/AVP");
	d->rawVideo->setCreator(d->jabberAccount->client()->jid().full());
	
	// iceVideo
	d->iceVideo = new JingleContent();
	d->iceVideo->setName("video-content");
	d->iceVideo->addPayloadTypes(videoPayloads);
	d->iceVideo->setTransport(iceTransport);
	d->iceVideo->setDescriptionNS("urn:xmpp:tmp:jingle:apps:video-rtp");
	d->iceVideo->setProfile("RTP/AVP");
	d->iceVideo->setCreator(d->jabberAccount->client()->jid().full());
}

bool JingleCallsManager::startNewSession(const XMPP::Jid& toJid)
{
	//TODO:There should be a way to start a video-only or audio-only session.
	qDebug() << "Starting Jingle session for : " << toJid.full();
	
	//Here, we parse the features to have a list of which transports can be used with the responder.
	bool iceUdp = false, rawUdp = false;
	JabberResource *bestResource = d->jabberAccount->resourcePool()->bestJabberResource( toJid );
	QStringList fList = bestResource->features().list();
	for (int i = 0; i < fList.count(); i++)
	{
		if (fList[i] == "urn:xmpp:tmp:jingle:transports:ice-udp")
			iceUdp = true;
		if (fList[i] == "urn:xmpp:tmp:jingle:transports:raw-udp")
			rawUdp = true;
	}
	QList<JingleContent*> contents;
	if (iceUdp)
	{
		contents << d->iceVideo << d->iceAudio;
		qDebug() << "ICE-UDP supported !!!!!!!!!!!!!!!!!!";
	}
	else if (rawUdp)
	{
		contents << d->rawVideo << d->rawAudio;
		qDebug() << "ICE-UDP not supported, back-fall to RAW-UDP !!!!!!!!!!!!!";
	}
	else
	{
		qDebug() << "No protocol supported, terminating.....";
		return false;
	}
	
	d->client->jingleSessionManager()->startNewSession(toJid, contents);
	return true;
}

void JingleCallsManager::slotNewSession(XMPP::JingleSession *sess)
{
	qDebug() << "New session incoming, showing the dialog.";
	
	d->sessions << sess;
	
	ContentDialog *cd = new ContentDialog();
	//cd->setContents(sess->contents());
	cd->setSession(sess);
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

#include "jinglecallsmanager.moc"
