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

class KopeteMessageManager;
class KopeteMetaContact;
class KopeteOnlineStatus;
class YahooProtocol;
class YahooAccount;

class YahooContact : public KopeteContact
{
	Q_OBJECT
public:
	YahooContact( YahooAccount *account, const QString &userId, const QString &fullName, KopeteMetaContact *metaContact );
	~YahooContact();

	/** Base Class Reimplementations **/
	virtual bool isOnline() const;
	virtual bool isReachable();
	virtual QPtrList<KAction> *customContextMenuActions();
	virtual KopeteMessageManager *manager( bool canCreate = false );
	virtual void serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData );

	void setYahooStatus( const KopeteOnlineStatus& );

	/** The group name getter and setter methods**/
	QString group() const;
	void setGroup( const QString& );


public slots:
	virtual void slotUserInfo();
	virtual void slotSendFile();
	virtual void slotDeleteContact();

	/**
	 * Must be called after the contact list has been received
	 * or it doesn't work well!
	 */
	void syncToServer();

private slots:
	void slotMessageManagerDestroyed();
	void slotSendMessage( KopeteMessage& );
	void slotTyping( bool );

private:
	//the user id of the contact
	QString m_userId;

	//the group name of the contact
	QString m_groupName;

	//The message manager
	KopeteMessageManager *m_manager;

	//the account that this contact belongs to
	YahooAccount* m_account;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

