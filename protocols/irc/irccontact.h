/***************************************************************************
                          msncontact.h  -  description
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

#ifndef MSNCONTACT_H
#define MSNCONTACT_H

#include <qlistview.h>
#include "imcontact.h"
#include <qobject.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qvaluestack.h>
#include <kiconloader.h>
#include <qtimer.h>
#include "kopete.h"

#include "ircprotocol.h"
#include "ircmessage.h"

class IRCMessage;

class IRCContact : public IMContact
{
	
	Q_OBJECT
	public:
		IRCContact();
		virtual void rightButtonPressed(const QPoint &);
		virtual void leftButtonDoubleClicked();
};
#endif
