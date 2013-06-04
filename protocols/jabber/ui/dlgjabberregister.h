
/***************************************************************************
                          dlgjabberregister.h  -  description
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

#ifndef DLGJABBERREGISTER_H
#define DLGJABBERREGISTER_H

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>

#include "im.h"
#include "xmpp.h"

#include "jabberaccount.h"
#include "ui_dlgregister.h"
#include "jabberformtranslator.h"

/**
  *@author Till Gerken <till@tantalo.net>
  */

class dlgJabberRegister : public QWidget, private Ui::dlgRegister
{

	Q_OBJECT

public:
	  dlgJabberRegister (JabberAccount *account, const XMPP::Jid & jid, QWidget * parent = 0);
	 ~dlgJabberRegister ();

private slots:
	void slotGotForm ();
	void slotSendForm ();
	void slotSentForm ();

private:
	  JabberAccount *m_account;
	  JabberFormTranslator * translator;

};

#endif
