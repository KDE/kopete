/*
	Kopete Oscar Protocol
	icquserinfo.h - ICQ User Info Data Types
	
	Copyright (c) 2004 Matt Rogers <mattr@kde.org>
	Copyright (c) 2006 Roman Jarosz <kedgedev@centrum.cz>
	
	Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
	
	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/

#ifndef _ICQUSERINFO_H_
#define _ICQUSERINFO_H_

#include <QByteArray>
#include <QList>

#include "liboscar_export.h"
#include "oscartypes.h"
#include "icqinfovalue.h"

class Buffer;

/**
 * @file icquserinfo.h
 * Classes encapsulating user data retrieved from the server
 */

class LIBOSCAR_EXPORT ICQInfoBase
{
public:
	
	ICQInfoBase() : m_sequence( 0 ) {}
	virtual ~ICQInfoBase() {}
	virtual void fill( Buffer* buffer ) = 0;
	virtual void store( Buffer* ) {}
	
	void setSequenceNumber( Oscar::DWORD number ) { m_sequence = number; }
	Oscar::DWORD sequenceNumber() { return m_sequence; }

private:
	Oscar::DWORD m_sequence;
};


class LIBOSCAR_EXPORT ICQShortInfo : public ICQInfoBase
{
public:
	ICQShortInfo();
	~ICQShortInfo() {}
	void fill( Buffer* buffer );
	void store( Buffer* buffer );
	
public:
	unsigned long uin;
	QByteArray nickname;
	QByteArray firstName;
	QByteArray lastName;
	QByteArray email;
	ICQInfoValue<bool> needsAuth;
	ICQInfoValue<bool> webAware; // 0=offline, 1=online, 2=not webaware
// 	unsigned int gender;
};

class LIBOSCAR_EXPORT ICQGeneralUserInfo : public ICQInfoBase
{
public:
	ICQGeneralUserInfo();
	~ICQGeneralUserInfo() {}
	void fill( Buffer* buffer );
	void store( Buffer* buffer );
	
public:
	ICQInfoValue<unsigned long> uin;
	ICQInfoValue<QByteArray> nickName;
	ICQInfoValue<QByteArray> firstName;
	ICQInfoValue<QByteArray> lastName;
	ICQInfoValue<QByteArray> email;
	ICQInfoValue<QByteArray> city;
	ICQInfoValue<QByteArray> state;
	ICQInfoValue<QByteArray> phoneNumber;
	ICQInfoValue<QByteArray> faxNumber;
	ICQInfoValue<QByteArray> address;
	ICQInfoValue<QByteArray> cellNumber;
	ICQInfoValue<QByteArray> zip;
	ICQInfoValue<int> country;
	ICQInfoValue<char> timezone;
	ICQInfoValue<bool> publishEmail;
	ICQInfoValue<bool> allowsDC;
	ICQInfoValue<bool> webAware;
	ICQInfoValue<bool> needsAuth;
};

class LIBOSCAR_EXPORT ICQWorkUserInfo : public ICQInfoBase
{
public:
	ICQWorkUserInfo();
	~ICQWorkUserInfo() {}
	void fill( Buffer* buffer );
	void store( Buffer* buffer );
	
public:
	ICQInfoValue<QByteArray> city;
	ICQInfoValue<QByteArray> state;
	ICQInfoValue<QByteArray> phone;
	ICQInfoValue<QByteArray> fax;
	ICQInfoValue<QByteArray> address;
	ICQInfoValue<QByteArray> zip;
	ICQInfoValue<int> country;
	ICQInfoValue<QByteArray> company;
	ICQInfoValue<QByteArray> department;
	ICQInfoValue<QByteArray> position;
	ICQInfoValue<int> occupation;
	ICQInfoValue<QByteArray> homepage;
};

class LIBOSCAR_EXPORT ICQMoreUserInfo : public ICQInfoBase
{
public:
	ICQMoreUserInfo();
	~ICQMoreUserInfo() {}
	void fill( Buffer* buffer );
	void store( Buffer* buffer );
	
public:
	ICQInfoValue<int> age;
	ICQInfoValue<unsigned int> gender;
	ICQInfoValue<QByteArray> homepage;
	ICQInfoValue<int> birthdayDay;
	ICQInfoValue<int> birthdayMonth;
	ICQInfoValue<int> birthdayYear;
	ICQInfoValue<unsigned int> lang1;
	ICQInfoValue<unsigned int> lang2;
	ICQInfoValue<unsigned int> lang3;
	ICQInfoValue<QByteArray> ocity;
	ICQInfoValue<QByteArray> ostate;
	ICQInfoValue<int> ocountry;
	ICQInfoValue<int> marital;
	ICQInfoValue<bool> sendInfo;
};

class LIBOSCAR_EXPORT ICQEmailInfo : public ICQInfoBase
{
public:
	ICQEmailInfo();
	~ICQEmailInfo() {}
	void fill( Buffer* buffer );
	void store( Buffer* buffer );

public:
	class EmailItem
	{
	public:
		bool publish;
		QByteArray email;
		bool operator==( const EmailItem& item ) const
		{
			return ( publish == item.publish && email == item.email );
		}
	};

	ICQInfoValue< QList<EmailItem> > emailList;
};

class LIBOSCAR_EXPORT ICQNotesInfo : public ICQInfoBase
{
public:
	ICQNotesInfo();
	~ICQNotesInfo() {}
	void fill( Buffer* buffer );
	void store( Buffer* buffer );
	
public:
	ICQInfoValue<QByteArray> notes;
};

class LIBOSCAR_EXPORT ICQInterestInfo : public ICQInfoBase
{
public:
	ICQInterestInfo();
	~ICQInterestInfo() {}
	void fill( Buffer* buffer );
	void store( Buffer* buffer );
	
public:
	ICQInfoValue<int> topics[4];
	ICQInfoValue<QByteArray> descriptions[4];
};

class LIBOSCAR_EXPORT ICQOrgAffInfo : public ICQInfoBase
{
public:
	ICQOrgAffInfo();
	~ICQOrgAffInfo() {}
	void fill( Buffer* buffer );
	void store( Buffer* buffer );
	
public:
	ICQInfoValue<int> org1Category;
	ICQInfoValue<int> org2Category;
	ICQInfoValue<int> org3Category;
	ICQInfoValue<QByteArray> org1Keyword;
	ICQInfoValue<QByteArray> org2Keyword;
	ICQInfoValue<QByteArray> org3Keyword;
	ICQInfoValue<int> pastAff1Category;
	ICQInfoValue<int> pastAff2Category;
	ICQInfoValue<int> pastAff3Category;
	ICQInfoValue<QByteArray> pastAff1Keyword;
	ICQInfoValue<QByteArray> pastAff2Keyword;
	ICQInfoValue<QByteArray> pastAff3Keyword;
};

class LIBOSCAR_EXPORT ICQSearchResult
{
public:
	ICQSearchResult();
	void fill( Buffer* buffer );
	quint32 uin;
	QByteArray firstName;
	QByteArray lastName;
	QByteArray nickName;
	QByteArray email;
	bool auth;
	bool online;
	char gender;
	quint16 age;
};

class LIBOSCAR_EXPORT ICQWPSearchInfo
{
public:
	ICQWPSearchInfo();
	
	QByteArray firstName;
	QByteArray lastName;
	QByteArray nickName;
	QByteArray email;
	int age;
	int gender;
	int language;
	QByteArray city;
	QByteArray state;
	int country;
	QByteArray company;
	QByteArray department;
	QByteArray position;
	int occupation;
	bool onlineOnly;
};

class LIBOSCAR_EXPORT ICQFullInfo : public ICQInfoBase
{
public:
	/**
	 * ICQFullInfo constructor
	 * @param assumeDirty if false only values that where explicitly set with set method will be stored.
	 */
	ICQFullInfo( bool assumeDirty = true );
	~ICQFullInfo() {}

	void fill( Buffer* buffer );
	void store( Buffer* buffer );

public:
	class AddressItem
	{
	public:
		AddressItem() : country(0) {}

		QByteArray	address;
		QByteArray	city;
		QByteArray	state;
		QByteArray	zip;
		quint32		country;
	};
	typedef QList<AddressItem> AddressItemList;

	class WorkItem
	{
	public:
		WorkItem() : country(0) {}

		QByteArray position;
		QByteArray companyName;
		QByteArray department;
		QByteArray homepage;
		QByteArray address;
		QByteArray city;
		QByteArray state;
		QByteArray zip;
		quint32 country;
	};
	typedef QList<WorkItem> WorkItemList;

	class InfoItem {
	public:
		InfoItem() : category(0) {}

		quint16 category;
		QByteArray description;
	};
	typedef QList<InfoItem> InfoItemList;

	ICQInfoValue<QByteArray>		uin;
	ICQInfoValue<QByteArray>		firstName;
	ICQInfoValue<QByteArray>		lastName;
	ICQInfoValue<QByteArray>		nickName;
	ICQInfoValue<QByteArray>		homepage;
	ICQInfoValue<char>				gender;				//0x00 - None, 0x01 - Female, 0x02 - Male
	ICQInfoValue<bool>				webAware;
	ICQInfoValue<quint16>			privacyProfile;		//0x00 - Low, 0x01 - Medium, 0x02 - High

	ICQInfoValue<quint16>			language1;
	ICQInfoValue<quint16>			language2;
	ICQInfoValue<quint16>			language3;

	ICQInfoValue<QByteArray>		statusDescription;

	ICQInfoValue<quint16>			timezone;
	ICQInfoValue<QByteArray>		notes;

	ICQInfoValue<AddressItemList>	homeList;
	ICQInfoValue<AddressItemList>	originList;
	ICQInfoValue<WorkItemList>		workList;

	ICQInfoValue<InfoItemList>		interestList;
	ICQInfoValue<InfoItemList>		organizationList;
	ICQInfoValue<InfoItemList>		pastAffliationList;
	ICQInfoValue<InfoItemList>		phoneList;

private:
	AddressItemList parseAddressItemList( const QByteArray& data ) const;
	QByteArray storeAddressItemList( const AddressItemList& infoList ) const;
	WorkItemList parseWorkItemList( const QByteArray& data ) const;
	QByteArray storeWorkItemList( const WorkItemList& infoList ) const;
	InfoItemList parseInfoItemList( const QByteArray& data ) const;
	QByteArray storeInfoItemList( const InfoItemList& infoList ) const;
};

#endif
//kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
