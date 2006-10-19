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

#include "kopete_export.h"
#include "icqinfovalue.h"

class Buffer;

/**
 * @file icquserinfo.h
 * Classes encapsulating user data retrieved from the server
 */

class KOPETE_EXPORT ICQInfoBase
{
public:
	
	ICQInfoBase() : m_sequence( 0 ) {}
	virtual ~ICQInfoBase() {}
	virtual void fill( Buffer* buffer ) = 0;
	virtual void store( Buffer* ) {}
	
	void setSequenceNumber( int number ) { m_sequence = number; }
	int sequenceNumber() { return m_sequence; }

private:
	int m_sequence;
};


class KOPETE_EXPORT ICQShortInfo : public ICQInfoBase
{
public:
	ICQShortInfo();
	~ICQShortInfo() {}
	void fill( Buffer* buffer );
	
public:
	unsigned long uin;
	QByteArray nickname;
	QByteArray firstName;
	QByteArray lastName;
	QByteArray email;
	bool needsAuth;
	unsigned int gender; // 0=offline, 1=online, 2=not webaware
};

class KOPETE_EXPORT ICQGeneralUserInfo : public ICQInfoBase
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
	ICQInfoValue<bool> authorization;
};

class KOPETE_EXPORT ICQWorkUserInfo : public ICQInfoBase
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

class KOPETE_EXPORT ICQMoreUserInfo : public ICQInfoBase
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

class KOPETE_EXPORT ICQEmailInfo : public ICQInfoBase
{
public:
	ICQEmailInfo();
	~ICQEmailInfo() {}
	void fill( Buffer* buffer );
	
public:
	QList<QByteArray> emailList;
};

class KOPETE_EXPORT ICQNotesInfo : public ICQInfoBase
{
public:
	ICQNotesInfo();
	~ICQNotesInfo() {}
	void fill( Buffer* buffer );
	
public:
	QByteArray notes;
};

class KOPETE_EXPORT ICQInterestInfo : public ICQInfoBase
{
public:
	ICQInterestInfo();
	~ICQInterestInfo() {}
	void fill( Buffer* buffer );
	
public:
	int count;
	int topics[4];
	QByteArray descriptions[4];
};

class KOPETE_EXPORT ICQOrgAffInfo : public ICQInfoBase
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

class KOPETE_EXPORT ICQSearchResult
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

class KOPETE_EXPORT ICQWPSearchInfo
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

/*
class ICQInfoItem
{
public:
	int category;
	QCString description;
};


typedef QValueList<ICQInfoItem> ICQInfoItemList;
*/

#endif
//kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
