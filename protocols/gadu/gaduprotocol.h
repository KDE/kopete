// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 2002-2003 Zack Rusin 	<zack@kde.org>
//
// gaduprotocol.cpp
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

#ifndef GADUPROTOCOL_H
#define GADUPROTOCOL_H

#include <qmap.h>

#include "kopeteprotocol.h"
#include "kopeteonlinestatus.h"
#include "kopetecontactproperty.h"

#include "gaducommands.h"

class KAction;
class KActionMenu;

class QWidget;
class QString;

namespace Kopete { class Contact; }
namespace Kopete { class MetaContact; }

class GaduSession;
class GaduContact;
class GaduAccount;
class GaduPreferences;

#define GG_STATUS_CONNECTING 0x0100

class GaduProtocol : public Kopete::Protocol
{
	Q_OBJECT

public:
	GaduProtocol( QObject* parent, const char* name, const QStringList& str);
	~GaduProtocol();

	static GaduProtocol *protocol();

	// Plugin reimplementation
	// {
	AddContactPage* createAddContactWidget( QWidget* parent, Kopete::Account* account );
	Kopete::Account* createNewAccount( const QString& accountId );
	KopeteEditAccountWidget *createEditAccountWidget( Kopete::Account* account, QWidget* parent );
	bool canSendOffline() const { return true; }

	virtual Kopete::Contact *deserializeContact( Kopete::MetaContact* metaContact,
						 const QMap<QString, QString>& serializedData,
						 const QMap<QString, QString>& addressBookData );
	// }
	//!Plugin reimplementation

	Kopete::OnlineStatus convertStatus( uint ) const;
	bool statusWithDescription( uint status );

	uint statusToWithDescription( Kopete::OnlineStatus status );
	uint statusToWithoutDescription( Kopete::OnlineStatus status );
	
	const Kopete::ContactPropertyTmpl propFirstName;
	const Kopete::ContactPropertyTmpl propLastName;
	const Kopete::ContactPropertyTmpl propEmail;
	const Kopete::ContactPropertyTmpl propAwayMessage;
	const Kopete::ContactPropertyTmpl propPhoneNr;
	//const Kopete::ContactPropertyTmpl propIgnore;

private slots:
	void settingsChanged();

private:
	static GaduProtocol*	protocolStatic_;
	GaduAccount*		defaultAccount_;
	//GaduPreferences*	prefs_;

	const Kopete::OnlineStatus gaduStatusBlocked_;
	const Kopete::OnlineStatus gaduStatusOffline_;
	const Kopete::OnlineStatus gaduStatusOfflineDescr_;
	const Kopete::OnlineStatus gaduStatusBusy_;
	const Kopete::OnlineStatus gaduStatusBusyDescr_;
	const Kopete::OnlineStatus gaduStatusInvisible_;
	const Kopete::OnlineStatus gaduStatusInvisibleDescr_;
	const Kopete::OnlineStatus gaduStatusAvail_;
	const Kopete::OnlineStatus gaduStatusAvailDescr_;
	const Kopete::OnlineStatus gaduConnecting_;

};


#endif
