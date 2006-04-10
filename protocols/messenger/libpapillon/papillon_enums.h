/*
   papillon_enums.h - Definition of enums 

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

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
	 * @brief Enum for Contact List flags.
	 * ForwardList: Contacts that you want to see their presence.
	 * AllowList: Contacts allowed to see your presence.
	 * BlockList: Contacts not allowed to see your presence.
	 * ReverseList: Contacts that have you on their Forward List.
	 * PendingList: Contacts pending to be added.
	 */
	struct ContactList
	{
		enum ContactListType
		{
			ForwardList = 1,
			AllowList = 2,
			BlockList = 4,
			ReverseList = 8,
			PendingList = 16
		};
		Q_DECLARE_FLAGS(ContactListFlags, ContactListType);
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS( ContactList::ContactListFlags )

	/**
	 * @brief Enum for features supported by the client.
	 * WindowsMobile: This is a Windows Mobile client.
	 * MsnMobileDevice: Client is running on a MSN Mobile device.
	 * MsnDirectDevice: Client is running on a MSN Direct device.
	 * WebMessenger: Client is using MSN Web Messenger.
	 * InkFormatGif: Support Ink messages in GIF format.
	 * InkFormatIsf: Support Ink messages in ISF format.
	 * SupportWebcam: Support Webcam.
	 * SupportMultiPacketMessaging: Support messages over multiple packets.
	 * SupportDirectIM: Support direct P2P connections instead of using Switchboard.
	 * SupportVoiceClips: Support Voice clips (introduced in MSN Messenger 7.5)
	 * SupportWinks: Client can receive Winks.
	 * SupportMsnSearch: Client supports MSN Search.
	 * MSNC1: MSNC(Mobile Status Notification Client) Version 1 (with MSN Messenger 6.0)
	 * MSNC2: MSNC Version 2 (with MSN Messenger 6.1)
	 * MSNC3: MSNC Version 3 (with MSN Messenger 6.2)
	 * MSNC4: MSNC Version 4 (with MSN Messenger 7.0)
	 * MSNC5: MSNC Version 5 (with MSN Messenger 7.5)
	 */
	struct ClientInfo
	{
		enum ClientFeature
		{
			WindowsMobile = 0x1,
			MsnMobileDevice = 0x40,
			MsnDirectDevice = 0x80,
			WebMessenger = 0x100,
			InkFormatGif = 0x04,
			InkFormatIsf = 0x08,
			SupportWebcam = 0x10,
			SupportMultiPacketMessaging = 0x20,
			SupportDirectIM =  0x4000,
			SupportVoiceClips = 0x40000,
			SupportWinks = 0x8000,
			SupportMsnSearch = 0x10000,
			MSNC1 = 0x10000000,
			MSNC2 = 0x20000000,
			MSNC3 = 0x30000000,
			MSNC4 = 0x40000000,
			MSNC5 = 0x50000000
		};
		Q_DECLARE_FLAGS(ClientFeatures, ClientFeature);
	};
	Q_DECLARE_OPERATORS_FOR_FLAGS( Papillon::ClientInfo::ClientFeatures )

}

#endif
