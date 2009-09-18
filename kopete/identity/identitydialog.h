/*
    identitydialog.h  -  Kopete identity configuration dialog

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

#ifndef IDENTITYDIALOG_H
#define IDENTITYDIALOG_H

#include <kopeteinfodialog.h>
#include <kopete_export.h>

namespace Kopete
{
class Identity;
}

class KOPETE_IDENTITY_EXPORT IdentityDialog : public Kopete::UI::InfoDialog
{
	Q_OBJECT
public:
	explicit IdentityDialog(Kopete::Identity *identity, QWidget *parent = 0);
	~IdentityDialog();

protected slots:
	void load();
	virtual void slotSave();

private slots:
	void slotSelectPhoto();
	void slotClearPhoto();

protected:
	void setPhoto(QString path);

private:
	class Private;
	Private * const d;

};


#endif

// vim: set noet ts=4 sts=4 sw=4:
