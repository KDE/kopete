
/***************************************************************************
                   jabberaccountwidget.h  -  Account widget for Jabber
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
                           Based on code by Olivier Goffart <ogoffart @ kde.org>
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

#ifndef JABBEREDITACCOUNTWIDEGET_H
#define JABBEREDITACCOUNTWIDEGET_H

#include <qwidget.h>
#include <kprogress.h>
#include "editaccountwidget.h"
#include "jabberaccount.h"
#include "dlgjabbereditaccountwidget.h"
#include "jabberprotocol.h"

/**
  *@author Till Gerken <till@tantalo.net>
  */

class JabberEditAccountWidget:public DlgJabberEditAccountWidget, public KopeteEditAccountWidget
{

Q_OBJECT

public:
	JabberEditAccountWidget (JabberProtocol * proto, JabberAccount *, QWidget * parent = 0, const char *name = 0);
	~JabberEditAccountWidget ();
	virtual bool validateData ();
	virtual Kopete::Account *apply ();
	JabberAccount *account ();

private slots:
	void registerClicked ();
	void slotChangePasswordClicked ();
	void slotChangePasswordFinished ();
	void deleteClicked ();
	void sslToggled (bool);
	void updateServerField ();

private:
	JabberProtocol *m_protocol;

	void reopen ();
	void writeConfig ();

};

#endif
