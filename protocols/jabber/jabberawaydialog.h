/*
    jabberawaydialog.h - Away dialog for Jabber

    Copyright (c) 2003 by Till Gerken <till@tantalo.net>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef JABBERAWAYDIALOG_H
#define JABBERAWAYDIALOG_H

#include <qwidget.h>
#include <kopeteawaydialog.h>
#include <kopeteonlinestatus.h>
#include <jabberaccount.h>

// forward declaration required due to cyclic includes
class JabberAccount;

/**
@author Till Gerken
*/
class JabberAwayDialog : public KopeteAwayDialog
{
	Q_OBJECT

public:
	JabberAwayDialog(JabberAccount *account, QWidget* parent=0, const char* name=0);
	~JabberAwayDialog();

	virtual void setAway(int awayType);

private:
	JabberAccount *m_account;

};

#endif
