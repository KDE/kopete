/*
 * messengerprotocol.h - Windows Live Messenger Kopete protocol definition.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#ifndef MESSENGERPROTOCOL_H
#define MESSENGERPROTOCOL_H

#include <kopeteprotocol.h>

#include <kopete_export.h>

namespace Kopete
{
	class Account;
}

class AddContactPage;
class KopeteEditAccountWidget;

/**
 * 
 * @author Michaël Larouche <larouche@kde.org>
 */
class MESSENGER_EXPORT MessengerProtocol : public Kopete::Protocol
{
public:
	MessengerProtocol(QObject *parent, const QStringList &args);
	
	virtual Kopete::Account *createNewAccount(const QString &accountId);
	virtual AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *account);
	virtual KopeteEditAccountWidget * createEditAccountWidget(Kopete::Account *account, QWidget *parent);

	static MessengerProtocol *protocol();

	virtual Kopete::Contact *deserializeContact( Kopete::MetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	const QMap<int, QString> &countries() { return mCountries; }
	const QMap<int, QString> &months() { return mMonths; }
	const QMap<int, QString> &days() { return mDays; }

	void MessengerProtocol::fillComboFromTable(QComboBox *box, const QMap<int, QString> &map);
	void MessengerProtocol::setComboFromTable(QComboBox *box, const QMap<int, QString> &map, int value);
	int MessengerProtocol::getCodeForCombo(QComboBox *cmb, const QMap<int, QString> &map);

	/**
	 * The possible Messenger online statuses
	 */
	const Kopete::OnlineStatus NLN;  //online
	const Kopete::OnlineStatus BSY;  //busy
	const Kopete::OnlineStatus BRB;  //be right back
	const Kopete::OnlineStatus AWY;  //away
	const Kopete::OnlineStatus PHN;  //on the phone
	const Kopete::OnlineStatus LUN;  //out to lunch
	const Kopete::OnlineStatus FLN;  //offline
	const Kopete::OnlineStatus HDN;  //invisible
	const Kopete::OnlineStatus IDL;  //idle
	const Kopete::OnlineStatus UNK;  //inknown (internal)
	const Kopete::OnlineStatus CNT;  //connecting (internal)

	//general User info
	const Kopete::ContactPropertyTmpl propEmail;
	const Kopete::ContactPropertyTmpl propContactType;
	const Kopete::ContactPropertyTmpl propFirstName;
	const Kopete::ContactPropertyTmpl propLastName;
	const Kopete::ContactPropertyTmpl propComment;
	const Kopete::ContactPropertyTmpl propAnniversary;
	const Kopete::ContactPropertyTmpl propBirthday;

	//Annotation
	const Kopete::ContactPropertyTmpl propABJobTitle;
	const Kopete::ContactPropertyTmpl propABNickName;
	const Kopete::ContactPropertyTmpl propABJobSpouse;

	//Email
	const Kopete::ContactPropertyTmpl propContactEmailBusiness;
	const Kopete::ContactPropertyTmpl propContactEmailMessenger;
	const Kopete::ContactPropertyTmpl propContactEmailOther;
	const Kopete::ContactPropertyTmpl propContactEmailPersonal;

	//Phone
	const Kopete::ContactPropertyTmpl propContactPhoneBusiness;
	const Kopete::ContactPropertyTmpl propContactPhoneFax;
	const Kopete::ContactPropertyTmpl propContactPhoneMobile;
	const Kopete::ContactPropertyTmpl propContactPhoneOther;	
	const Kopete::ContactPropertyTmpl propContactPhonePager;	
	const Kopete::ContactPropertyTmpl propContactPhonePersonal;	

	//Business Location
	const Kopete::ContactPropertyTmpl propBusinessName;
	const Kopete::ContactPropertyTmpl propBusinessStreet;
	const Kopete::ContactPropertyTmpl propBusinessCity;
	const Kopete::ContactPropertyTmpl propBusinessState;
	const Kopete::ContactPropertyTmpl propBusinessCountry;
	const Kopete::ContactPropertyTmpl propBusinessPostalCode;

	//Personal Location
	const Kopete::ContactPropertyTmpl propPersonalName;
	const Kopete::ContactPropertyTmpl propPersonalStreet;
	const Kopete::ContactPropertyTmpl propPersonalCity;
	const Kopete::ContactPropertyTmpl propPersonalState;
	const Kopete::ContactPropertyTmpl propPersonalCountry;
	const Kopete::ContactPropertyTmpl propPersonalPostalCode;

	//Website
	const Kopete::ContactPropertyTmpl propContactWebSiteBusiness;
	const Kopete::ContactPropertyTmpl propContactWebSitePersonal;

	const Kopete::ContactPropertyTmpl propPhoneHome;
	const Kopete::ContactPropertyTmpl propPhoneWork;
	const Kopete::ContactPropertyTmpl propPhoneMobile;
	const Kopete::ContactPropertyTmpl propClient;
	const Kopete::ContactPropertyTmpl propGuid;
	const Kopete::ContactPropertyTmpl propPersonalMessage; // it's the equivalent of away message.

	// Enums used to build the Kopete's Messenger ClientId.
	enum MessengerClientInformationFields
	{
		WindowsMobile = 0x1,
		InkFormatGIF = 0x04,
		InkFormatISF = 0x08,
		SupportWebcam = 0x10,
		SupportMultiPacketMessaging = 0x20,
		MessengerMobileDevice = 0x40,
		MessengerDirectDevice = 0x80,
		WebMessenger = 0x100,
		SupportDirectIM =  0x4000,
		SupportWinks = 0x8000,
		MessengerC1 = 0x10000000,
		MessengerC2 = 0x20000000,
		MessengerC3 = 0x30000000,
		MessengerC4 = 0x40000000
	};


private:
	static MessengerProtocol *s_self;
	QMap<int, QString> mCountries;
	QMap<int, QString> mMonths;
	QMap<int, QString> mDays;

private:
	void initCountries();
	void initMonths();
	void initDays();

};

#endif
