#include "jabberjinglesession.h"

#include "jinglesession.h"

//using namespace XMPP;

JabberJingleSession::JabberJingleSession()
{

}

JabberJingleSession::~JabberJingleSession()
{

}

void JabberJingleSession::setJingleSession(XMPP::JingleSession* sess)
{
	m_jingleSession = sess;
	connect(sess, SIGNAL(needData(const QDomElement&)), this, SLOT(writeRtpData(const QDomElement&)));
}

void JabberJingleSession::writeRtpData(const QDomElement& payload)
{
	
}
