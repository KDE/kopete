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

#include <psi/types.h>
#include <psi/tasks.h>

#include "jabberformtranslator.h"
#include "dlgbrowse.h"

/**
  *@author Kopete developers
  */

class DlgJabberBrowse : public dlgBrowse
{

   Q_OBJECT

public: 
	DlgJabberBrowse(const Jabber::Jid &jid, QWidget *parent=0, const char *name=0);
	~DlgJabberBrowse();

private slots:
	void slotGotForm();
	void slotSendForm();
	void slotSentForm();

private:
	JabberFormTranslator *translator;

};

#endif
