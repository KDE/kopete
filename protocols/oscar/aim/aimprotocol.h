/*
 oscarprotocol.h  -  Oscar Protocol Plugin

 Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
 Copyright (c) 2005 by Matt Rogers <mattr@kde.org>
 Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */

#ifndef AIMPROTOCOL_H
#define AIMPROTOCOL_H

#include "oscarprotocol.h"
#include "kopetemimetypehandler.h"
#include "kopeteonlinestatus.h"

class AIMStatusManager;

class AIMProtocolHandler : public Kopete::MimeTypeHandler
{
public:
	AIMProtocolHandler();
	void handleURL( const QString&, const QUrl & url ) const Q_DECL_OVERRIDE;
	using Kopete::MimeTypeHandler::handleURL;
};

class AIMProtocol : public OscarProtocol
{
	Q_OBJECT

public:
	AIMProtocol( QObject *parent, const QVariantList &args );
	virtual ~AIMProtocol();
	/**
	 * Return the active instance of the protocol
	 * because it's a singleton, can only be used inside AIM classes, not in oscar lib
	 */
	static AIMProtocol *protocol();

	bool canSendOffline() const { return false; }

	AddContactPage*createAddContactWidget( QWidget *parent, Kopete::Account *account ) Q_DECL_OVERRIDE;
	KopeteEditAccountWidget* createEditAccountWidget( Kopete::Account *account, QWidget *parent ) Q_DECL_OVERRIDE;
	Kopete::Account* createNewAccount( const QString &accountId ) Q_DECL_OVERRIDE;

	OscarStatusManager *statusManager() const Q_DECL_OVERRIDE;

	const Kopete::PropertyTmpl clientProfile;

private:
	/** The active instance of oscarprotocol */
	static AIMProtocol *protocolStatic_;
	AIMStatusManager* statusManager_;
	AIMProtocolHandler protohandler;
};

#endif
