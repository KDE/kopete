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

#include "addaccountwizardpage1.h"
#include "addaccountwizardpage2.h"
#include "addaccountwizardpage3.h"

#include <qmap.h>
#include <kwizard.h>

class EditAccountWidget;
class KopeteProtocol;
class QListViewItem;

/**
 * @author  Olivier Goffart <ogoffart@tiscalinet.be>
 */
class AddAccountWizard : public KWizard
{
	Q_OBJECT

	public:
		AddAccountWizard( QWidget *parent = 0, const char *name = 0 , bool modal=false );
		~AddAccountWizard();

	private slots:
		void slotProtocolListClicked( QListViewItem * );

	protected slots:
		virtual void next();
		virtual void accept();

	private:
		QMap <QListViewItem*,KopeteProtocol*> m_protocolItems;
		EditAccountWidget *accountPage;
		AddAccountWizardPage1 *intro;
		AddAccountWizardPage2 *selectService;
		AddAccountWizardPage3 *finish;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

