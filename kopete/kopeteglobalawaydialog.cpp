/*
  kopeteglobalawaydialog.cpp  -  Kopete Global Away Dialog

  Copyright (c) 2002 by Christopher TenHarmsel <tenharmsel@users.sourceforge.net>

  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#include "kopeteglobalawaydialog.h"
#include "kopeteaccountmanager.h"
#include "kopeteaway.h"
#include <kdebug.h>

KopeteGlobalAwayDialog::KopeteGlobalAwayDialog(QWidget *parent, const char *name)
	: KopeteAwayDialog(parent, name)
{
}

void KopeteGlobalAwayDialog::setAway( int /*awayType*/ )
{
	kdDebug(14000) << k_funcinfo << "Setting all protocols away " << endl;
	// Set the global away message
	awayInstance->setGlobalAwayMessage( getSelectedAwayMessage() );
	// Tell all the protocols to set themselves away.
	Kopete::AccountManager::self()->setAwayAll( getSelectedAwayMessage() );
}

#include "kopeteglobalawaydialog.moc"
// vim: set noet ts=4 sts=4 sw=4:
