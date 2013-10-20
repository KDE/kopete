/*
 aimcontact.h  -  Oscar Protocol Plugin

 Copyright (c) 2003 by Will Stephenson
 Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
 Copyright (c) 2006 by Roman Jarosz <kedgedev@centrum.cz>
 Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

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

#include "aimcontactbase.h"

class AIMProtocol;
class AIMUserInfoDialog;
class KToggleAction;

class AIMContact : public AIMContactBase
{
Q_OBJECT

public:
	AIMContact( Kopete::Account*, const QString&, Kopete::MetaContact*, 
	            const QString& icon = QString() );
	virtual ~AIMContact();

	bool isReachable();
	QList<KAction*> *customContextMenuActions();
	using AIMContactBase::customContextMenuActions;

	int warningLevel() const;

	virtual void setSSIItem( const OContact& ssiItem );

public slots:
	void slotUserInfo();
	void userInfoUpdated( const QString& contact, const UserDetails& details );
	void userOnline( const QString& userId );
	void userOffline( const QString& userId );
	void updateProfile( const QString& contact, const QString& profile );
	void gotWarning( const QString& contact, quint16, quint16 );

signals:
	void updatedProfile();

private slots:
	void closeUserInfoDialog();
	void warnUser();

	void slotVisibleTo();
	void slotInvisibleTo();

private:
	AIMProtocol* mProtocol;
	AIMUserInfoDialog* m_infoDialog;
	
	KAction* m_warnUserAction;
	KToggleAction *m_actionVisibleTo;
	KToggleAction *m_actionInvisibleTo;

};
#endif 
//kate: tab-width 4; indent-mode csands;
