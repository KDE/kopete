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
class KopeteMetaContact;
class IRCAccount;
class QListViewItem;

/**
  *@author Nick Betcher <nbetcher@kde.org>
  */
class IRCAddContactPage : public AddContactPage
{
   Q_OBJECT
public:
	IRCAddContactPage(QWidget *parent=0, IRCAccount* account = 0);
	~IRCAddContactPage();
	ircAddUI *ircdata;
public slots:
	virtual bool apply(KopeteAccount *account , KopeteMetaContact *m);

private slots:
	virtual bool validateData();
	void slotSearch();
	void slotListedChannel( const QString &channel, uint users, const QString &topic );
	void slotListEnd();
	void slotSelectionChanged( QListViewItem *i );
private:
	IRCAccount *mAccount;
	QString search;
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

