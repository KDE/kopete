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

#include <QtCore/QBitArray>
#include <QtNetwork/QHostAddress>
#include "oscartypes.h"
#include "kopete_export.h"

class Buffer;
using namespace Oscar;

/**
 * Holds information from the extended user info packet
 * @author Matt Rogers
 */
class LIBOSCAR_EXPORT UserDetails
{
public:
	UserDetails();
	~UserDetails();

	void clear();
	
	QString userId() const; //! User ID accessor
	int warningLevel() const; //! Warning level accessor
	Oscar::WORD idleTime() const; //! Idle time accessor
	QHostAddress dcInternalIp() const; //! DC local IP accessor
	QHostAddress dcExternalIp() const; //! DC outside IP accessor
	Oscar::DWORD dcPort() const; //! DC port number    
    Oscar::WORD dcProtoVersion() const;
	QDateTime onlineSinceTime() const; //! Online since accessor
	QDateTime awaySinceTime() const; //! Away since accessor
	QDateTime memberSinceTime() const; //! Member since accessor
	int userClass() const; //! User class accessor
	Oscar::DWORD extendedStatus() const; //!User status accessor
	int xtrazStatus() const;
	int statusMood() const;
	Oscar::WORD iconType() const; //!Buddy icon type
	Oscar::BYTE iconCheckSumType() const; //!Buddy icon hash type
	QByteArray buddyIconHash() const; //! Buddy icon md5 hash accessor
	QString clientName() const; //! Client name and version
	bool hasCap( int capNumber ) const; //! Tell if we have this capability
	bool onlineStatusMsgSupport() const; //! Client supports online status messages
	QString personalMessage() const; //! User's status (away or available) message

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
	bool awaySinceSpecified() const { return m_awaySinceSpecified; }
	bool numSecondsOnlineSpecified() const { return m_numSecondsOnlineSpecified; }
	bool idleTimeSpecified() const { return m_idleTimeSpecified; }
	bool extendedStatusSpecified() const { return m_extendedStatusSpecified; }
	bool xtrazStatusSpecified() const { return m_xtrazStatusSpecified; }
	bool statusMoodSpecified() const { return m_statusMoodSpecified; }
	bool capabilitiesSpecified() const { return m_capabilitiesSpecified; }
	bool dcOutsideSpecified() const { return m_dcOutsideSpecified; }
	bool dcInsideSpecified() const { return m_dcInsideSpecified; }
	bool iconSpecified() const { return m_iconSpecified; }

private:
    /**
     * Parse the character array for validness and a version string
    * \param buffer the buffer we'll be parsing for capabilities
    * \param versionString a QString reference that will contain the
    * version string of the detected client. Will be QString() if 
    * no client is found
     */
    void parseCapabilities(Buffer &inbuf, int &xStatus);

    /**
    * Parse the character array for capabilities (TLV 0x19)
    * \param inbuf the buffer we'll be parsing for capabilities
    */
    void parseNewCapabilities(Buffer &inbuf);

    //! Do client detection 
    void detectClient();


private:
	QString m_userId; /// the screename/uin of the contact
	int m_warningLevel; /// the warning level of the contact
	int m_userClass; /// the class of the user - TLV 0x01
	QDateTime m_memberSince; /// how long the user's been a member - TLV 0x05
	QDateTime m_onlineSince; /// how long the contact's been online - TLV 0x03
	QDateTime m_awaySince; /// how long the contact's been away - TLV 0x29
	Oscar::DWORD m_numSecondsOnline; /// how long the contact's been online in seconds
	Oscar::WORD m_idleTime; /// the idle time of the contact - TLV 0x0F
	Oscar::DWORD m_extendedStatus; /// the extended status of the contact - TLV 0x06
	int m_xtrazStatus;
	int m_statusMood;
	QBitArray m_capabilities; //TLV 0x05
	QString m_clientVersion; /// the version of client they're using
	QString m_clientName; /// the name of the client they're using
	QHostAddress m_dcOutsideIp; /// DC Real IP Address - TLV 0x0A
	QHostAddress m_dcInsideIp; /// DC Internal IP Address - TLV 0x0C
	Oscar::DWORD m_dcPort; /// DC Port - TLV 0x0C
	Oscar::BYTE m_dcType; /// DC Type - TLV 0x0C
	Oscar::WORD m_dcProtoVersion; /// DC Protocol Version - TLV 0x0C
	Oscar::DWORD m_dcAuthCookie; /// DC Authorization Cookie - TLV 0x0C
	Oscar::DWORD m_dcWebFrontPort; /// DC Web Front Port - TLV 0x0C
	Oscar::DWORD m_dcClientFeatures; /// DC client features( whatever they are ) - TLV 0x0C
	Oscar::DWORD m_dcLastInfoUpdateTime; /// DC last info update time - TLV 0x0C
	Oscar::DWORD m_dcLastExtInfoUpdateTime; /// DC last exteneded info update time - TLV 0x0C
	Oscar::DWORD m_dcLastExtStatusUpdateTime; /// DC last extended status update time - TLV 0x0C
	Oscar::WORD m_iconType; /// The OSCAR icon type for the buddy icon TLV 0x1D
	Oscar::BYTE m_iconChecksumType; /// The OSCAR checksum type for the buddy icon TLV 0x1D
	QByteArray m_md5IconHash; /// Buddy Icon MD5 Hash - TLV 0x1D
	QString m_personalMessage; /// User's away (or available) status message - TLV 0x0D
	bool m_onlineStatusMsgSupport; /// User's client supports online status messages - TLV 0x08
	Guid m_identCap; /// Save guid for client identification

	bool m_userClassSpecified;
	bool m_memberSinceSpecified;
	bool m_onlineSinceSpecified;
	bool m_awaySinceSpecified;
	bool m_numSecondsOnlineSpecified;
	bool m_idleTimeSpecified;
	bool m_extendedStatusSpecified;
	bool m_xtrazStatusSpecified;
	bool m_statusMoodSpecified;
	bool m_capabilitiesSpecified;
	bool m_dcOutsideSpecified;
	bool m_dcInsideSpecified;
	bool m_iconSpecified;

};

#endif 
//kate: tab-width 4; indent-mode csands;
