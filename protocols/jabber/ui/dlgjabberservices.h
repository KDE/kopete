/***************************************************************************
                          dlgjabberservices.h  -  description
                             -------------------
    begin                : Mon Dec 9 2002
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

#ifndef DLGJABBERSERVICES_H
#define DLGJABBERSERVICES_H

#include <qwidget.h>
#include "dlgservices.h"

/**
  *@author Kopete developers
  */

class DlgJabberServices : public dlgServices
{

   Q_OBJECT

public: 
	DlgJabberServices(QWidget *parent=0, const char *name=0);
	~DlgJabberServices();

private slots:
	void slotQuery();
	void slotQueryFinished();

};

#endif
