/***************************************************************************
                          msncontact.cpp  -  description
                             -------------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "irccontact.h"
#include <kmessagebox.h>
#include <kdebug.h>

IRCContact::IRCContact()
	: IMContact(kopeteapp->contactList())
{
}

void IRCContact::rightButtonPressed(const QPoint &point)
{

}

void IRCContact::leftButtonDoubleClicked()
{
}

