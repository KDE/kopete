/*
    ircaddcontactpage.h - IRC Add Contact Widget

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCADDCONTACTPAGE_H
#define IRCADDCONTACTPAGE_H

#include "addcontactpage.h"

namespace Kopete
{
class MetaContact;
 }

class IRCAccount;
class IRCAddContactPagePrivate;

/**
  *@author Nick Betcher <nbetcher@kde.org>
  */
class IRCAddContactPage
	: public AddContactPage
{
   Q_OBJECT
public:
	IRCAddContactPage(QWidget *parent=0, IRCAccount* account = 0);
	~IRCAddContactPage();

public slots:
	virtual bool apply(Kopete::Account *account , Kopete::MetaContact *m);

private slots:
	virtual bool validateData();
	void slotChannelSelected( const QString &channel );
	void slotChannelDoubleClicked( const QString &channel );

private:
	IRCAddContactPagePrivate *d;
};

#endif

