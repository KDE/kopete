/*
    msnmessagemanager.h - MSN Message Manager

    Copyright (c) 2002 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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

#include "kopetemessagemanager.h"

class MSNSwitchBoardSocket;
class KActionCollection;
class MSNInvitation;
class MSNFileTransferSocket;


/**
 * @author Olivier Goffart
 */
class MSNMessageManager : public KopeteMessageManager
{
	Q_OBJECT

public:
	MSNMessageManager( KopeteProtocol *protocol, const KopeteContact *user, KopeteContactPtrList others, const char *name = 0 );
	~MSNMessageManager();

	void createChat( const QString &handle, const QString &adress, const QString &auth, const QString &ID = QString::null );

	KActionCollection * chatActions();

	MSNSwitchBoardSocket *service() { return m_chatService; };

	void sendFile( const QString &fileLocation, const QString &fileName,
		long unsigned int fileSize );

public slots:
	void slotCloseSession();
	void slotInviteOtherContact();

private slots:
	void slotMessageSent( KopeteMessage &message, KopeteMessageManager *kmm );

	void slotUpdateChatMember( const QString &handle, const QString &publicName, bool add );
	void slotSwitchBoardClosed();
	void slotInviteContact( KopeteContact *contact );
	void slotAcknowledgement( unsigned int id, bool ack );
	void slotInvitation( const QString &handle, const QString &msg );

	void slotFileTransferDone( MSNFileTransferSocket* MFTS );

	/**
	 * (debug)
	 */
	void slotDebugRawCommand();
private:

	MSNSwitchBoardSocket *m_chatService;
//	KopeteMessage *m_msgQueued;
	QString otherString;
	KActionCollection *m_actions;

	//Messages sent before the ending of the connection are queued
	QValueList<KopeteMessage> m_messagesQueue;
	void sendMessageQueue();

	QMap<unsigned int, KopeteMessage> m_messagesSent;

	QMap<long unsigned int, MSNInvitation*> m_invitations;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:

