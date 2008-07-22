#ifndef JABBER_JINGLE_SESSION_H
#define JABBER_JINGLE_SESSION_H
#include <QObject>
#include <QDomElement>

namespace XMPP
{
	class JingleSession;
	class JingleContent;
}
class JingleCallsManager;
class JabberJingleContent;
class JingleMediaManager;
class JingleRtpSession;

class JabberJingleSession : public QObject
{
	Q_OBJECT
public:
	JabberJingleSession(JingleCallsManager*);
	~JabberJingleSession();
	
	void setJingleSession(XMPP::JingleSession*);
	XMPP::JingleSession *jingleSession() {return m_jingleSession;}
	
	void setMediaManager(JingleMediaManager*);
	JabberJingleContent *contentWithName(const QString&);
	XMPP::JingleSession *session() const {return m_jingleSession;}
	JingleMediaManager *mediaManager() const {return m_mediaManager;}

public slots:
	void writeRtpData(XMPP::JingleContent*);

private:
	XMPP::JingleSession *m_jingleSession;
	JingleCallsManager *m_callsManager;
	JingleMediaManager *m_mediaManager;
	QList<JabberJingleContent*> jabberJingleContents;
	JingleRtpSession *m_rtpSession;
};

#endif
