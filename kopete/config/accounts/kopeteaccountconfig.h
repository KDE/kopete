/*
    accountconfig.h  -  Kopete account config page

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2003-2004 by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2003-2007 by the Kopete developers  <kopete-devel@kde.org>

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

#include <kcmodule.h>
#include <qmap.h>
#include <qcolor.h>

#include "kopeteonlinestatus.h"
#include "ui_kopeteaccountconfigbase.h"

namespace Kopete
{
class Account;
class Contact;
}

class KopeteAccountLVI;

/**
 * @author Olivier Goffart <ogoffart@kde.org>
 */
class KopeteAccountConfig : public KCModule, private Ui::KopeteAccountConfigBase
{
	Q_OBJECT

public:
	KopeteAccountConfig(QWidget *parent, const QStringList &args );

public slots:
	virtual void save();
	virtual void load();

private:
	KopeteAccountLVI* selectedAccount();
	Kopete::OnlineStatus mStatus;

	bool m_protected;

private slots:
	void slotRemoveAccount();
	void slotEditAccount();
	void slotAddAccount();
	void slotAddWizardDone();
	void slotItemSelected();
	void slotOnlineStatusChanged( Kopete::Contact *contact,
			                      const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldStatus );
};
#endif

// vim: set noet ts=4 sts=4 sw=4:
