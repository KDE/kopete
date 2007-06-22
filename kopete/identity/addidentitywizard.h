/*
    addidentitywizard.h - Kopete Add Identity Wizard

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ADDIDENTITYWIZARD_H
#define ADDIDENTITYWIZARD_H

#include <QMap>

#include <KAssistantDialog>

#include <kopete_export.h>

#include "ui_addidentitywizardpage1.h"
//#include "ui_addidentitywizardpage2.h"

/**
 * @author Gustavo Pichorim Boiko <gustavo.boiko@kemail.net>
 */
class KOPETEADDIDENTITYWIZARD_EXPORT AddIdentityWizard : public KAssistantDialog
{
	Q_OBJECT

public:
	explicit AddIdentityWizard( QWidget *parent = 0 );
	~AddIdentityWizard();

private slots:
	void slotValidate();
	void slotIdentityListDoubleClicked();

protected slots:
	virtual void back();
	virtual void next();
	virtual void accept();
	virtual void reject();

private:
	class Private;
	Private *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

