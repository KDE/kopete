/*
   papillon_enums.h - Definition of enums 

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#ifndef PAPILLON_ENUMS_H
#define PAPILLON_ENUMS_H

#include <QtCore/QFlags>

namespace Papillon
{
	/**
	 * @struct ContactListEnums papillon_enums.h <Papillon/Enums>
	 * @brief Enums related to Contact list
	 */
	struct ContactListEnums
	{
		/**
		 * @brief Flags for the different contact list type.
		 */
		enum Type
		{
			ForwardList = 1, ///<Contacts that you want to see their presence.
			AllowList = 2, ///<Contacts allowed to see your presence.
			BlockList = 4, ///<Contacts not allowed to see your presence.
			ReverseList = 8, ///<Contacts that have you on their Forward List.
			PendingList = 16 ///<Contacts pending to be added.
		};
		Q_DECLARE_FLAGS(ListFlags, Type)
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS( ContactListEnums::ListFlags )

	/**
	 * @struct ClientInfo papillon_enums.h <Papillon/Enums>
	 * @brief Enums related to ClientInfo
	 */
	struct ClientInfo
	{
		/**
		 * @brief Enum for features supported by the client.
		*/
		enum Feature
		{
			FeatureNone = 0x0, ///<Internal default
			WindowsMobile = 0x1,  ///<This is a Windows Mobile client.
			MsnMobileDevice = 0x40, ///<Client is running on a MSN Mobile device.
			MsnDirectDevice = 0x80, ///<Client is running on a MSN Direct device.
			WebMessenger = 0x100, ///<Client is using MSN Web Messenger.
			InkFormatGif = 0x04, ///<Support Ink messages in GIF format.
			InkFormatIsf = 0x08, ///<Support Ink messages in ISF format.
			SupportWebcam = 0x10, ///<Support Webcam. 
			SupportMultiPacketMessaging = 0x20, ///<Support messages over multiple packets.
			SupportDirectIM =  0x4000, ///<Support direct P2P connections instead of using Switchboard.
			SupportVoiceClips = 0x40000, ///<Support Voice clips (introduced in MSN Messenger 7.5)
			SupportWinks = 0x8000, ///<Client can receive Winks.
			SupportMsnSearch = 0x10000, ///<Client supports MSN Search.
			MSNC1 = 0x10000000, ///<MSNC(Mobile Status Notification Client) Version 1 (with MSN Messenger 6.0)
			MSNC2 = 0x20000000, ///<MSNC Version 2 (with MSN Messenger 6.1)
			MSNC3 = 0x30000000, ///<MSNC Version 3 (with MSN Messenger 6.2)
			MSNC4 = 0x40000000, ///<MSNC Version 4 (with MSN Messenger 7.0)
			MSNC5 = 0x50000000, ///<MSNC Version 5 (with MSN Messenger 7.5)
			MSNC6 = 0x60000000 ///<MSNC Version 6 (with Windows Live Messenger 8)
		};
		Q_DECLARE_FLAGS(Features, Feature)

		/**
		 * @brief This enum is used to determine the personal information changed on server.
		 */
		enum PersonalInformation
		{
			PersonalInfoNone = 0, ///<Internal default
			Nickname, ///<Setting the nickname
			PhoneHome, ///<Setting the home phone number
			PhoneWork, ///<Setting the work phone number
			PhoneMobile, ///<Setting the mobile phone number
			MobileAuthorization, ///<are other people authorised to contact me on my MSN Mobile (Y or N)
			MobileDeviceEnabled ///<do I have a mobile device enabled on MSN Mobile (Y or N)
		};
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS( Papillon::ClientInfo::Features )

	/**
	 * @struct Presence papillon_enums.h <Papillon/Enums>
	 * @brief Enums related to online presence.
	 */
	struct Presence
	{
		/**
		 * Types of online presense.
	 	 */
		enum Status
		{
			Away, ///<Correspond to AWY
			BeRightBack, ///<Correspond to BRB
			Busy, ///<Correspond to BSY
			Idle, ///<Correspond to IDL
			Invisible, ///<Correspond to HDN
			Offline, ///<Correspond to FLN
			Online, ///<Correspond to NLN
			OnThePhone, ///<Correspond to PHN
			OutToLunch ///<Correspond to LUN
		};

		/**
		 * Types of media application
		 */
		enum MediaType
		{
			MediaNone = 0, ///<Internal default
			MediaMusic, ///<Current music played.
			MediaGames, ///<Current game played.
			MediaOffice ///<Current office task done.
		};
	};
}

#endif
