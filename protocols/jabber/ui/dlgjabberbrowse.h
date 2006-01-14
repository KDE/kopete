
/***************************************************************************
                          dlgjabberbrowse.h  -  description
                             -------------------
    begin                : Wed Dec 11 2002
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

#ifndef DLGJABBERBROWSE_H
#define DLGJABBERBROWSE_H

#include <qwidget.h>

#include "xmpp_tasks.h"

#include "jabberaccount.h"
#include "jabberformtranslator.h"
#include "dlgbrowse.h"

/**
  *@author Till Gerken <till@tantalo.net>
  */

class dlgJabberBrowse:public dlgBrowse
{

	Q_OBJECT

public:
	dlgJabberBrowse (JabberAccount *account, const XMPP::Jid & jid, QWidget * parent = 0, const char *name = 0);
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
