/*
    kopeteidentityconfig.h  -  Kopete identity config page

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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

#ifndef __KOPETEIDENTITYCONFIG_H
#define __KOPETEIDENTITYCONFIG_H

#define KDE3_SUPPORT
#include <kcmodule.h>
#undef KDE3_SUPPORT
#include <qmap.h>
#include <qcolor.h>

#include "kopeteonlinestatus.h"
#include "ui_kopeteidentityconfigbase.h"

namespace Kopete
{
class Identity;
}

class KopeteIdentityLVI;

/**
 * @author Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 */
class KopeteIdentityConfig : public KCModule, private Ui::KopeteIdentityConfigBase
{
	Q_OBJECT

public:
	KopeteIdentityConfig(QWidget *parent, const QStringList &args );

public slots:
	virtual void save();
	virtual void load();

private:
	KopeteIdentityLVI* selectedIdentity();

	bool m_protected;

private slots:
	void slotRemoveIdentity();
	void slotEditIdentity();
	void slotSetDefaultIdentity();
	void slotAddIdentity();
	void slotItemSelected();
};
#endif

// vim: set noet ts=4 sts=4 sw=4:
