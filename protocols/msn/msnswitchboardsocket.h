/*
    msnswitchboardsocket.h - switch board connection socket

    Copyright (c) 2002      by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2006 by Olivier Goffart        <ogoffart@ kde.org>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    Portions of this code are taken from KMerlin,
              (c) 2001 by Olaf Lueg              <olueg@olsd.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNSWITCHBOARDSOCKET_H
#define MSNSWITCHBOARDSOCKET_H

#include <qobject.h>
#include <qstrlist.h>
#include <qvaluevector.h>

#include <kstringhandler.h>

#include "msnsocket.h"

namespace Kopete { class Message; }
class MSNAccount;
class QTimer;

class MSNP2PDisplatcher;
class KTempFile;

namespace P2P { class Dispatcher; }

#include "dispatcher.h"

class KOPETE_EXPORT MSNSwitchBoardSocket : public MSNSocket
{
	Q_OBJECT

public:
	/**
	 * Contructor: id is the KopeteMessageMangager's id
	 */
	MSNSwitchBoardSocket( MSNAccount * account , QObject *parent);
	~MSNSwitchBoardSocket();

private:
	P2P::Dispatcher *m_dispatcher;
	MSNAccount *m_account;

	QString m_myHandle; // our handle

	// contains the handle of the last person that msg'ed us.
	// since we receive the actual message by readBlock(), we need
	// to remember what the handle was of the person sending us the message.
	QString m_msgHandle;

	QString m_ID;
	QString m_auth;
	QStringList m_chatMembers;

	//used for emoticons
	QValueList<const Kopete::Message> m_msgQueue;
	unsigned  m_recvIcons;
	QMap<QString , QPair<QString , KTempFile*> >  m_emoticons;
	Kopete::Message &parseCustomEmoticons(Kopete::Message &msg);
	QTimer *m_emoticonTimer;
	QPtrList<KTempFile> m_typewrited;

	struct InkMessage{
		Q_UINT32 chunks;
		QString data;
	};
	QMap<QString, InkMessage> m_inkMessageBuffer;

	/** the number of chunk for currents messages */
	unsigned int m_chunks;

	/** true is we already sent the x-clientcaps message */
	bool m_clientcapsSent;

private:
	void DispatchInkMessage(const QString &base64String);

protected:
	/**
	 * Handle an MSN command response line.
	 */
	virtual void parseCommand( const QString &cmd, uint id,
		const QString &data );

	/**
	 * Handle exceptions that might occur during a chat.
	 */
	virtual void handleError( uint code, uint id );

	QString parseFontAttr( QString str, QString attr );


public:
	void connectToSwitchBoard( QString ID, QString address, QString auth );
	void setHandle( QString handle ) { m_myHandle = handle; }
	void setMsgHandle( QString handle ) { m_msgHandle = handle; }

	const QStringList &chatMembers() { return m_chatMembers; }

	void userLeftChat( const QString &handle , const QString &reason );
	int  sendMsg( const Kopete::Message &msg );
    int  sendCustomEmoticon(const QString &name, const QString &filename);

	int sendNudge();

	P2P::Dispatcher* PeerDispatcher();

public slots:
	void slotCloseSession();
	void slotInviteContact(const QString &handle);

	/**
	 * Notify the server that the user is typing a message
	 */
	void sendTypingMsg( bool isTyping );

	void requestDisplayPicture();
	
	/** workaround Bug 113425 . see slotKeepAliveTimer() **/
	QTimer *m_keepAlive;
	int m_keepAliveNb;
	


private slots:
	void slotOnlineStatusChanged( MSNSocket::OnlineStatus status );
	void slotSocketClosed(  );
	void slotReadMessage( const QByteArray &bytes );
	void slotEmoticonReceived( KTempFile *, const QString& );
	void slotIncomingFileTransfer(const QString& from, const QString& fileName, Q_INT64 fileSize);
	void cleanQueue();
	
	/** workaround Bug 113425 . see comment inside the function **/
	void slotKeepAliveTimer();

signals:
	void msgReceived( Kopete::Message &msg );
	void receivedTypingMsg( const QString &contactId, bool isTyping );
	void msgAcknowledgement(unsigned int, bool);
	void userJoined(const QString& handle , const QString &publicName , bool IRO);
	void userLeft(const QString& handle , const QString &reason);
	void nudgeReceived(const QString &handle);

	void switchBoardClosed(  );
	void invitation(const QString& handle, const QString& msg);

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

