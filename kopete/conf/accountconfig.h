/*
    accountconfig.h  -  Kopete account config page

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

#ifndef __ACCOUNTCONFIG_H
#define __ACCOUNTCONFIG_H

#include "configmodule.h"

/**
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 */
 
class AccountConfigBase;
class KopeteAccount;
class QListViewItem;
//class AddAccountWizard;
 
class AccountConfig : public ConfigModule
{
	Q_OBJECT

public:
	AccountConfig(QWidget * parent);
	~AccountConfig();

	virtual void save();
	virtual void reopen();

private:
	AccountConfigBase *m_view;
	QMap <QListViewItem*,KopeteAccount*> m_accountItems;
	KopeteAccount *previousAccount;


private slots:
	void slotRemoveAccount();
	void slotEditAccount();
	void slotAddAccount();
	void slotAddWizardDone();
	void slotItemSelected();
	void slotAccountUp();
	void slotAccountDown();
};
#endif

// vim: set noet ts=4 sts=4 sw=4:
