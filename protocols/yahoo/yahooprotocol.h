/*
    yahooprotocol.h - Yahoo Plugin for Kopete

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2003-2004 by Matt Rogers <mattrogers@sbcglobal.net

    Copyright (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOPROTOCOL_H
#define YAHOOPROTOCOL_H

#include "libyahoo2/yahoo2.h"
#include "libyahoo2/yahoo2_callbacks.h"

// Local Includes
#include "kyahoo.h"

// Kopete Includes

// QT Includes
#include <qpixmap.h>
#include <qmap.h>

// KDE Includes
#include "kopeteprotocol.h"
#include "kopetecontactproperty.h"

class YahooContact;
class KPopupMenu;
class KActionMenu;
class KAction;
class KopeteMetaContact;
class KopeteMessage;
class YahooPreferences;
class KopeteOnlineStatus;

class YahooProtocol : public KopeteProtocol
{
	Q_OBJECT
public:
	YahooProtocol( QObject *parent, const char *name, const QStringList &args );
	~YahooProtocol();

	//Online Statuses
	const KopeteOnlineStatus Offline;
	const KopeteOnlineStatus Online;
	const KopeteOnlineStatus BeRightBack;
	const KopeteOnlineStatus Busy;
	const KopeteOnlineStatus NotAtHome;
	const KopeteOnlineStatus NotAtMyDesk;
	const KopeteOnlineStatus NotInTheOffice;
	const KopeteOnlineStatus OnThePhone;
	const KopeteOnlineStatus OnVacation;
	const KopeteOnlineStatus OutToLunch;
	const KopeteOnlineStatus SteppedOut;
	const KopeteOnlineStatus Invisible;
	const KopeteOnlineStatus Custom;
	const KopeteOnlineStatus Idle;
	const KopeteOnlineStatus Connecting;

	const Kopete::ContactPropertyTmpl awayMessage;

	/** Protocol Accessor **/
	static YahooProtocol *protocol();

	virtual KopeteContact *deserializeContact( KopeteMetaContact *metaContact,
					 const QMap<QString,QString> &serializedData,
					 const QMap<QString, QString> &addressBookData );

	KopeteOnlineStatus statusFromYahoo( int status );

public slots:
	virtual AddContactPage *createAddContactWidget(QWidget * parent, KopeteAccount* a);
	virtual KopeteEditAccountWidget *createEditAccountWidget(KopeteAccount *account, QWidget *parent);
	virtual KopeteAccount *createNewAccount(const QString &accountId);


private:
	static YahooProtocol* s_protocolStatic_;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

