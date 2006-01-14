/*
	Kopete Oscar Protocol
	userdetails.h - user details from the extended status packet
	
	Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
	
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
#ifndef USERDETAILS_H
#define USERDETAILS_H

#include <ksocketaddress.h>
#include "oscartypes.h"
#include "kopete_export.h"

class Buffer;
using namespace Oscar;

/**
 * Holds information from the extended user info packet
 * @author Matt Rogers
 */
class KOPETE_EXPORT UserDetails
{
public:
	UserDetails();
	~UserDetails();
	
	QString userId() const; //! User ID accessor
	int warningLevel() const; //! Warning level accessor
	WORD idleTime() const; //! Idle time accessor
	KNetwork::KIpAddress dcInternalIp() const; //! DC local IP accessor
	KNetwork::KIpAddress dcExternalIp() const; //! DC outside IP accessor
	DWORD dcPort() const; //! DC port number
	QDateTime onlineSinceTime() const; //! Online since accessor
	QDateTime memberSinceTime() const; //! Member since accessor
	int userClass() const; //! User class accessor
	DWORD extendedStatus() const; //!User status accessor
	BYTE iconCheckSumType() const; //!Buddy icon hash type
	QByteArray buddyIconHash() const; //! Buddy icon md5 hash accessor
	QString clientName() const; //! Client name and version
	bool hasCap( int capNumber ) const; //! Tell if we have this capability
	
	/** 
	 * Fill the class with data from a buffer
	 * It only updates what's available.
	 */
	void fill( Buffer* buffer );

	/**
	 * Merge only those data from another UserDetails
	 * which are marked as specified.
	 */
	void merge( const UserDetails& ud );

	bool userClassSpecified() const { return m_userClassSpecified; }
	bool memberSinceSpecified() const { return m_memberSinceSpecified; }
	bool onlineSinceSpecified() const { return m_onlineSinceSpecified; }
	bool numSecondsOnlineSpecified() const { return m_numSecondsOnlineSpecified; }
	bool idleTimeSpecified() const { return m_idleTimeSpecified; }
	bool extendedStatusSpecified() const { return m_extendedStatusSpecified; }
	bool capabilitiesSpecified() const { return m_capabilitiesSpecified; }
	bool dcOutsideSpecified() const { return m_dcOutsideSpecified; }
	bool dcInsideSpecified() const { return m_dcInsideSpecified; }
	bool iconSpecified() const { return m_iconSpecified; }
private:
	//! Do client detection 
	void detectClient();


private:
	QString m_userId; /// the screename/uin of the contact
	int m_warningLevel; /// the warning level of the contact
	int m_userClass; /// the class of the user - TLV 0x01
	QDateTime m_memberSince; /// how long the user's been a member - TLV 0x05
	QDateTime m_onlineSince; /// how long the contact's been online - TLV 0x03
	DWORD m_numSecondsOnline; /// how long the contact's been online in seconds
	WORD m_idleTime; /// the idle time of the contact - TLV 0x0F
	DWORD m_extendedStatus; /// the extended status of the contact - TLV 0x06	
	DWORD m_capabilities; //TLV 0x05
	QString m_clientVersion; /// the version of client they're using
	QString m_clientName; /// the name of the client they're using
	KNetwork::KIpAddress m_dcOutsideIp; /// DC Real IP Address - TLV 0x0A
	KNetwork::KIpAddress m_dcInsideIp; /// DC Internal IP Address - TLV 0x0C
	DWORD m_dcPort; /// DC Port - TLV 0x0C
	BYTE m_dcType; /// DC Type - TLV 0x0C
	WORD m_dcProtoVersion; /// DC Protocol Version - TLV 0x0C
	DWORD m_dcAuthCookie; /// DC Authorization Cookie - TLV 0x0C
	DWORD m_dcWebFrontPort; /// DC Web Front Port - TLV 0x0C
	DWORD m_dcClientFeatures; /// DC client features( whatever they are ) - TLV 0x0C
	DWORD m_dcLastInfoUpdateTime; /// DC last info update time - TLV 0x0C
	DWORD m_dcLastExtInfoUpdateTime; /// DC last exteneded info update time - TLV 0x0C
	DWORD m_dcLastExtStatusUpdateTime; /// DC last extended status update time - TLV 0x0C
	BYTE m_iconChecksumType; /// The OSCAR checksum type for the buddy icon TLV 0x1D
	QByteArray m_md5IconHash; /// Buddy Icon MD5 Hash - TLV 0x1D
	QString m_availableMessage; /// Message a person can have when available - TLV 0x0D
	
	bool m_userClassSpecified;
	bool m_memberSinceSpecified;
	bool m_onlineSinceSpecified;
	bool m_numSecondsOnlineSpecified;
	bool m_idleTimeSpecified;
	bool m_extendedStatusSpecified;
	bool m_capabilitiesSpecified;
	bool m_dcOutsideSpecified;
	bool m_dcInsideSpecified;
	bool m_iconSpecified;
	
};

#endif 
//kate: tab-width 4; indent-mode csands;
