#include "jabberjinglesession.h"
#include "jabberjinglecontent.h"
#include "rtpsession.h"
#include "jinglemediamanager.h"

#include "jinglesession.h"
#include "jinglecontent.h"

//using namespace XMPP;

JabberJingleSession::JabberJingleSession(JingleCallsManager* parent)
: m_callsManager(parent)
{
	qDebug() << "Created a new JabberJingleSession";
	m_rtpSession = 0;
}

JabberJingleSession::~JabberJingleSession()
{

}

void JabberJingleSession::setJingleSession(XMPP::JingleSession* sess)
{
	qDebug() << "Setting JingleSession in the JabberJingleSession :" << (unsigned int) sess;
	m_jingleSession = sess;
	connect(sess, SIGNAL(needData(XMPP::JingleContent*)), this, SLOT(writeRtpData(XMPP::JingleContent*)));
}

void JabberJingleSession::setMediaManager(JingleMediaManager* mm)
{
	m_mediaManager = mm;
}

void JabberJingleSession::writeRtpData(XMPP::JingleContent* content)
{
	qDebug() << "Called void JabberJingleSession::writeRtpData(XMPP::JingleContent* content)";
	JabberJingleContent *jContent = contentWithName(content->name());
	if (jContent == 0)
	{
		jContent = new JabberJingleContent(this, content);
		jContent->setContent(content);
		jabberJingleContents << jContent;
	}
	jContent->startWritingRtpData();
	//FIXME:need different m_rtpSession for each content.
}

JabberJingleContent *JabberJingleSession::contentWithName(const QString& name)
{
	for (int i = 0; i < jabberJingleContents.count(); i++)
	{
		if (jabberJingleContents[i]->contentName() == name)
			return jabberJingleContents[i];
	}
	return 0;
}
