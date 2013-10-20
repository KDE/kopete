/*
    accountconfig.h  -  Kopete account config page

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2003-2004 by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2007      by Will Stephenson <wstephenson@kde.org>

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

#ifndef KOPETEACCOUNTCONFIG_H
#define KOPETEACCOUNTCONFIG_H

#include <kcmodule.h>
#include <qmap.h>
#include <qcolor.h>

#include "kopeteonlinestatus.h"
#include "ui_kopeteaccountconfigbase.h"

namespace Kopete
{
class Account;
class Contact;
class Identity;
}

class KopeteAccountLVI;
class KopeteIdentityLVI;
class KMenu;
class KAction;

/**
 * @author Olivier Goffart <ogoffart@kde.org>
 */
class KopeteAccountConfig : public KCModule, private Ui::KopeteAccountConfigBase
{
	Q_OBJECT

public:
	KopeteAccountConfig(QWidget *parent, const QVariantList &args );

protected:
	virtual bool eventFilter( QObject *obj, QEvent *event );

public slots:
	virtual void save();
	virtual void load();

private:
	KopeteAccountLVI* selectedAccount();
	KopeteIdentityLVI* selectedIdentity();
	Kopete::OnlineStatus mStatus;
	
	void configureActions();
	void configureMenus();

	void modifyAccount(Kopete::Account *);
	void modifyIdentity(Kopete::Identity *);
	bool m_protected;
	KMenu *m_identityContextMenu;
	KMenu *m_accountContextMenu;

	KAction *m_actionAccountAdd;
	KAction *m_actionAccountModify;
	KAction *m_actionAccountRemove;
	KAction *m_actionAccountSwitchIdentity;
	KAction *m_actionAccountSetColor;

	KAction *m_actionIdentityAdd;
	KAction *m_actionIdentityCopy;
	KAction *m_actionIdentityModify;
	KAction *m_actionIdentityRemove;
	KAction *m_actionIdentitySetDefault;

private slots:
	void slotModify();

	void slotAddAccount();
	void removeAccount();
	void slotAccountSwitchIdentity();
	void slotAccountSetColor();

	void slotAddIdentity();
	void removeIdentity();
	void slotSetDefaultIdentity();

	void slotCopyIdentity();
	void slotAccountAdded(Kopete::Account *);
	void slotAccountRemoved(const Kopete::Account *);
	void slotItemSelected();
	void slotOnlineStatusChanged( Kopete::Contact *contact,
			                      const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldStatus );
	void slotItemChanged(QTreeWidgetItem*);
	void slotItemClicked ( QTreeWidgetItem * item, int column );
};
#endif

// vim: set noet ts=4 sts=4 sw=4:
