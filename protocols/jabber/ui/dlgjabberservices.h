
/***************************************************************************
                          dlgjabberservices.h  -  description
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

#ifndef DLGJABBERSERVICES_H
#define DLGJABBERSERVICES_H

#include <qwidget.h>

#include "jabberaccount.h"
#include "xmpp_tasks.h"

#include "ui_dlgservices.h"
#include <q3listview.h>

/**
  *@author Till Gerken <till@tantalo.net>
  */

class dlgJabberServices : public QDialog, private Ui::dlgServices
{
	Q_OBJECT

public:
	  dlgJabberServices (JabberAccount *account, QWidget *parent = 0, const char *name = 0);
	 ~dlgJabberServices ();

private slots:
	void slotSetSelection (Q3ListViewItem *);
	void slotService ();
	void slotServiceFinished ();
	void slotRegister ();
	void slotBrowse ();
	
	void slotDisco();
	void slotDiscoFinished();

private:
	JabberAccount *m_account;
	XMPP::Jid current_jid;

};


class dlgJabberServies_item : protected QObject, public Q3ListViewItem  
{
	Q_OBJECT
	public:
		dlgJabberServies_item( Q3ListView *parent , const QString &s1 , const QString &s2 ) 
			: Q3ListViewItem(parent,s1,s2), can_browse(false) , can_register(false) {}
		bool can_browse, can_register;
		XMPP::Jid jid;
		
		void updateInfo(const XMPP::Jid& jid, const QString &node , JabberAccount *account);
	private slots:
		void slotDiscoFinished();
};

#endif
