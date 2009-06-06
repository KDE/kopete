
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

#include <kdialog.h>
#include "ui_dlgchatjoin.h"

#include "jabberaccount.h"

class dlgJabberChatJoin : public KDialog
{
  Q_OBJECT

public:
  explicit dlgJabberChatJoin(JabberAccount *account, QWidget* parent = 0);
  ~dlgJabberChatJoin();
  /*$PUBLIC_FUNCTIONS$*/

public slots:
  /*$PUBLIC_SLOTS$*/
  virtual void          slotJoin();
  virtual void          slotQuery();

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/

private:
	JabberAccount *m_account;
	Ui::dlgChatJoin m_ui;
	/*
		TODO : Used to look for the default chat server,
		this is duplicate with dlgjabberservices.h
		should be merged elsewhere !
	*/
	void checkDefaultChatroomServer();
private slots:
	void slotQueryFinished();
	void slotDiscoFinished();
	void slotChatRooomsQueryFinished();
	void slotCheckData();
	void slotDoubleClick(QTreeWidgetItem *item);

	// end todo.

};

#endif

