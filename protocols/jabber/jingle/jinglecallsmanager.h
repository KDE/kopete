/*
 * This Class manages all incoming and outgoing Jingle *audio* calls
 * (We'll see later for video management).
 * This is an instance of this class that Kopete will keep to start
 * new Jingle sessions by calling for example manager->newSession(jid).
 */

#ifndef JINGLE_CALLS_MANAGER
#define JINGLE_CALLS_MANAGER

#include <QObject>

#include "im.h"

class JabberAccount;
namespace XMPP
{
	class JingleSession;
}

class JingleCallsManager : public QObject
{
	Q_OBJECT
public:
	JingleCallsManager(JabberAccount*);
	~JingleCallsManager();

	enum Reason {
		Declined = 0,
		Other,
		Unknown
	};
	void startNewSession(const XMPP::Jid&);

public slots:
	void slotNewSession(XMPP::JingleSession*);
	void slotUserAccepted();
	void slotUserRejected();

signals:
	void newSessionCreated();
	void sessionTerminated(Reason);

private:
	class Private;
	Private *d;
	void init();

};

#endif
