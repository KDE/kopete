/*
  oscarchangestatus.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

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

#include "oscarchangestatus.h"

#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include "kopeteaway.h"
#include "oscarsocket.h"


OscarChangeStatus::OscarChangeStatus(OscarSocket *engine, QWidget *parent, const char *name)
	: KopeteAwayDialog(parent, name)
{

	mEngine = engine; // Pointer to the oscar engine
	setCaption(i18n("Select Away Message")); // Set our caption (from KDialog)
}

void OscarChangeStatus::setAway(QString awayType)
{
	// We don't really have away types, so we'll
	// ignore it
	// Get the away message and set it
	mEngine->sendAway(OSCAR_AWAY, getSelectedAwayMessage());
}

#include "oscarchangestatus.moc"
// vim: set noet ts=4 sts=4 sw=4:
