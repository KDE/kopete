/***************************************************************************
                          msncontact.h  -  description
                             -------------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MSNCONTACT_H
#define MSNCONTACT_H

#include <qlistview.h>
#include "imcontact.h"
#include <qobject.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qvaluestack.h>
#include <kiconloader.h>
#include <qtimer.h>
#include "kopete.h"
#include "msnprotocol.h"
#include "msnmessage.h"
#include <kmsnchatservice.h>

struct MSNMessageStruct
{
	QString userid;
	QString message;
};

class MSNMessage;

class MSNContact : public IMContact
{
	
	Q_OBJECT
	public:
		MSNContact(QString userid, const QString name, MSNProtocol *protocol);
		virtual void rightButtonPressed(const QPoint &);
		virtual void leftButtonDoubleClicked();
		QString mUserID;
		QString mName;
	public slots:
		void slotMessageBoxClosing();
		void slotIncomingChat(KMSNChatService *, QString);
	private slots:
		void removeThisUser();
		void slotUpdateContact (QString, uint);
		// We have to delete the contact if MSN disconenct
		// We will use the engine signal
		void slotDeleteMySelf ( bool );
		void slotNewMessage(QString, QString, QString);
		void slotFlashIcon();
	private:
		QString mStatus;
		uint mStatus_n;
		bool isMessageIcon;
		//QPixmap onlineIcon;
		//QPixmap offlineIcon;
		MSNProtocol *mProtocol;
		QValueStack<MSNMessageStruct> *messageQueue;
		QTimer *messageTimer;
		MSNMessage *messageBox;
		bool messageBoxInited;
	signals:
		void chatToUser( QString );
};

#endif
