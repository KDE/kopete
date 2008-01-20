
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

#ifndef DLGREGISTER_H
#define DLGREGISTER_H

#include <KDialog>

#include "im.h"
#include "xmpp.h"

class QLabel;
class JabberAccount;
class JabberFormTranslator;
class JabberXDataWidget;

class dlgRegister : public KDialog
{
	Q_OBJECT
public:
	  dlgRegister(JabberAccount *account, const XMPP::Jid &jid, QWidget *parent = 0);
	 ~dlgRegister();

private slots:
	void slotGotForm();
	void slotSendForm();
	void slotSentForm();

private:
	JabberAccount *mAccount;
	QWidget *mMainWidget;
	QLabel *lblWait;
	XMPP::Form mForm;
	JabberFormTranslator *mTranslator;
	JabberXDataWidget *mXDataWidget;
};

#endif
