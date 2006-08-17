/*
    kopeteidentityconfig.h  -  Kopete identity config page

    Copyright (c) 2005 by Michaël Larouche <michael.larouche@kdemail.net>

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

#include "kopetemetacontact.h"

namespace Kopete
{
class Contact;
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
	~KopeteIdentityConfig();

public slots:
	virtual void save();
	virtual void load();

private:
	void loadIdentities();
	void saveCurrentIdentity();

	Kopete::MetaContact::PropertySource selectedNameSource() const;
	Kopete::MetaContact::PropertySource selectedPhotoSource() const;
	Kopete::Contact* selectedNameSourceContact() const;
	Kopete::Contact* selectedPhotoSourceContact() const;

private slots:
	void slotLoadNameSources();
	void slotLoadPhotoSources();
	void slotEnableAndDisableWidgets();

	void slotUpdateCurrentIdentity(const QString &selectedIdentity);
	void slotNewIdentity();
	void slotCopyIdentity();
	void slotRenameIdentity();
	void slotRemoveIdentity();

	void slotChangeAddressee();
	void slotChangePhoto(const QString &photoUrl);
	void slotClearPhoto();

	void slotSettingsChanged();

private:
	class Private;
	Private *d;

};
#endif

// vim: set noet ts=4 sts=4 sw=4:
