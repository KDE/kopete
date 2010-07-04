
/***************************************************************************
                   jabberaccountwidget.h  -  Account widget for Jabber
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
                           Based on code by Olivier Goffart <ogoffart@kde.org>
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

#ifndef JABBEREDITACCOUNTWIDGET_H
#define JABBEREDITACCOUNTWIDGET_H

#include <qwidget.h>

#include "editaccountwidget.h"
#include "jabberaccount.h"
#include "ui_dlgjabbereditaccountwidget.h"
#include "jabberprotocol.h"

/**
  *@author Till Gerken <till@tantalo.net>
  */
#ifdef JINGLE_SUPPORT
class Item;
#endif

class JabberEditAccountWidget: public QWidget, public Ui::DlgJabberEditAccountWidget, public KopeteEditAccountWidget
{

Q_OBJECT

public:
	JabberEditAccountWidget (JabberProtocol * proto, JabberAccount *, QWidget * parent = 0);
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
	void awayPriorityToggled (bool);
	void updateServerField ();
	void slotPrivacyListsClicked ();

private:
	JabberProtocol *m_protocol;

#ifdef JINGLE_SUPPORT
	QList<Item> outputDevices;
	QList<Item> inputDevices;

	void checkAudioDevices();
#endif
	void reopen ();
	void writeConfig ();

};

#endif // JABBEREDITACCOUNTWIDGET_H
