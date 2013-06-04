
/***************************************************************************
                          jabberaddcontactpage.h  -  Add contact widget
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
                           (C) 2003 by Daniel Stone <dstone@kde.org>
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

#ifndef JABBERADDCONTACTPAGE_H
#define JABBERADDCONTACTPAGE_H

#include <addcontactpage.h>
#include <QLabel>

/**
  *@author Daniel Stone
  */
namespace Ui { class dlgAddContact; }
class QLabel;

class JabberAddContactPage:public AddContactPage
{
  Q_OBJECT

public:
	  explicit JabberAddContactPage (Kopete::Account * owner, QWidget * parent = 0);
	 ~JabberAddContactPage ();
	virtual bool validateData ();
	virtual bool apply (Kopete::Account *, Kopete::MetaContact *);
	Ui::dlgAddContact *jabData;
	QLabel *noaddMsg1;
	QLabel *noaddMsg2;
	bool canadd;
public slots:
	void slotPromtReceived();
};

class JabberTransport;

/**
 * @author Olivier Goffart
 * this class is just there to workaround the fact that it's not possible to add contact assync with Kopete::AddContactPage::apply
 */
class  JabberAddContactPage_there_is_no_possibility_to_add_assync_WORKAROUND : public QObject
{ Q_OBJECT
	public:
		JabberAddContactPage_there_is_no_possibility_to_add_assync_WORKAROUND( JabberTransport * , Kopete::MetaContact *mc, QObject *parent);
		Kopete::MetaContact *metacontact;
		JabberTransport *transport;
	public slots:
		void slotJidReceived();
};


#endif


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:
