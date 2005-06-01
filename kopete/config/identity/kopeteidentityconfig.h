/*
    kopeteidentityconfig.h  -  Kopete identity config page

    Copyright (c) 2005 by Michaël Larouche <shock@shockdev.ca.tc>

    Kopete    (c) 2003-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef _IDENTITYCONFIG_H
#define _IDENTITYCONFIG_H

#include <kcmodule.h>
#include <kconfigdialog.h>

namespace Kopete
{
class Account;
}

class KopeteIdentityConfigBase;

/**
 * @author Michaël Larouche <shock@shockdev.ca.tc>
 */
class KopeteIdentityConfig : public KCModule
{
	Q_OBJECT

public:
	KopeteIdentityConfig(QWidget *parent, const char *name, const QStringList &args );

public slots:
	virtual void save();
	virtual void load();

private:
	KopeteIdentityConfigBase *m_view;
	QMap<QString, Kopete::Account*> m_listAccounts;

	bool m_useGlobal,m_useAccount;
	QString m_protocolSelected, m_accountSelected, m_nickname;

private slots:
	void slotSelectImage();
	void slotTextChanged(const QString &);
	void slotSettingsChanged();
	void slotComboActivated(int);
};
#endif

// vim: set noet ts=4 sts=4 sw=4:
