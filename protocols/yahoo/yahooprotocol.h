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

// Kopete Includes
#include "kopeteonlinestatus.h"

// QT Includes
#include <qpixmap.h>
#include <qmap.h>

// KDE Includes
#include "kopeteprotocol.h"
#include "kopeteproperty.h"

namespace Kopete { class MetaContact; }
namespace Kopete { class Message; }
namespace Kopete { class OnlineStatus; }

class YahooProtocol : public Kopete::Protocol
{
	Q_OBJECT
public:
	YahooProtocol( QObject *parent, const QVariantList &args );
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
	const Kopete::OnlineStatus OnSMS;
	const Kopete::OnlineStatus Invisible;
	const Kopete::OnlineStatus Custom;
	const Kopete::OnlineStatus Idle;
	const Kopete::OnlineStatus Connecting;

	const Kopete::PropertyTmpl 	iconCheckSum;
	const Kopete::PropertyTmpl 	iconExpire;
	const Kopete::PropertyTmpl 	iconRemoteUrl;

	// Personal
	const Kopete::PropertyTmpl	propfirstName;
	const Kopete::PropertyTmpl	propSecondName;
	const Kopete::PropertyTmpl	propLastName;
	const Kopete::PropertyTmpl	propNickName;
	const Kopete::PropertyTmpl	propTitle;

	// Primary Information	
	const Kopete::PropertyTmpl	propPhoneMobile;
	const Kopete::PropertyTmpl	propEmail;
	const Kopete::PropertyTmpl	propYABId;

	// Additional Information
	const Kopete::PropertyTmpl	propPager;
	const Kopete::PropertyTmpl	propFax;
	const Kopete::PropertyTmpl	propAdditionalNumber;
	const Kopete::PropertyTmpl	propAltEmail1;
	const Kopete::PropertyTmpl	propAltEmail2;
	const Kopete::PropertyTmpl	propImAIM;
	const Kopete::PropertyTmpl	propImICQ;
	const Kopete::PropertyTmpl	propImMSN;
	const Kopete::PropertyTmpl	propImGoogleTalk;
	const Kopete::PropertyTmpl	propImSkype;
	const Kopete::PropertyTmpl	propImIRC;
	const Kopete::PropertyTmpl	propImQQ;

	// Private Information
	const Kopete::PropertyTmpl	propPrivateAddress;
	const Kopete::PropertyTmpl	propPrivateCity;
	const Kopete::PropertyTmpl	propPrivateState;
	const Kopete::PropertyTmpl	propPrivateZIP;
	const Kopete::PropertyTmpl	propPrivateCountry;
	const Kopete::PropertyTmpl	propPrivatePhone;
	const Kopete::PropertyTmpl	propPrivateURL;
		
	// Work Information
	const Kopete::PropertyTmpl	propCorporation;
	const Kopete::PropertyTmpl	propWorkAddress;
	const Kopete::PropertyTmpl	propWorkCity;
	const Kopete::PropertyTmpl	propWorkState;
	const Kopete::PropertyTmpl	propWorkZIP;
	const Kopete::PropertyTmpl	propWorkCountry;
	const Kopete::PropertyTmpl	propWorkPhone;
	const Kopete::PropertyTmpl	propWorkURL;

	// Miscellaneous
	const Kopete::PropertyTmpl	propBirthday;
	const Kopete::PropertyTmpl	propAnniversary;
	const Kopete::PropertyTmpl	propNotes;
	const Kopete::PropertyTmpl	propAdditional1;
	const Kopete::PropertyTmpl	propAdditional2;
	const Kopete::PropertyTmpl	propAdditional3;
	const Kopete::PropertyTmpl	propAdditional4;

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

