/***************************************************************************
                          imchatservice.h  -  description
                             -------------------
    begin                : Tue Nov 27 2001
    copyright            : (C) 2001 by Olaf Lueg
    email                : olueg@olsd.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef KMSNCHATSERVICE_H
#define KMSNCHATSERVICE_H

#include <qobject.h>
#include <kstringhandler.h>
#include <qstrlist.h>


/**
  *@author Olaf Lueg
  */

class QSocket;
class KMSNService;
class KExtendedSocket;

class KMSNChatService : public QObject  {
Q_OBJECT
public: 
	KMSNChatService();
	~KMSNChatService();
	KExtendedSocket *msgSocket;
	QString msgHandle;
	KStringHandler kstr;
	QSocket *fileSocket;
	KMSNService *imService;
	int socketTimer;

protected:
	QString myHandle;
	QString buffer;
// functions
	QString readLine();
	bool canReadLine();
	QString readBlock(uint len);
	void timerEvent(QTimerEvent *ev);
	QString parseFontAttr(QString str, QString attr);
public:
	void connectToSwitchBoard(QString ID, QString address, QString auth);
	void callUser();
	void setHandle(QString handle){myHandle = handle;}
// slots
public slots:
	void slotDataReceived();
	void slotSendMsg(QString message);
	void slotSocketClosed();
	void slotCloseSession();
	void slotInviteContact(QString handle);
	void slotTypingMsg();
  /** No descriptions */

// signals	
signals:
  	void msgReceived(QString, QString, QString , QFont, QColor);
	void startChat(KMSNChatService* switchoard);
  	void userTypingMsg(QString);
  	void msgAcknowledgement(bool);
  	void userInChat(QString);
  	void chatWith(QString,bool);
  	void switchBoardIsActive(bool);
  	void updateChatMember(QString,QString,bool);
};

#endif
