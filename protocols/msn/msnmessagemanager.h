/***************************************************************************
                          msnmessagemanager.h  -  description
                             -------------------
    begin                : dim oct 20 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MSNMESSAGEMANAGER_H
#define MSNMESSAGEMANAGER_H


#include "kopetemessagemanager.h"

/**
  *@author Olivier Goffart
  */

class MSNSwitchBoardSocket;
class KActionCollection;
class MSNFileTransferSocket;
class KopeteTransfer;
class KopeteFileTransferInfo;

class MSNMessageManager : public KopeteMessageManager
{
   Q_OBJECT
public: 
	MSNMessageManager(const KopeteContact *user, KopeteContactPtrList others, QString logFile=QString::null, const char *name=0);
	~MSNMessageManager();

	void createChat(const QString &handle, const QString &adress, const QString &auth, const QString &ID=QString::null);

	KActionCollection * chatActions();

	MSNSwitchBoardSocket *service() { return m_chatService; };
  /** No descriptions */
  void sendFile(const QString& file);

private:
	MSNSwitchBoardSocket *m_chatService;
//	KopeteMessage *m_msgQueued;
	QString otherString;
	KActionCollection *m_actions;
	bool m_timerOn;
	QMap<const KopeteContact*,QTime> typingMap ;

	//Messages sent before the ending of the connection are queued
	QValueList<KopeteMessage> m_messagesQueue;
	void sendMessageQueue();

	QMap<unsigned int, KopeteMessage> m_messagesSent;

	QMap<long unsigned int, MSNFileTransferSocket*> m_invitations;

	
protected slots: // Protected slots
	virtual void slotTyping(bool t);
  
private slots: // Private slots
	void slotMessageSent(const KopeteMessage &message,KopeteMessageManager *);

	void slotUpdateChatMember(const QString&, const QString&,bool);
	void slotUserTypingMsg( const QString& );
	void slotSwitchBoardClosed();
	void slotInviteContact(const QString &_handle);
	void slotAcknowledgement(unsigned int id, bool ack);
	void slotInvitation(const QString &handle, const QString &msg);

	void slotFileTransferAccepted(KopeteTransfer *trans, const QString& fileName);
	void slotFileTransferDone(MSNFileTransferSocket* MFTS);
	void slotFileTransferRefused(const KopeteFileTransferInfo&);

	void slotTimer();

public slots:
	void slotCloseSession();

};

#endif
