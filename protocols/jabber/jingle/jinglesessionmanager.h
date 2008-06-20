/*
 * Manages a Jingle session using iris library jingle implementation
 */
#ifndef JINGLE_SESSION_MANAGER
#define JINGLE_SESSION_MANAGER

#include <QObject>

#include "xmpp_client.h"
#include "xmpp_jid.h"

using namespace XMPP;

class JingleSessionManager : public QObject
{
	Q_OBJECT
public:
	JingleSessionManager(Client*, Jid*);
	~JingleSessionManager();
	void start();

private:
	Client *m_client;
	Jid *m_jid;
};

#endif
