
/***************************************************************************
                          dlgjabberchatjoin.h  -  description
                             -------------------
    begin                : Fri Dec 13 2002
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

#ifndef DLGJABBERCHATJOIN_H
#define DLGJABBERCHATJOIN_H

#include <kdialogbase.h>
#include "jabberaccount.h"

/**
  *@author Till Gerken <till@tantalo.net>
  */

class dlgJabberChatJoin : public KDialogBase
{

	Q_OBJECT

public:
	  dlgJabberChatJoin (JabberAccount *account, QWidget * parent = 0, const char *name = 0);

private slots:
	void slotOk ();
	void slotCancel ();

private:
	JabberAccount *m_account;

};

#endif
