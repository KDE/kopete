
/***************************************************************************
                          dlgjabberbrowse.h  -  description
                             -------------------
    begin                : Wed Dec 11 2002
    copyright            : (C) 2002 by Till Gerken
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

#ifndef DLGJABBERBROWSE_H
#define DLGJABBERBROWSE_H

#include <qwidget.h>

#include "types.h"
#include "tasks.h"

#include "jabberaccount.h"
#include "jabberformtranslator.h"
#include "dlgbrowse.h"

/**
  *@author Kopete developers
  */

class dlgJabberBrowse:public dlgBrowse
{

	Q_OBJECT

public:
	dlgJabberBrowse (JabberAccount *account, const Jabber::Jid & jid, QWidget * parent = 0, const char *name = 0);
	~dlgJabberBrowse ();

private slots:
	void slotGotForm ();
	void slotSendForm ();
	void slotSentForm ();

private:
	JabberAccount *m_account;
	JabberFormTranslator * translator;

};

#endif
