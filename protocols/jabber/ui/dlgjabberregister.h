/***************************************************************************
                          dlgjabberregister.h  -  description
                             -------------------
    begin                : Mon Dec 9 2002
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

#ifndef DLGJABBERREGISTER_H
#define DLGJABBERREGISTER_H

#include <qwidget.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>

#include "types.h"
#include "tasks.h"

#include "dlgregister.h"
#include "jabberformtranslator.h"

/**
  *@author Kopete developers
  */

class DlgJabberRegister : public dlgRegister
{

   Q_OBJECT

public: 

	DlgJabberRegister(const Jabber::Jid &jid, QWidget *parent=0, const char *name=0);
	~DlgJabberRegister();

private slots:
	void slotGotForm();
	void slotSendForm();
	void slotSentForm();

private:
	JabberFormTranslator *translator;

};

#endif
