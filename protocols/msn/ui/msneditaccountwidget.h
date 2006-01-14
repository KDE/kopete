/*
    msneditaccountwidget.h - MSN Account Widget

    Copyright (c) 2003      by Olivier Goffart       <ogoffart @ kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNEDITACCOUNTWIDEGET_H
#define MSNEDITACCOUNTWIDEGET_H

#include <qwidget.h>

#include "editaccountwidget.h"

namespace Kopete { class Account; }

class MSNProtocol;

class MSNEditAccountWidgetPrivate;

/**
 * @author Olivier Goffart <ogoffart @ kde.org>
 */
class MSNEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
	Q_OBJECT

public:
	MSNEditAccountWidget( MSNProtocol *proto, Kopete::Account *account, QWidget *parent = 0, const char *name = 0 );
	~MSNEditAccountWidget();
	virtual bool validateData();
	virtual Kopete::Account * apply();

private slots:
	void slotAllow();
	void slotBlock();
	void slotShowReverseList();
	void slotSelectImage();
	void slotOpenRegister();

private:
	MSNEditAccountWidgetPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

