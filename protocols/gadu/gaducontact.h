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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef GADUCONTACT_H
#define GADUCONTACT_H

#include <qpoint.h>
#include <qhostaddress.h>
#include <qlist.h>

#include "gaducontactlist.h"

#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopetemessage.h"

#include <libgadu.h>

class KAction;
class GaduAccount;
namespace Kopete { class Account; }
namespace Kopete { class ChatSession; }
class KGaduNotify;
class QString;

class GaduContact : public Kopete::Contact
{
	Q_OBJECT

public:
	GaduContact( unsigned int, Kopete::Account*, Kopete::MetaContact* );

	virtual bool isReachable();
	virtual void serialize( QMap<QString, QString>&, QMap<QString, QString>& );
	virtual QList<KAction*>* customContextMenuActions();
	using Kopete::Contact::customContextMenuActions;
	virtual QString identityId() const;

	GaduContactsList::ContactLine* contactDetails();

	// this one set's only:
	// email, firstname, surname, phonenr, ignored, nickname
	// uin is const for GaduContact, and displayname needs to be changed through metaContact
	bool setContactDetails( const GaduContactsList::ContactLine* );

	void setParentIdentity( const QString& );
	void setIgnored( bool );
	bool ignored();

	static QString findBestContactName( const GaduContactsList::ContactLine* );
	void changedStatus( KGaduNotify* );

	uin_t uin() const;

	QHostAddress&  contactIp();
	unsigned short contactPort();
	
public slots:
	void slotUserInfo();
	void deleteContact();
	void messageReceived( Kopete::Message& );
	void messageSend( Kopete::Message&, Kopete::ChatSession* );
	void messageAck();
	void slotShowPublicProfile();
	void slotEditContact();
	virtual void sendFile( const KUrl &sourceURL = KUrl(),
		const QString &fileName = QString(), uint fileSize = 0L );


protected:
	virtual Kopete::ChatSession* manager( Kopete::Contact::CanCreateFlags canCreate = Kopete::Contact::CanCreate );
	void initActions();

private:
	const uin_t		uin_;
	bool 			ignored_;

	Kopete::ChatSession*	msgManager_;
	QString			description_;
	QString			parentIdentity_;
	GaduAccount*		account_;

	QList<Contact*>		thisContact_;

	QHostAddress remote_ip;
	unsigned int remote_port;
	unsigned int version;
	unsigned int image_size;


private slots:
	void slotChatSessionDestroyed();

};

#endif
