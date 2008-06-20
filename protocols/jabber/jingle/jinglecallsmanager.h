/*
 * This Class manages all incoming and outgoing Jingle *audio* calls
 * (We'll see later for video management).
 * This is an instance of this class that Kopete will keep to start
 * new Jingle sessions by calling for example manager->newSession(jid).
 */

#ifndef JINGLE_CALLS_MANAGER
#define JINGLE_CALLS_MANAGER

#include <QObject>

#include "xmpp_client.h"
#include "xmpp_jid.h"

#include "jinglesessionmanager.h"
#include "jingleaudiomanager.h"

using namespace XMPP;

//friend class Client;

class JingleCallsManager : public QObject
{
	Q_OBJECT
public:
	JingleCallsManager(Client*);
	~JingleCallsManager();

	enum Reason {
		Declined = 0,
		Other,
		Unknown
	};
	void newSession(Jid*);

//public slots:


signals:
	void newSessionCreated();
	void sessionTerminated(Reason);

private:
	Client *m_client;
	QList<JingleSessionManager*> m_sessionList;

};

#endif
