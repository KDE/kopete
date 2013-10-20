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

class ICQProtocol;
class KToggleAction;

class AIMContact : public AIMContactBase
{
Q_OBJECT

public:
	AIMContact( Kopete::Account*, const QString&, Kopete::MetaContact*, 
	            const QString& icon = QString() );
	virtual ~AIMContact();

	bool isReachable();
	
	/**
	 * Returns a set of custom menu items for
	 * the context menu
	 */
	virtual QList<KAction*> *customContextMenuActions();
	using AIMContactBase::customContextMenuActions;

	virtual void setSSIItem( const OContact& ssiItem );

public slots:
	void userInfoUpdated( const QString& contact, const UserDetails& details );
	void userOnline( const QString& userId );
	void userOffline( const QString& userId );

private slots:

	void slotIgnore();
	void slotVisibleTo();
	void slotInvisibleTo();

private:
	ICQProtocol* mProtocol;

	KAction *m_selectEncoding;

	KToggleAction *m_actionIgnore;
	KToggleAction *m_actionVisibleTo;
	KToggleAction *m_actionInvisibleTo;

};
#endif 
//kate: tab-width 4; indent-mode csands;
