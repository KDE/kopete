#ifndef JABBER_JINGLE_SESSION_H
#define JABBER_JINGLE_SESSION_H
#include <QObject>
#include <QDomElement>

namespace XMPP
{
	class JingleSession;
}

class JabberJingleSession : public QObject
{
public:
	JabberJingleSession();
	~JabberJingleSession();
	
	void setJingleSession(XMPP::JingleSession*);
	XMPP::JingleSession *jingleSession() {return m_jingleSession;}
	
	//void setMediaManager();
	//MediaManager *mediaManager() {return m_mediaManager}

private slots:
	void writeRtpData(const QDomElement&);

private:
	XMPP::JingleSession *m_jingleSession;
};

#endif
