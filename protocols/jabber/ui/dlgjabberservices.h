
/***************************************************************************
                          dlgjabberservices.h  -  description
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
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

#include "jabberaccount.h"
#include "xmpp_tasks.h"

#include "dlgservices.h"

/**
  *@author Till Gerken <till@tantalo.net>
  */

class dlgJabberServices:public dlgServices
{
	Q_OBJECT

public:
	  dlgJabberServices (JabberAccount *account, QWidget *parent = 0, const char *name = 0);
	 ~dlgJabberServices ();

private slots:
	void slotSetSelection (int row, int, int, const QPoint &);
	void slotQuery ();
	void slotQueryFinished ();
	void slotRegister ();
	void slotBrowse ();

private:
	JabberAccount *m_account;
	XMPP::JT_GetServices * serviceTask;
	int selectedRow;

};

#endif
