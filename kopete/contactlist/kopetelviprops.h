/*
    kopetelviprops.h

    Kopete Contactlist Properties GUI for Groups and MetaContacts

    Copyright (c) 2002-2003 by Stefan Gehn            <metz AT gehn.net>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETELVIPROPS_H
#define KOPETELVIPROPS_H

#include <kdialogbase.h>

#include "kopetegvipropswidget.h"
#include "kopetemetalvipropswidget.h"

class KopeteGroupViewItem;
class KopeteMetaContactLVI;
class KopeteAddressBookExport;

class KopeteGVIProps: public KDialogBase
{
	Q_OBJECT

	public:
		KopeteGVIProps(KopeteGroupViewItem *gvi, QWidget *parent, const char *name=0L);
		~KopeteGVIProps();

	private:
		KopeteGVIPropsWidget *mainWidget;
		KopeteGroupViewItem *item;

	private slots:
		void slotOkClicked();
		void slotUseCustomIconsToggled(bool on);
};


class KopeteMetaLVIProps: public KDialogBase
{
	Q_OBJECT

	public:
		KopeteMetaLVIProps(KopeteMetaContactLVI *gvi, QWidget *parent, const char *name=0L);
		~KopeteMetaLVIProps();

	private:
		KopeteMetaLVIPropsWidget *mainWidget;
		KopeteMetaContactLVI *item;
		KopeteAddressBookExport *mExport;

	private slots:
		void slotOkClicked();
		void slotUseCustomIconsToggled( bool on );
		void slotHasAddressbookEntryToggled( bool on );
		void slotSelectAddresseeClicked();
		void slotMergeClicked();
};

#endif
