/*
 * This class manages all incoming and outgoing Jingle calls
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
class JabberJingleSession;
class MediaManager;
class JingleCallsManager : public QObject
{
	Q_OBJECT
public:
	JingleCallsManager(JabberAccount*);
	~JingleCallsManager();

	// Would feel better in Iris
	enum Reason {
		Declined = 0,
		Other,
		Unknown
	};
	// Returns true if the session is possible to start, false otherwise.
	// False is returned only if the responder has no compatible transports.
	// That may not be necessary, it should simply show a message or write it in
	// the Jingle calls GUI if it is not possible to start a session.
	bool startNewSession(const XMPP::Jid&);
	void showCallsGui();
	void hideCallsGui();
	QList<JabberJingleSession*> jabberSessions();
	MediaManager* mediaManager();

public slots:
	void slotNewSession(XMPP::JingleSession*);
	void slotSessionTerminate(XMPP::JingleSession*);
	void slotSessionTerminated();
	void slotUserAccepted();
	void slotUserRejected();

signals:
	//FIXME:Those signals aren't used and I don't know what they are doing here.
	void newSessionCreated();
	void sessionTerminated(Reason);

private:
	class Private;
	Private *d;
	void init();

};

#endif
