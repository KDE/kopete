/***************************************************************************
                          jabbergroupchat.cpp  -  description
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

#include "jabbergroupchat.h"

JabberGroupChat::JabberGroupChat(QString userId, QString nickname, QStringList groups,
								 JabberProtocol *p, KopeteMetaContact *mc, QString identity)
								 : JabberContact( userId, nickname, groups, p, mc, identity )
{
}

JabberGroupChat::~JabberGroupChat()
{
}                                
