/*
    ircaddcontactpage.h - IRC Add Contact Widget

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

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

class ircAddUI;
class IRCProtocol;
class KopeteMetaContact;

/**
  *@author Nick Betcher <nbetcher@kde.org>
  */
class IRCAddContactPage : public AddContactPage
{
   Q_OBJECT
public:
	IRCAddContactPage(IRCProtocol *owner, QWidget *parent=0, const char *name=0);
	~IRCAddContactPage();
	ircAddUI *ircdata;
	IRCProtocol *plugin;
public slots:
	virtual void slotFinish(KopeteMetaContact *m);

private slots:
	virtual bool validateData();
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

