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

#include <qpushbutton.h>
#include <qtextedit.h>
#include "dlgjabbersendraw.h"
#include "dlgsendraw.h"
#include "jabberprotocol.h"

dlgJabberSendRaw::dlgJabberSendRaw(JabberProtocol *owner, QWidget *parent, const char *name) : dlgSendRaw(parent, name)
{

	plugin = owner;

	connect(btnFinish, SIGNAL(clicked()), this, SLOT(slotFinish()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

	show();

}

dlgJabberSendRaw::~dlgJabberSendRaw()
{

}

void dlgJabberSendRaw::slotFinish()
{

	plugin->sendRawMessage(tePacket->text());
	hide();

}

void dlgJabberSendRaw::slotCancel()
{

	hide();

}

#include "dlgjabbersendraw.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

