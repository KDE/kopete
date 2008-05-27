 /*
    icqaddcontactpage.h  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Stefan Gehn <metz@gehn.net>
    Copyright (c) 2004-2005 by Matt Rogers <mattr@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ICQADDCONTACTPAGE_H
#define ICQADDCONTACTPAGE_H

#include <qwidget.h>
#include <QShowEvent>
#include <addcontactpage.h>

/**
  *@author Matt Rogers
  *@author Stefan Gehn
  */
namespace Ui { class icqAddUI; }
class ICQAccount;
class ICQSearchDialog;

class ICQAddContactPage : public AddContactPage
{
Q_OBJECT

public:
	explicit ICQAddContactPage(ICQAccount *owner, QWidget *parent = 0);
	~ICQAddContactPage();

	virtual bool validateData();
	virtual bool apply(Kopete::Account* , Kopete::MetaContact *parentContact);

	void setUINFromSearch( const QString& );

protected:
	void showEvent(QShowEvent *e);

private slots:
	void showSearchDialog();
	void searchDialogDestroyed();
private:
	ICQAccount *mAccount;
	Ui::icqAddUI *addUI;
	ICQSearchDialog* m_searchDialog;
};

#endif

//kate: space-indent off; replace-tabs off; indent-mode csands;
