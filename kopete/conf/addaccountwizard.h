/*
    addidentitywizard.h - Kopete Add Identity Wizard

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

#ifndef ADDIDENTITYWIZARD_H
#define ADDIDENTITYWIZARD_H

#include <qptrlist.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <klistview.h>

#include "addidentitywizard_base.h"


class EditIdentityWidget;
class KopeteProtocol;

/**
 * @author  Olivier Goffart <ogoffart@tiscalinet.be>
 */
class AddIdentityWizard : public AddIdentityWizard_Base
{
	Q_OBJECT

public:
	AddIdentityWizard( QWidget *parent = 0, const char *name = 0 , bool modale=false );
	~AddIdentityWizard();

private:
	EditIdentityWidget *identityPage;

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

