// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 	2002-2003	 Zack Rusin 	<zack@kde.org>
//
// gaducontact.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#ifndef GADUCONTACT_H
#define GADUCONTACT_H

#include <qstringlist.h>
#include <qstring.h>
#include <qpoint.h>

#include "gaduaccount.h"
#include "gaducommands.h"
#include "gaducontactlist.h"

#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopetemessage.h"

#include "libgadu.h"

class KAction;
class GaduAccount;
class KopeteAccount;
class KopeteMessageManager;

class GaduContact : public KopeteContact
{
	Q_OBJECT

public:
	GaduContact( unsigned int, const QString&, KopeteAccount*, KopeteMetaContact* );

	virtual bool isReachable();
	virtual void serialize( QMap<QString, QString>&, QMap<QString, QString>& );
	virtual QPtrList<KAction>* customContextMenuActions();
	virtual QString identityId() const;

//	void	setInfo( const QString& email, const QString& firstName, const QString& secondName,
//			 	const QString& nickName, const QString& phonenr );
	GaduContactsList::ContactLine* contactDetails();

	void	setParentIdentity( const QString& );
	void	setDescription( const QString& );

	QString description() const;

	uin_t uin() const;

public slots:
	void slotUserInfo();
	void slotDeleteContact();
	void messageReceived( KopeteMessage& );
	void messageSend( KopeteMessage&, KopeteMessageManager* );
	void messageAck();
	void slotShowPublicProfile();
	void slotEditContact();

protected:
	virtual KopeteMessageManager* manager( bool canCreate = false );
	void initActions();

private:
	KopeteMessageManager* msgManager_;
	uin_t			uin_;
	QString		description_;
	QString		parentIdentity_;
	GaduAccount*	account_;

	KAction*		actionSendMessage_;
	KAction*		actionInfo_;
	KAction*		actionRemove_;

	KopeteContactPtrList	thisContact_;

private slots:
	void slotMessageManagerDestroyed();

};

#endif
