/*
    kabcexport.h - Export Contacts to Address Book Wizard for Kopete

    Copyright (c) 2005 by Will Stephenson        <will@stevello.free-online.co.uk>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KABCEXPORTWIZARD_H
#define KABCEXPORTWIZARD_H

#include "kabcexport_base.h"

namespace KABC {
	class AddressBook;
	class Addressee;
}

namespace Kopete {
	class MetaContact;
}

namespace KRES {
	class Resource;
}

class KabcExportWizard : public KabcExportWizard_Base
{
Q_OBJECT
	public:
		KabcExportWizard( QWidget *parent = 0, const char *name = 0 );
		~KabcExportWizard();
	public slots:
		void accept();
	protected slots:
		void slotDeselectAll();
		void slotSelectAll();
		void slotResourceSelectionChanged( QListBoxItem * lbi );
	protected:
		void exportDetails( Kopete::MetaContact * mc, KABC::Addressee & addr );
	private:
		KABC::AddressBook* m_addressBook;
		QMap<int, KABC::Resource*> m_resourceMap;
		QMap<int, Kopete::MetaContact*> m_contactMap;
};

#endif
