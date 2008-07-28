#ifndef JABBER_JINGLE_CONTENT
#define JABBER_JINGLE_CONTENT

#include <QObject>
#include <QString>
#include <QDomElement>

namespace XMPP
{
	class JingleContent;
	class JingleSession;
}
class JabberJingleSession;
class JingleMediaManager;
class JingleRtpSession;

class JabberJingleContent : public QObject
{
	Q_OBJECT
public:
	JabberJingleContent(JabberJingleSession* parent = 0, XMPP::JingleContent* c = 0);
	~JabberJingleContent();

	void setContent(XMPP::JingleContent*);
	void startWritingRtpData();
	QString contentName();
	QDomElement bestPayload(const QList<QDomElement>&, const QList<QDomElement>&);
	bool samePayload(const QDomElement&, const QDomElement&);
	QString elementToSdp(const QDomElement&);

public slots:
	void slotSendRtpData();
	void slotPrepareRtpInSession();
	void slotPrepareRtpOutSession();

private:
	XMPP::JingleContent *m_content;
	XMPP::JingleSession *m_jingleSession;
	JingleMediaManager *m_mediaManager;
	JingleRtpSession *m_rtpInSession;
	JingleRtpSession *m_rtpOutSession;
	JabberJingleSession *m_jabberSession;
};

#endif
