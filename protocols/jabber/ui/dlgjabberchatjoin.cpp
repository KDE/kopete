
/***************************************************************************
                          dlgjabberchatjoin.cpp  -  description
                             -------------------
    begin                : Fri Dec 13 2002
    copyright            : (C) 2002 by Kopete developers
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

#include <qpushbutton.h>
#include "dlgjabberchatjoin.h"

dlgJabberChatJoin::dlgJabberChatJoin (QWidget * parent, const char *name):dlgChatJoin (parent, name)
{
	connect (buttonOk, SIGNAL (clicked ()), this, SLOT (slotDialogDone ()));
}

void dlgJabberChatJoin::slotDialogDone ()
{

	/*if(!JabberProtocol::protocol()->isConnected())
	   {
	   JabberProtocol::protocol()->errorConnectFirst();
	   return;
	   }

	   // send the join request
	   JabberProtocol::protocol()->jabberClient->groupChatJoin(leServer->text(), leRoom->text(), leNick->text());
	 */

}

dlgJabberChatJoin::~dlgJabberChatJoin ()
{
}

#include "dlgjabberchatjoin.moc"
