/***************************************************************************
                          dlgjabberchatjoin.h  -  description
                             -------------------
    begin                : Fri Dec 13 2002
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

#ifndef DLGJABBERCHATJOIN_H
#define DLGJABBERCHATJOIN_H

#include <qwidget.h>
#include "dlgchatjoin.h"

/**
  *@author Kopete developers
  */

class DlgJabberChatJoin : public dlgChatJoin
{

	Q_OBJECT

public: 
	DlgJabberChatJoin(QWidget *parent=0, const char *name=0);
	~DlgJabberChatJoin();

private slots:
	void slotDialogDone();

};

#endif
