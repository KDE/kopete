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
#include <kopeteglobal.h>
#include <kopete_export.h>


#define MESSENGER_DEFAULT_PORT	1863
#define MESSENGER_DEFAULT_SERVER "messenger.hotmail.com"

namespace Kopete
{
	class Account;
	class MetaContact;
}

class QWidget;
class QComboBox;
class AddContactPage;
class KopeteEditAccountWidget;

/**
 * 
 * @author Michaël Larouche <larouche@kde.org>
 */
class MESSENGER_EXPORT MessengerProtocol : public Kopete::Protocol
{
public:
	MessengerProtocol(QObject *parent, const QVariantList &args);
	~MessengerProtocol();
	
	/**
	 * Creates the "add contact" dialog specific to this protocol
	 */
	virtual Kopete::Account *createNewAccount(const QString &accountId);
	virtual AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *account);
	virtual KopeteEditAccountWidget * createEditAccountWidget(Kopete::Account *account, QWidget *parent);

	/**
	 * Deserialize contact data
	 */
	virtual Kopete::Contact *deserializeContact( Kopete::MetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	const QMap<int, QString> &countries() { return mCountries; }
	const QMap<int, QString> &months() { return mMonths; }
	const QMap<int, QString> &days() { return mDays; }

	void fillComboFromTable(QComboBox *box, const QMap<int, QString> &map);
	void setComboFromTable(QComboBox *box, const QMap<int, QString> &map, int value);
	int getCodeForCombo(QComboBox *cmb, const QMap<int, QString> &map);

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

	const Kopete::PropertyTmpl propGuid;
	
	//general User info
	const Kopete::PropertyTmpl propEmail;
	const Kopete::PropertyTmpl propContactType;
	const Kopete::PropertyTmpl propFirstName;
	const Kopete::PropertyTmpl propLastName;
	const Kopete::PropertyTmpl propComment;
	const Kopete::PropertyTmpl propAnniversary;
	const Kopete::PropertyTmpl propBirthday;

	//Annotation
	const Kopete::PropertyTmpl propABJobTitle;
	const Kopete::PropertyTmpl propABNickName;
	const Kopete::PropertyTmpl propABJobSpouse;

	//Email
	const Kopete::PropertyTmpl propContactEmailBusiness;
	const Kopete::PropertyTmpl propContactEmailMessenger;
	const Kopete::PropertyTmpl propContactEmailOther;
	const Kopete::PropertyTmpl propContactEmailPersonal;

	//Phone
	const Kopete::PropertyTmpl propContactPhoneBusiness;
	const Kopete::PropertyTmpl propContactPhoneFax;
	const Kopete::PropertyTmpl propContactPhoneMobile;
	const Kopete::PropertyTmpl propContactPhoneOther;	
	const Kopete::PropertyTmpl propContactPhonePager;	
	const Kopete::PropertyTmpl propContactPhonePersonal;	

	//Business Location
	const Kopete::PropertyTmpl propBusinessName;
	const Kopete::PropertyTmpl propBusinessStreet;
	const Kopete::PropertyTmpl propBusinessCity;
	const Kopete::PropertyTmpl propBusinessState;
	const Kopete::PropertyTmpl propBusinessCountry;
	const Kopete::PropertyTmpl propBusinessPostalCode;

	//Personal Location
	const Kopete::PropertyTmpl propPersonalName;
	const Kopete::PropertyTmpl propPersonalStreet;
	const Kopete::PropertyTmpl propPersonalCity;
	const Kopete::PropertyTmpl propPersonalState;
	const Kopete::PropertyTmpl propPersonalCountry;
	const Kopete::PropertyTmpl propPersonalPostalCode;

	//Website
	const Kopete::PropertyTmpl propContactWebSiteBusiness;
	const Kopete::PropertyTmpl propContactWebSitePersonal;

	const Kopete::PropertyTmpl propPhoneHome;
	const Kopete::PropertyTmpl propPhoneWork;
	const Kopete::PropertyTmpl propPhoneMobile;
	const Kopete::PropertyTmpl propClient;
	const Kopete::PropertyTmpl propPersonalMessage; // it's the equivalent of away message.

	/**
	 * This returns our protocol instance
	 */
	static MessengerProtocol *protocol();


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



	static bool validContactId(const QString&);

private:
	static MessengerProtocol *protocolInstance;
	QMap<int, QString> mCountries;
	QMap<int, QString> mMonths;
	QMap<int, QString> mDays;

private:
	void initCountries();
	void initMonths();
	void initDays();
	QMap<QString, int> reverseMap( const QMap<int, QString>& ) const;

};

#endif
