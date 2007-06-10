/*
    Messenger Add contact UI
    Copyright (c) 2007 by pyzhang <pyzhang@gmail.com>

	Kopete    (c) 2002-2007 by The Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MESSENGERADDCONTACTPAGE_H
#define MESSENGERADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>
#include <qlabel.h>

/**
  *@author pyzhang
  */
namespace Ui { class messengerAddUI; }
class MessengerAccount;
class MessengerProtocol;

class MessengerAddContactPage : public AddContactPage
{
   Q_OBJECT

public:
	explicit MessengerAddContactPage(MessengerAccount *owner, QWidget *parent=0);
	~MessengerAddContactPage();

	virtual bool validateData();
	virtual bool apply( Kopete::Account*, Kopete::MetaContact* );

private:
	Ui::MessengerAddUI *addUI;
	MessengerAccount *mAccount;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

