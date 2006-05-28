/*
    msnchatsession.h - MSN Message Manager

    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart @ kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNMESSAGEMANAGER_H
#define MSNMESSAGEMANAGER_H

#include "kopetechatsession.h"

class MSNSwitchBoardSocket;
class KActionCollection;
class MSNInvitation;
class MSNContact;
class KActionMenu;
class QLabel;


/**
 * @author Olivier Goffart
 */
class KOPETE_EXPORT MSNChatSession : public Kopete::ChatSession
{
	Q_OBJECT

public:
	MSNChatSession( Kopete::Protocol *protocol, const Kopete::Contact *user, Kopete::ContactPtrList others, const char *name = 0 );
	~MSNChatSession();

	void createChat( const QString &handle, const QString &address, const QString &auth, const QString &ID = QString::null );

	MSNSwitchBoardSocket *service() { return m_chatService; };

	void sendFile( const QString &fileLocation, const QString &fileName,
		long unsigned int fileSize );

	/**
	 * append an invitation in the invitation map, and send the first invitation message
	 */
	void initInvitation(MSNInvitation* invitation);
	
	virtual void inviteContact(const QString& );

public slots:
	void slotCloseSession();
	void slotInviteOtherContact();

	void invitationDone( MSNInvitation*  );

	void slotRequestPicture();

	/**
	 * this is a reimplementation of ChatSesstion slot.
	 * the original slot is not virtual, but that's not a problem because it's a slot.
	 */
	virtual void receivedTypingMsg( const QString &, bool );
	
	void slotConnectionTimeout();

private slots:
	void slotMessageSent( Kopete::Message &message, Kopete::ChatSession *kmm );
	void slotMessageReceived( Kopete::Message &message );

	void slotUserJoined( const QString &handle, const QString &publicName, bool IRO );
	void slotUserLeft( const QString &handle, const QString &reason );
	void slotSwitchBoardClosed();
	void slotInviteContact( Kopete::Contact *contact );
	void slotAcknowledgement( unsigned int id, bool ack );
	void slotInvitation( const QString &handle, const QString &msg );

	void slotActionInviteAboutToShow();

	void slotDisplayPictureChanged();

	/**
	 * (debug)
	 */
	void slotDebugRawCommand();

	void slotSendNudge();
	void slotWebcamReceive();
	void slotWebcamSend();
	void slotSendFile();

	void slotNudgeReceived(const QString& handle);

private:

	MSNSwitchBoardSocket *m_chatService;
	QString otherString;
	KActionMenu *m_actionInvite;
	QPtrList<KAction> m_inviteactions;
	KAction *m_actionNudge;
	KAction *m_actionWebcamReceive;
	KAction *m_actionWebcamSend;

	//Messages sent before the ending of the connection are queued
	QValueList<Kopete::Message> m_messagesQueue;
	void sendMessageQueue();
	void cleanMessageQueue( const QString &reason);
	void startChatSession();

	QMap<unsigned int, Kopete::Message> m_messagesSent;

	QMap<long unsigned int, MSNInvitation*> m_invitations;


	/**
	 * weither or not the "has opened a new chat" message need to be sent if the user is typing
	 */
	bool m_newSession;

	QLabel *m_image;
	QTimer *m_timeoutTimer;
	uint m_connectionTry;


signals:
	/*
	 * This signal is relayed to the protocol and after, to plugins
	 */
	void invitation(MSNInvitation*& invitation,  const QString &bodyMSG , long unsigned int cookie , MSNChatSession* msnMM , MSNContact* c );
};

#endif

// vim: set noet ts=4 sts=4 tw=4:

