/*
    identityconfig.h  -  Kopete identity config page

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

#ifndef __IDENTITYCONFIG_H
#define __IDENTITYCONFIG_H

#include "configmodule.h"

/**
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 */
 
class IdentityConfigBase;
class KopeteIdentity;
class QListViewItem;
//class AddIdentityWizard;
 
class IdentityConfig : public ConfigModule
{
	Q_OBJECT

public:
	IdentityConfig(QWidget * parent);
	~IdentityConfig();

	virtual void save();
	virtual void reopen();

private:
	IdentityConfigBase *m_view;
	QMap <QListViewItem*,KopeteIdentity*> m_identityItems;

private slots:
	void slotRemoveIdentity();
	void slotEditIdentity();
	void slotAddIdentity();
	void slotAddWizardDone();
	void slotItemSelected();
	void slotIdentityUp();
	void slotIdentityDown();
};
#endif

// vim: set noet ts=4 sts=4 sw=4:
