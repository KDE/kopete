/***************************************************************************
                          jabbergroupchat.h  -  description
                             -------------------
    begin                : Fre Feb 28 2003
    copyright            : (C) 2003 by Till Gerken (till@tantalo.net)
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERGROUPCHAT_H
#define JABBERGROUPCHAT_H

#include "types.h"
#include "jabbercontact.h"

/**
  *@author Kopete developers
  */

class JabberGroupChat : public JabberContact
{

public: 
	JabberGroupChat(Jabber::Jid room, QStringList groups,
					JabberProtocol *p, KopeteMetaContact *mc, QString identity);
	~JabberGroupChat();

	void updatePresence(const Jabber::Jid &jid, const Jabber::Status &status);

private slots:
	virtual void slotMessageManagerDeleted();

private:
	Jabber::Jid room;

};

#endif
