/***************************************************************************
                          oscarchangestatus.h  -  description
                             -------------------
    begin                : Wed Jul 31 2002
    copyright            : (C) 2002 by twl6
    email                : twl6@po.cwru.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OSCARCHANGESTATUS_H
#define OSCARCHANGESTATUS_H

#include <qwidget.h>
#include <oscarchangestatusbase.h>

/**
 * The dialog that is dislpayed when we want to put up an away message
 * @author twl6
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
