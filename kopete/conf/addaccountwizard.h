/*
    addaccountwizard.h - Kopete Add Account Wizard

    Copyright (c) 2003 by Olivier Goffart <ogoffart@tiscalinet.be>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ADDACCOUNTWIZARD_H
#define ADDACCOUNTWIZARD_H

#include <qptrlist.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <klistview.h>

#include "addaccountwizard_base.h"


class EditAccountWidget;
class KopeteProtocol;

/**
 * @author  Olivier Goffart <ogoffart@tiscalinet.be>
 */
class AddAccountWizard : public AddAccountWizard_Base
{
	Q_OBJECT

public:
	AddAccountWizard( QWidget *parent = 0, const char *name = 0 , bool modale=false );
	~AddAccountWizard();

private:
	EditAccountWidget *accountPage;

private slots:
	void slotProtocolListClicked( QListViewItem * );

protected slots:
	virtual void next();
	virtual void accept();

private:

	QMap <QListViewItem*,KopeteProtocol*> m_protocolItems;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

