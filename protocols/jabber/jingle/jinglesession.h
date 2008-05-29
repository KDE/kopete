/*
    jinglesession.h - Define a Jingle session.

    Copyright (c) 2006      by Michaël Larouche     <larouche@kde.org>

    Kopete    (c) 2001-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef JINGLESESSION_H
#define JINGLESESSION_H

//#include <qobject.h>
//#include <qstring.h>

#include <xmpp.h> // XMPP::Jid
#include <q3valuelist.h>

//#include "jingleconnectioncandidate.h"

struct JingleContentType;
class JingleTransport;


//BEGIN JingleStateEnum
enum JingleStateEnum{
	PENDING,
	ACTIVE,
	ENDED
};
//END JingleStateEnum

//BEGIN JingleLastMessageEnum
enum JingleLastMessageEnum{
	sessionInitiate,
	sessionAccept,
	contentAccept,
	sessionInfo,
	sessionTerminate,
	contentAdd,
	contentRemove,
	contentModify,
	transportInfo
};
//END JingleLastMessageEnum

class JabberAccount;
/**
 * @brief Base class for peer-to-peer session that use Jingle signaling
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class JingleSession : public QObject
{
	Q_OBJECT
public:
	typedef Q3ValueList<XMPP::Jid> JidList;

	JingleSession(JabberAccount *account, const JidList &peers);
	virtual ~JingleSession();

	/**
	 * Return the JabberAccount associated with this session.
	 */
	JabberAccount *account();

	const XMPP::Jid &myself() const;
	const JidList &peers() const;
	JidList &peers();
	
	/**
	 * Return the type of session(ex: voice, video, games)
	 * Note that you must return the XML namespace that define 
	 * the session: ex:(http://jabber.org/protocol/jingle/sessions/audio)
	 */
	virtual QString sessionType() = 0;

public slots:
	/**
	 * @brief Start a session with the give JID.
	 * You should begin the negotiation here.
	 */
	virtual void start() = 0;
	/**
	 * @brief Acept a session request.
	 */
	virtual void accept() = 0;
	/**
	 * @brief Decline a session request.
	 */
	virtual void decline() = 0;
	/**
	 * @brief Terminate a Jingle session.
	 */
	virtual void terminate() = 0;

protected slots:
	void sendStanza(const QString &stanza);
	
signals:
	/**
	 * Session is started(negotiation and connection test are done).
	 */
	void sessionStarted();

	void accepted();
	void declined();
	void terminated();

protected:
	/**
	 * Process the XMPP stanza, and take appropriate action.
	 * Some parts of this function will die if the stanza is malformed,
	 * checks need to be added
	 */
	void processStanza(QDomDocument doc);

	/**
	 * Removes designated content type.  If there are none left, closes the session.
	 * Otherwise, sends content-accept message.
	 */
	virtual void removeContent(QDomElement stanza);

	/**
	 * Updates the <tt>JingleContentType</tt> in <tt>types</tt> with the 
	 * same name as in the message with the properties from the message.
	 */
	virtual int updateContent(QDomElement stanza) = 0;

	/**
	 * Checks modifications to a JingleContentType in types.
	 * If the changes are acceptable, sends an accept-content
	 * or beings transport negociations.
	 */
	virtual void checkContent(QDomElement stanza) = 0;

	/**
	* Checks a new content type (from session-initiate or add-content).
	* Sends an error or a reciept, and then begins negociations.
	*/
	virtual void checkNewContent(QDomElement stanza) = 0;

	/**
	 * Sends all local candidates for the transport connection
	 * for content <tt>contentIndex</tt> in <tt>types</tt>
	 * to the remote machine.
	 */
	virtual void sendTransportCandidates(int contentIndex);

	/**
	 * Checks a remote candidate. If there is not a working connection,
	 * and this candidate works, uses it.
	 */
	virtual bool addRemoteCandidate(QDomElement contentElement) = 0;

	//virtual JingleTransport* transport() = 0;

	/**
	 * Returns true if the session is being modified.  Used to determine
	 * whether both parties are trying to modify the session at once.
	 */
	bool isModifying();

	//NOTE this does not scale to multiple-content sessions
	//JingleConnectionCandidate connection;

	/**
	 * Updates all contents with values from accepter,
	 * including the candidate the acceptor chose.
	 * If that candidate cannot be written to, 
	 * immediately returns false. Otherwise, returns true.
	 */
	virtual bool handleSessionAccept(QDomElement stanza) {return false;}

	//this should probably just be checked by namespace.
	//default: do nothing
	virtual void receiveSessionInfo(QDomElement stanza) {return;}

	QList<JingleContentType> types;

	JingleStateEnum state;
	JingleLastMessageEnum lastMessage;
	QString initiator;
	QString responder;
	QString sid;
	bool amIInitiator;
	QString transactionID;
	bool acknowledged;


private:
	class Private;
	Private *d;

	/**
	 * Deal with an <error> response.
	 */
	void handleError(QDomElement errorElement);
};

#endif
