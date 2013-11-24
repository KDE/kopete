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

#include <kassistantdialog.h>

#include <kopete_export.h>
#include <kabc/resource.h>

#include "ui_kabcexport_page1.h"
#include "ui_kabcexport_page2.h"


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

class KOPETE_CONTACT_LIST_EXPORT KabcExportWizard : public KAssistantDialog
{
Q_OBJECT
	public:
		KabcExportWizard( QWidget *parent = 0 );
		~KabcExportWizard();
	public slots:
		void accept();
	protected slots:
		void slotDeselectAll();
		void slotSelectAll();
		void slotResourceSelectionChanged( QListWidgetItem * lbi );
	protected:
		void exportDetails( Kopete::MetaContact * mc, KABC::Addressee & addr );
	private:
		KABC::AddressBook* m_addressBook;
		QMap<int, KABC::Resource*> m_resourceMap;
		QMap<int, Kopete::MetaContact*> m_contactMap;
		Ui::KabcExportWizardPage1 m_page1;
		KPageWidgetItem *m_page1WidgetItem;
		Ui::KabcExportWizardPage2 m_page2;
		KPageWidgetItem *m_page2WidgetItem;
};

#endif
