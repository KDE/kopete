/*
    oscarchangestatus.cpp  -  Oscar Protocol Plugin

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
  o  * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include "kopeteaway.h"
#include "oscarchangestatus.h"
#include "oscarsocket.h"


OscarChangeStatus::OscarChangeStatus(OscarSocket *engine,
				QWidget *parent, const char *name )
				: KopeteAwayDialog(parent, name)
{
		// Pointer to the oscar engine
		mEngine = engine;
		// Set us as modal
		setWFlags(Qt::WType_Modal);
		// Set our caption (from KDialog)
		setCaption(i18n("Select Away Message"));
		
}

void OscarChangeStatus::slotOkayClicked(){
		// Get the away message and set it
		mEngine->sendAway(OSCAR_AWAY, getSelectedAwayMessage());
		
		// Close the dialog
		close();
}

#include "oscarchangestatus.moc"

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
