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
#include "oscarchangestatus.moc"

#include "oscarprotocol.h"
#include "oscarsocket.h"

//#include <klineedit.h>
//#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>

#include "kopeteaway.h"
#include "oscarsocket.h"

// TODO: also make this compatible to ICQ

OscarChangeStatus::OscarChangeStatus(OscarSocket *engine, QWidget *parent, const char *name)
	: KopeteAwayDialog(parent, name)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	mEngine = engine; // Pointer to the oscar engine
	setCaption(i18n("Select Away Message")); // Set our caption (from KDialog)
}

void OscarChangeStatus::setAway(int awayType)
{
	kdDebug(14150) << k_funcinfo << "awayType=" << awayType << endl;
	// We don't really have away types, so we'll
	// ignore it
	// Get the away message and set it
	switch(awayType)
	{
		case (OscarProtocol::ICQAWAY):
			mEngine->sendStatus(ICQ_STATUS_AWAY); // TODO: also handle awaymessage
			break;

		case (OscarProtocol::AIMAWAY):
			mEngine->sendAway(true, getSelectedAwayMessage());
			break;

		default:
			kdDebug(14150) << k_funcinfo << "Unknown away type, aborting." << endl;
	}
}
// vim: set noet ts=4 sts=4 sw=4:
