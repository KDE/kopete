/*
	fastaddcontactwizard.h - Kopete's Fast Add Contact Wizard
	
	Copyright (c) 2003 by Will Stephenson        <will@stevello.free-online.co.uk>
	
	Derived from AddContactWizard
	Copyright (c) 2002 by Nick Betcher           <nbetcher@kde.org>
	Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
	
	Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>
	
	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; either version 2 of the License, or     *
	* (at your option) any later version.                                   *
	*                                                                       *
	*************************************************************************
*/
#ifndef FASTADDCONTACTWIZARD_H
#define FASTADDCONTACTWIZARD_H
#include <kdebug.h>
#include <klistview.h>

#include <qptrlist.h>
#include <qvaluelist.h>
#include <qmap.h>

#include <kdebug.h>
#include <klistview.h>

#include "fastaddcontactwizard_base.h"

class AddContactPage;
class QListViewItem;

namespace Kopete
{
class Account;
}

/**
 * This is a streamlined add contact wizard for users with simple tastes.
 * @author Will Stephenson
 */
 
class FastAddContactWizard : public FastAddContactWizard_Base
{
	Q_OBJECT
public:
	FastAddContactWizard( QWidget *parent = 0, const char *name = 0 );
	~FastAddContactWizard();
private:
	QMap <Kopete::Account*,AddContactPage*> protocolPages;
	QMap <QListViewItem*,Kopete::Account*> m_accountItems;
public slots:
	virtual void accept();
	void slotProtocolListClicked( QListViewItem * );
protected slots:
	virtual void next();
};

#endif
