/*
    qqprotocol.h - Kopete QQ Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef QQPROTOCOL_H
#define QQPROTOCOL_H

#include <kopeteprotocol.h>
#include <kopeteproperty.h>


/**
 * Encapsulates the generic actions associated with this protocol
 * @author Will Stephenson
 */
class QQProtocol : public Kopete::Protocol
{
	Q_OBJECT
public:
	QQProtocol(QObject *parent, const QVariantList &args);
    ~QQProtocol();

	/**
	 * The possible QQ online statuses
	 */
	const Kopete::OnlineStatus Online;  //online
	const Kopete::OnlineStatus BSY;  //busy
	const Kopete::OnlineStatus BRB;  //be right back
	const Kopete::OnlineStatus AWY;  //away
	const Kopete::OnlineStatus PHN;  //on the phone
	const Kopete::OnlineStatus LUN;  //out to lunch
	const Kopete::OnlineStatus Offline;  //offline
	const Kopete::OnlineStatus HDN;  //invisible
	const Kopete::OnlineStatus IDL;  //idle
	const Kopete::OnlineStatus UNK;  //unknown (internal)
	const Kopete::OnlineStatus CNT;  //connecting (internal)

	/**
	 * Convert the serialised data back into a QQContact and add this
	 * to its Kopete::MetaContact
	 */
	virtual Kopete::Contact *deserializeContact(
			Kopete::MetaContact *metaContact,
			const QMap< QString, QString > & serializedData,
			const QMap< QString, QString > & addressBookData
		);
	/**
	 * Generate the widget needed to add QQContacts
	 */
	virtual AddContactPage * createAddContactWidget( QWidget *parent, Kopete::Account *account );
	/**
	 * Generate the widget needed to add/edit accounts for this protocol
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( Kopete::Account *account, QWidget *parent );
	/**
	 * Generate a QQAccount
	 */
	virtual Kopete::Account * createNewAccount( const QString &accountId );
	/**
	 * Access the instance of this protocol
	 */
	static QQProtocol *protocol();
	/** 
	 * Validate whether userId is a legal QQ account
	 */
	static bool validContactId( const QString& userId );
	/**
	 * Represents contacts that are Online
	 */
	const Kopete::OnlineStatus qqOnline;
	/**
	 * Represents contacts that are Away
	 */
	const Kopete::OnlineStatus qqAway;
	/**
	 * Represents contacts that are Offline
	 */
	const Kopete::OnlineStatus qqOffline;

	const Kopete::PropertyTmpl propNickName;
	const Kopete::PropertyTmpl propFullName;
	const Kopete::PropertyTmpl propCountry;
	const Kopete::PropertyTmpl propState;
	const Kopete::PropertyTmpl propCity;
	const Kopete::PropertyTmpl propStreet;
	const Kopete::PropertyTmpl propZipcode;
	const Kopete::PropertyTmpl propAge;
	const Kopete::PropertyTmpl propGender;
	const Kopete::PropertyTmpl propOccupation;
	const Kopete::PropertyTmpl propHomepage;
	const Kopete::PropertyTmpl propIntro;
	const Kopete::PropertyTmpl propGraduateFrom;
	const Kopete::PropertyTmpl propHoroscope;
	const Kopete::PropertyTmpl propZodiac;
	const Kopete::PropertyTmpl propBloodType;
	const Kopete::PropertyTmpl propEmail;


protected:
	static QQProtocol *s_protocol;
};

#endif // QQPROTOCOL_H
