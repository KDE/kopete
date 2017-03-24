/*
  icqcontact.h  -  ICQ Contact

  Copyright (c) 2003 by Stefan Gehn  <metz@gehn.net>
  Copyright (c) 2003 by Olivier Goffart <ogoffart@kde.org>
  Copyright (c) 2004 by Richard Smith <kde@metafoo.co.uk>
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

#ifndef ICQCONTACT_H
#define ICQCONTACT_H

#include "icqcontactbase.h"
#include <QList>

class OContact;

class AIMProtocol;
class KToggleAction;

/**
 * Contact for ICQ over Oscar protocol
 * @author Stefan Gehn
 * @author Richard Smith
 * @author Matt Rogers
 */
class ICQContact : public ICQContactBase
{
Q_OBJECT

public:

	/** Normal ICQ constructor */
	ICQContact( Kopete::Account* account, const QString &name, Kopete::MetaContact *parent,
	            const QString& icon = QString() );
	virtual ~ICQContact();

	/**
	 * Returns a set of custom menu items for
	 * the context menu
	 */
	QList<QAction*> *customContextMenuActions() Q_DECL_OVERRIDE;
	using ICQContactBase::customContextMenuActions;

	/** Return whether or not this contact is reachable. */
	bool isReachable() Q_DECL_OVERRIDE;

	void setSSIItem( const OContact& ssiItem ) Q_DECL_OVERRIDE;

public slots:
	void userInfoUpdated( const QString& contact, const UserDetails& details ) Q_DECL_OVERRIDE;

	void userOnline( const QString& userId ) Q_DECL_OVERRIDE;
	void userOffline( const QString& userID ) Q_DECL_OVERRIDE;
	void loggedIn();

private:
	AIMProtocol *mProtocol;

	KToggleAction *m_actionVisibleTo;
	KToggleAction *m_actionInvisibleTo;

private slots:
	void slotVisibleTo();
	void slotInvisibleTo();
};

#endif
