/*
 aimcontact.h  -  Oscar Protocol Plugin

 Copyright (c) 2003 by Will Stephenson
 Copyright (c) 2004 by Matt Rogers <mattr@kde.org> 
 Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
*/

#ifndef AIMCONTACT_H
#define AIMCONTACT_H

#include "oscarcontact.h"


namespace Kopete
{
class ChatSession;
}

class AIMAccount;
class AIMProtocol;
class AIMUserInfoDialog;

class AIMContact : public OscarContact
{
Q_OBJECT

public:
	AIMContact( Kopete::Account*, const QString&, Kopete::MetaContact*, 
	            const QString& icon = QString::null, const Oscar::SSI& ssiItem = Oscar::SSI() );
	virtual ~AIMContact();

	bool isReachable();
	QPtrList<KAction> *customContextMenuActions();

	const QString &userProfile() { return mUserProfile; }

	/** !!! Only usable for the myself() contact */
	void setOwnProfile( const QString &profile );

	virtual const QString awayMessage();
	virtual void setAwayMessage( const QString &message );
	
	int warningLevel() const;

public slots:
	void updateSSIItem();
	void slotUserInfo();
	void userInfoUpdated( const QString& contact, const UserDetails& details );
	void userOnline( const QString& userId );
	void userOffline( const QString& userId );
	void updateAwayMessage( const QString& userId, const QString& message );
	void updateProfile( const QString& contact, const QString& profile );
	void gotWarning( const QString& contact, Q_UINT16, Q_UINT16 );

signals:
	void updatedProfile();

private slots:
	void closeUserInfoDialog();
	void warnUser();

private:
	AIMProtocol* mProtocol;
	AIMUserInfoDialog* m_infoDialog;
	QString mUserProfile;
	bool m_haveAwayMessage;
	
	KAction* m_warnUserAction;
};
#endif 
//kate: tab-width 4; indent-mode csands;
