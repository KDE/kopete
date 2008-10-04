
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

#ifndef DLGSEARCH_H
#define DLGSEARCH_H

#include "jabberaccount.h"
#include "jabberformtranslator.h"
#include "ui_dlgsearch.h"

class JabberXDataWidget;

class dlgSearch : public KDialog
{
	Q_OBJECT
public:
	dlgSearch(JabberAccount *account, const XMPP::Jid &jid, QWidget *parent = 0);
	~dlgSearch();

private slots:
	void slotGotForm();
	void slotSendForm();
	void slotSentForm();

private:
	Ui::dlgSearch ui;
	JabberAccount *mAccount;
	JabberXDataWidget *mXDataWidget;
	Form mForm;
	JabberFormTranslator * translator;
};

#endif
