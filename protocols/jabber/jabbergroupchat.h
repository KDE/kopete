
/***************************************************************************
                          jabbergroupchat.h  -  description
                             -------------------
    begin                : Fre Feb 28 2003
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
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

#include "jabbercontact.h"

/**
  *@author Till Gerken <till@tantalo.net>
  */

class JabberGroupChat:public JabberContact
{

  public:
	JabberGroupChat (XMPP::Jid room, QStringList groups, JabberAccount * p, KopeteMetaContact * mc);
	~JabberGroupChat ();

	void updatePresence (const XMPP::Jid & jid, const XMPP::Status & status);

	private slots:virtual void slotMessageManagerDeleted ();

  private:
	  XMPP::Jid room;

};

#endif
