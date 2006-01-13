//
// C++ Interface: jinglevoicesession
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef JINGLEVOICESESSION_H
#define JINGLEVOICESESSION_H

#include <jinglesession.h>

#include <xmpp.h> // XMPP::Jid
#include <qvaluelist.h>

namespace cricket
{
	class Call;
}

class JabberAccount;
class JingleSession;

/**
 * Implement a Jingle voice peer-to-peer session that is compatible with Google Talk voice offering.
 *
 * @author Michaël Larouche
*/
class JingleVoiceSession : public JingleSession
{
	Q_OBJECT
public:
	typedef QValueList<XMPP::Jid> JidList;

	JingleVoiceSession(JabberAccount *account, const JidList &peers);
	virtual ~JingleVoiceSession();

	virtual QString sessionType();

public slots:
	virtual void accept();
	virtual void decline();
	virtual void start();
	virtual void terminate();

protected slots:
	void receiveStanza(const QString &stanza);

private:
	void setCall(cricket::Call *call);

	class Private;
	Private *d;

	class SlotsProxy;
	SlotsProxy *slotsProxy;
	
	class JingleIQResponder;
};

#endif
