/*
    addcontactwizard.h - Kopete's Add Contact Wizard

    Copyright (c) 2003 by Will Stephenson        <will@stevello.free-online.co.uk>
    Copyright (c) 2002 by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ADDCONTACTWIZARD_H
#define ADDCONTACTWIZARD_H

#include <qptrlist.h>
#include <qvaluelist.h>
#include <qmap.h>

#include <kdebug.h>
#include <kabc/addressbook.h>
#include "addcontactwizard_base.h"

class AddContactPage;
class QCheckListItem;

namespace Kopete
{
class Protocol;
class Account;
class Group;

namespace UI
{
class AddressBookSelectorWidget;
}

}

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class AddContactWizard : public AddContactWizard_Base
{
	Q_OBJECT

public:
	AddContactWizard( QWidget *parent = 0, const char *name = 0 );
	~AddContactWizard();
	virtual void showPage( QWidget *page );

private:
	//Kopete::Protocol *currentProtocol;
	//AddContactPage *currentDataWidget;
	QMap <Kopete::Account*,AddContactPage*> protocolPages;
	QMap <QCheckListItem*,Kopete::Account*> m_accountItems;
	QMap <QCheckListItem*,Kopete::Group*> m_groupItems;
	Kopete::UI::AddressBookSelectorWidget *m_addressbookSelectorWidget;

public slots:
	virtual void accept();
	virtual void reject();

	void slotProtocolListClicked( QListViewItem * );

	void slotAddGroupClicked();

protected slots:
	virtual void next();
	void slotCheckAddresseeChoice( bool on );
	void slotAddresseeListClicked( QListViewItem *addressee );
	void slotDataValid( AddContactPage *onPage, bool bOn);
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

