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

class MSNMessageManager : public KopeteMessageManager
{
   Q_OBJECT
public: 
	MSNMessageManager(const KopeteContact *user, KopeteContactPtrList others, QString logFile=QString::null, const char *name=0);
	~MSNMessageManager();

	void createChat(QString handle, QString adress, QString auth, QString ID=QString::null);

	KActionCollection * chatActions();

	MSNSwitchBoardSocket *service() { return m_chatService; };

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

	
protected slots: // Protected slots
	virtual void slotTyping(bool t);
  
private slots: // Private slots
	void slotMessageSent(const KopeteMessage &message,KopeteMessageManager *);
	void slotUpdateChatMember(QString,QString,bool);
	void slotUserTypingMsg( QString );
	void slotSwitchBoardClosed();
	void slotInviteContact(const QString &_handle);
	void slotAcknowledgement(unsigned int id, bool ack);

public slots:
	void slotCloseSession();
  /** No descriptions */
  void slotTimer();
};

#endif
