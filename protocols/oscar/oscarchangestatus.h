/*
    oscarchangestatus.h  -  Oscar Protocol Plugin

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef OSCARCHANGESTATUS_H
#define OSCARCHANGESTATUS_H

#include <qwidget.h>
#include <oscarchangestatusbase.h>

/**
 * The dialog that is dislpayed when we want to put up an away message
 * @author Tom Linsky <twl6@po.cwru.edu>
 * @author Chris TenHarmsel
 */

class OscarChangeStatus : public OscarChangeStatusBase  {
   Q_OBJECT
public: 
	OscarChangeStatus(QWidget *parent=0, const char *name=0);
	~OscarChangeStatus();
  /** Gets a status message */
  QString getStatusMessage(void);

};

#endif
