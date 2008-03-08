/*
 * messengercontact.h - Windows Live Messenger Kopete Contact.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#ifndef MESSENGERCONTACT_H
#define MESSENGERCONTACT_H

#include <QMap>
#include <QList>
#include <kaction.h>
#include <kopetecontact.h>
#include <kopetechatsession.h>
#include <kopetemetacontact.h>
//#include <kopeteonlinestatus.h>
#include <kopetegroup.h>
#include "messengeraccount.h"
#include "messengerprotocol.h"
//#include "ui/messengeruserinfowidget.h"

// namespace Kopete
// {
// 	class ChatSession;
// 	class MetaContact;
// }
//class MessengerAccount;

class MessengerContact : public Kopete::Contact
{
	Q_OBJECT
public:
	MessengerContact(MessengerAccount *account, const QString &contactId, Kopete::MetaContact *parent);
	~MessengerContact();

	virtual bool isReachable();
	virtual void serialize(QMap< QString, QString >& serializedData, QMap< QString, QString >& addressBookData);
	
	virtual QList<KAction *> *customContextMenuActions();
	virtual Kopete::ChatSession *manager( CanCreateFlags canCreate = CannotCreate );
	void closeUserInfoDialog();
	
	//bool isBlocked();

	QString guid();
	QString phoneHome();
	QString phoneWork();
	QString phoneMobile();

public slots:
	virtual void slotUserInfo();
	virtual void deleteContact();
	virtual void sendFile( const KUrl &sourceURL = KUrl(),
						   const QString &fileName = QString(), uint fileSize = 0L );
	void slotUserProfile();
	void slotBlockUser();
	void slotSendMail();

	virtual void sync( unsigned int cvhanged= 0xff);
private:
	QMap<QString, Kopete::Group *> m_serverGroups;
	//MessengerUserInfoWidget *m_infoWidget;

	bool m_blocked;
	bool m_allowed;
	bool m_deleted;
	bool m_reversed;
	bool m_moving;
	bool m_phone_mob;
	
	uint m_clientFlags;

	QString m_phoneHome;
	QString m_phoneWork;
	QString m_phoneMobile;


	KAction *actionBlock;
	KAction *actionShowProfile;
	KAction *actionSendMail;
	KAction *actionWebcamReceive;
	KAction *actionWebcamSend;

	QString m_obj; //the MessengerObject

	//Kopete::OnlineStatus m_currentStatus;

	//MSNProtocol::deserializeContact need to acess some contact insternals
	friend class MessengerProtocol;
};
#endif
