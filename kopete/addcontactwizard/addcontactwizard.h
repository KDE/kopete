/*
    addcontactwizard.h - Kopete's Add Contact Wizard

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

#ifndef ADDCONTACTWIZARD_H
#define ADDCONTACTWIZARD_H

#include <qptrlist.h>
#include <qvaluelist.h>
#include <qmap.h>

#include <kdebug.h>

#include "addcontactwizard_base.h"

class AddContactPage;
class KopeteProtocol;
class QCheckListItem;
class KopeteAccount;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class AddContactWizard : public AddContactWizard_Base
{
	Q_OBJECT

public:
	AddContactWizard( QWidget *parent = 0, const char *name = 0 );
	~AddContactWizard();

private:
	//KopeteProtocol *currentProtocol;
	//AddContactPage *currentDataWidget;
	QMap <KopeteAccount*,AddContactPage*> protocolPages;
	QMap <QCheckListItem*,KopeteAccount*> m_accountItems;

public slots:
	virtual void accept();
	void slotProtocolListClicked( QListViewItem * );
	void slotGroupListClicked( QListViewItem * );

	void slotAddGroupClicked();
	void slotRemoveGroupClicked();

protected slots:
	virtual void next();
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

