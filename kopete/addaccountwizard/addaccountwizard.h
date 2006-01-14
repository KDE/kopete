/*
    addaccountwizard.h - Kopete Add Account Wizard

    Copyright (c) 2003      by Olivier Goffart       <ogoffart @ kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

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

#include <qmap.h>

#include <kwizard.h>

class QListViewItem;

class KPluginInfo;

namespace Kopete
{
class Protocol;
}

class AddAccountWizardPage1;
class AddAccountWizardPage2;
class KopeteEditAccountWidget;

/**
 * @author  Olivier Goffart <ogoffart @ kde.org>
 */
class AddAccountWizard : public KWizard
{
	Q_OBJECT

public:
	AddAccountWizard( QWidget *parent = 0, const char *name = 0 , bool modal = false, bool firstRun = false );

private slots:
	void slotProtocolListClicked( QListViewItem *item );
	void slotProtocolListDoubleClicked( QListViewItem *lvi );

protected slots:
	virtual void back();
	virtual void next();
	virtual void accept();
	virtual void reject();

private:
	QMap<QListViewItem *, KPluginInfo *>  m_protocolItems;
	KopeteEditAccountWidget              *m_accountPage;
	AddAccountWizardPage1                *m_selectService;
	AddAccountWizardPage2                *m_finish;
	Kopete::Protocol                     *m_proto;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

