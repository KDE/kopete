#include "jinglecallsmanager.h"
#include "contentdialog.h"
#include "jinglesessionmanager.h"

#include "jabberaccount.h"

//using namespace XMPP;

class JingleCallsManager::Private
{
public:
	JabberAccount *jabberAccount;
	XMPP::Client *client;
	QList<JingleSession*> sessions;
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
	QDomDocument doc("");
	QDomElement payload = doc.createElement("payload-type");
	payload.setAttribute("id", "97");
	payload.setAttribute("name", "speex");
	payload.setAttribute("clockrate", "8000");
	audioPayloads << payload;
	d->client->jingleSessionManager()->setSupportedAudioPayloads(audioPayloads);
}

void JingleCallsManager::startNewSession(const XMPP::Jid& toJid)
{
	qDebug() << "Starting Jingle session for : " << toJid.full();
	d->client->jingleSessionManager()->startNewSession(toJid);
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
