/*
    yahoocontact.h - Yahoo Contact

    Copyright (c) 2003-2004 by Matt Rogers <matt.rogers@kdemail.net>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Portions based on code by Bruno Rodrigues <bruno.rodrigues@litux.org>

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOCONTACT_H
#define YAHOOCONTACT_H

/* Kopete Includes */
#include "kopetecontact.h"

namespace Kopete { class MessageManager; }
namespace Kopete { class MetaContact; }
namespace Kopete { class OnlineStatus; }
class YahooProtocol;
class YahooAccount;

class YahooContact : public Kopete::Contact
{
	Q_OBJECT
public:
	YahooContact( YahooAccount *account, const QString &userId, const QString &fullName, Kopete::MetaContact *metaContact );
	~YahooContact();

	/** Base Class Reimplementations **/
	virtual bool isOnline() const;
	virtual bool isReachable();
	virtual QPtrList<KAction> *customContextMenuActions();
	virtual Kopete::MessageManager *manager( Kopete::Contact::CanCreateFlags canCreate= Kopete::Contact::CanCreate );
	virtual void serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData );

	void setYahooStatus( const Kopete::OnlineStatus& );

	/** The group name getter and setter methods**/
	QString group() const;
	void setGroup( const QString& );


public slots:
	virtual void slotUserInfo();
	virtual void slotSendFile();
	virtual void deleteContact();

	/**
	 * Must be called after the contact list has been received
	 * or it doesn't work well!
	 */
	void syncToServer();

	void sync(unsigned int);

private slots:
	void slotMessageManagerDestroyed();
	void slotSendMessage( Kopete::Message& );
	void slotTyping( bool );

private:
	//the user id of the contact
	QString m_userId;

	//the group name of the contact
	QString m_groupName;

	//The message manager
	Kopete::MessageManager *m_manager;

	//the account that this contact belongs to
	YahooAccount* m_account;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

