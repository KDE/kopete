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
namespace Kopete { class MetaContact; }
namespace Kopete { class Message; }
class YahooPreferences;
namespace Kopete { class OnlineStatus; }

class YahooProtocol : public Kopete::Protocol
{
	Q_OBJECT
public:
	YahooProtocol( QObject *parent, const char *name, const QStringList &args );
	~YahooProtocol();

	//Online Statuses
	const Kopete::OnlineStatus Offline;
	const Kopete::OnlineStatus Online;
	const Kopete::OnlineStatus BeRightBack;
	const Kopete::OnlineStatus Busy;
	const Kopete::OnlineStatus NotAtHome;
	const Kopete::OnlineStatus NotAtMyDesk;
	const Kopete::OnlineStatus NotInTheOffice;
	const Kopete::OnlineStatus OnThePhone;
	const Kopete::OnlineStatus OnVacation;
	const Kopete::OnlineStatus OutToLunch;
	const Kopete::OnlineStatus SteppedOut;
	const Kopete::OnlineStatus Invisible;
	const Kopete::OnlineStatus Custom;
	const Kopete::OnlineStatus Idle;
	const Kopete::OnlineStatus Connecting;

	const Kopete::ContactPropertyTmpl awayMessage;

	/** Protocol Accessor **/
	static YahooProtocol *protocol();

	virtual Kopete::Contact *deserializeContact( Kopete::MetaContact *metaContact,
					 const QMap<QString,QString> &serializedData,
					 const QMap<QString, QString> &addressBookData );

	Kopete::OnlineStatus statusFromYahoo( int status );

public slots:
	virtual AddContactPage *createAddContactWidget(QWidget * parent, Kopete::Account* a);
	virtual KopeteEditAccountWidget *createEditAccountWidget(Kopete::Account *account, QWidget *parent);
	virtual Kopete::Account *createNewAccount(const QString &accountId);


private:
	static YahooProtocol* s_protocolStatic_;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

