/*
	Kopete Oscar Protocol
	icquserinfo.h - ICQ User Info Data Types
	
	Copyright (c) 2004 Matt Rogers <mattr@kde.org>
	
	Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
	
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

#include <qcstring.h>
#include <qvaluelist.h>
#include <qdatetime.h>
#include "kopete_export.h"

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
	QCString nickname;
	QCString firstName;
	QCString lastName;
	QCString email;
	bool needsAuth;
	unsigned int gender; // 0=offline, 1=online, 2=not webaware
};

class KOPETE_EXPORT ICQGeneralUserInfo : public ICQInfoBase
{
public:
	ICQGeneralUserInfo();
	~ICQGeneralUserInfo() {}
	void fill( Buffer* buffer );
	
public:
	unsigned long uin;
	QCString nickname;
	QCString firstName;
	QCString lastName;
	QCString email;
	QCString city;
	QCString state;
	QCString phoneNumber;
	QCString faxNumber;
	QCString address;
	QCString cellNumber;
	QCString zip;
	int country;
	char timezone;
	bool publishEmail;
	bool allowsDC;
	bool webaware;
};

class KOPETE_EXPORT ICQWorkUserInfo : public ICQInfoBase
{
public:
	ICQWorkUserInfo();
	~ICQWorkUserInfo() {}
	void fill( Buffer* buffer );
	
public:
	QCString city;
	QCString state;
	QCString phone;
	QCString fax;
	QCString address;
	QCString zip;
	int country;
	QCString company;
	QCString department;
	QCString position;
	int occupation;
	QCString homepage;
};

class KOPETE_EXPORT ICQMoreUserInfo : public ICQInfoBase
{
public:
	ICQMoreUserInfo();
	~ICQMoreUserInfo() {}
	void fill( Buffer* buffer );
	
public:
	int age;
	unsigned int gender;
	QCString homepage;
	QDate birthday;
	unsigned int lang1;
	unsigned int lang2;
	unsigned int lang3;
	QCString ocity;
	QCString ostate;
	int ocountry;
	int marital;
};

class KOPETE_EXPORT ICQEmailInfo : public ICQInfoBase
{
public:
	ICQEmailInfo();
	~ICQEmailInfo() {}
	void fill( Buffer* buffer );
	
public:
	QValueList<QCString> emailList;
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
	QCString descriptions[4];
};


class KOPETE_EXPORT ICQSearchResult
{
public:
	ICQSearchResult();
	void fill( Buffer* buffer );
	Q_UINT32 uin;
	QCString firstName;
	QCString lastName;
	QCString nickName;
	QCString email;
	bool auth;
	bool online;
	char gender;
	Q_UINT16 age;
};

class KOPETE_EXPORT ICQWPSearchInfo
{
public:
	ICQWPSearchInfo();
	
	QCString firstName;
	QCString lastName;
	QCString nickName;
	QCString email;
	int age;
	int gender;
	int language;
	QCString city;
	QCString state;
	int country;
	QCString company;
	QCString department;
	QCString position;
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
