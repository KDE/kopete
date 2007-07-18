/*
    messengerchatsession.h - MSN Message Manager

    Copyright (c) 2007		by Zhang Panyong        <pyzhang8@gmail.com>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef MESSENGERCHATSESSION_H
#define MESSENGERCHATSESSION_H

/**
 * @author Zhang Panyong
 */
class MessengerChatSession : public Kopete::ChatSession
{
	Q_OBJECT
public:
	MessengerChatSession ( MessengerProtocol *protocol, const JabberBaseContact *user,
						   Kopete::ContactPtrList others, const QString &resource = "" );
	
	~MessengerChatSession();

	virtual void inviteContact(const QString& );
}

#endif/* MESSENGERCHATSESSION_H*/
// vim: set noet ts=4 sts=4 tw=4:
