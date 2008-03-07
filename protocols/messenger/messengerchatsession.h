/*
    messengerchatsession.h - MSN Message Manager

    Copyright (c) 2007		by Zhang Panyong        <pyzhang@gmail.com>
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

#include <QLabel>
#include <QList>

#include "messengerprotocol.h"
#include <kopetecontact.h>
class QString;
class MessengerContact;
class KActionMenu;
class QLabel;
class KAction;



/**
 * @author Zhang Panyong
 */
class MessengerChatSession : public Kopete::ChatSession
{
	Q_OBJECT
public:
	MessengerChatSession ( MessengerProtocol *protocol, const Kopete::Contact *user,
						   Kopete::ContactPtrList others, const QString &resource = "" );

	~MessengerChatSession();

	virtual void inviteContact(const QString& );
private:

	QString otherString;
	KActionMenu *m_actionInvite;
	QList<KAction*> m_inviteactions;
	KAction *m_actionNudge;
	KAction *m_actionWebcamReceive;
	KAction *m_actionWebcamSend;
	KAction *m_actionSendFile;

public slots:
	void slotActionInviteAboutToShow();
	void slotDebugRawCommand();
	void slotSendNudge();
	void slotWebcamReceive();
	void slotWebcamSend();
	void slotRequestPicture();
};

#endif/* MESSENGERCHATSESSION_H*/
// vim: set noet ts=4 sts=4 tw=4:
