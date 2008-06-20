#include "jinglecallsmanager.h"

JingleCallsManager::JingleCallsManager(Client* client)
: m_client(client)
{

}

JingleCallsManager::~JingleCallsManager()
{

}

void JingleCallsManager::newSession(Jid *toJid)
{
	JingleSessionManager *session = new JingleSessionManager(m_client, toJid);

	/*
	 * QString argument is the session id.
	 */
	QObject::connect(session, SIGNAL(declined(const QString&)), this, SLOT(slotDeclined(const QString&)));
	QObject::connect(session, SIGNAL(terminated(const QString&)), this, SLOT(slotTerminated(const QString&)));
	/*...*/
	
	session->start();
	m_sessionList << session;
}
