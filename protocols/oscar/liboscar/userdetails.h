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

#include "oscartypes.h"
#include <kopete_export.h>

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
	DWORD dcInternalIp() const; //! DC local IP accessor
	DWORD dcExternalIp() const; //! DC outside IP accessor
	DWORD dcPort() const; //! DC port number
	QDateTime onlineSinceTime() const; //! Online since accessor
	QDateTime memberSinceTime() const; //! Member since accessor
	int userClass() const; //! User class accessor
	DWORD extendedStatus() const;
	
	/** 
	 * Fill the class with data from a buffer
	 * It only updates what's available.
	 */
	void fill( Buffer* buffer );
private:
	//! Do client detection 
	void detectClient();
	
	//! Tell if we have this capability
	bool hasCap( int capNumber );
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
	DWORD m_dcOutsideIp; /// DC Real IP Address - TLV 0x0A
	DWORD m_dcInsideIp; /// DC Internal IP Address - TLV 0x0C
	DWORD m_dcPort; /// DC Port - TLV 0x0C
	BYTE m_dcType; /// DC Type - TLV 0x0C
	WORD m_dcProtoVersion; /// DC Protocol Version - TLV 0x0C
	DWORD m_dcAuthCookie; /// DC Authorization Cookie - TLV 0x0C
	DWORD m_dcWebFrontPort; /// DC Web Front Port - TLV 0x0C
	DWORD m_dcClientFeatures; /// DC client features( whatever they are ) - TLV 0x0C
	DWORD m_dcLastInfoUpdateTime; /// DC last info update time - TLV 0x0C
	DWORD m_dcLastExtInfoUpdateTime; /// DC last exteneded info update time - TLV 0x0C
	DWORD m_dcLastExtStatusUpdateTime; /// DC last extended status update time - TLV 0x0C
	
};

#endif 
//kate: tab-width 4; indent-mode csands;
