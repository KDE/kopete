
/***************************************************************************
                      dlgjabbersendraw.cpp  -  Raw XML dialog
                             -------------------
    begin                : Sun Aug 25 2002
    copyright            : (C) 2002 by Till Gerken,
                           Kopete Development team
    email                : till@tantalo.net
		***************************************************************************

		***************************************************************************
		*                                                                         *
		*   This program is free software; you can redistribute it and/or modify  *
		*   it under the terms of the GNU General Public License as published by  *
		*   the Free Software Foundation; either version 2 of the License, or     *
		*   (at your option) any later version.                                   *
		*                                                                         *
		***************************************************************************/

#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <kdebug.h>
#include "dlgjabbersendraw.h"


dlgJabberSendRaw::dlgJabberSendRaw (Jabber::Client * engine, QWidget * parent, const char *name):DlgSendRaw (parent, name)
{

	// Grab the thing that lets us talk
	mEngine = engine;

	// Connect the GUI elements to things that do stuff
	connect (btnSend, SIGNAL (clicked ()), this, SLOT (slotFinish ()));
	connect (btnClose, SIGNAL (clicked ()), this, SLOT (slotCancel ()));
	connect (inputWidget, SIGNAL (activated (int)), this, SLOT (sloCtreateMessage(int)));
}

dlgJabberSendRaw::~dlgJabberSendRaw ()
{
	// Nothing yet
}

void dlgJabberSendRaw::slotFinish ()
{
	// Debugging output
	kdDebug (14130) << "[dlgJabberSendRaw] Sending RAW message" << endl;
	// Tell our engine to send
	mEngine->send (tePacket->text ());
	// Hide ourselves
	hide ();
}

void dlgJabberSendRaw::slotCancel ()
{
	// Clear the contents
	tePacket->clear ();
	// Hide ourselves
	hide ();
}

void dlgJabberSendRaw::slotCreateMessage(int index)
{
}

#include "dlgjabbersendraw.moc"

/*
 * Local variables:
 * mode: c++
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:
