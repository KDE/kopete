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

#include <qlineedit.h>
#include <qpushbutton.h>
#include "dlgjabberchatjoin.h"

DlgJabberChatJoin::DlgJabberChatJoin(QWidget *parent, const char *name ) : dlgChatJoin(parent,name)
{
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotDialogDone()));
}

void DlgJabberChatJoin::slotDialogDone()
{

	emit okClicked();

}

const QString DlgJabberChatJoin::host() const
{
	return leServer->text();
}

const QString DlgJabberChatJoin::room() const
{
	return leRoom->text();
}

const QString DlgJabberChatJoin::nick() const
{
	return leNick->text();
}

DlgJabberChatJoin::~DlgJabberChatJoin()
{
}

#include "dlgjabberchatjoin.moc"
