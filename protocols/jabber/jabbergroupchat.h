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

#include <jabbercontact.h>

/**
  *@author Kopete developers
  */

class JabberGroupChat : public JabberContact
{

public: 
	JabberGroupChat(QString userId, QString nickname, QStringList groups,
					JabberProtocol *p, KopeteMetaContact *mc, QString identity);
	~JabberGroupChat();

private:
	QDict<JabberContact*> members;

};

#endif
