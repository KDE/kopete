/*
    kopetelviprops.h

    Kopete Contactlist Properties GUI for Groups and MetaContacts

    Copyright (c) 2002-2003 by Stefan Gehn            <metz@gehn.net>

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

#include <kdialog.h>
#include <kabc/sound.h>

#include "kopetemetacontact.h"

#include "ui_kopetemetalvipropswidget.h"
#include "ui_kopetegvipropswidget.h"

class AddressBookLinkWidget;
class CustomNotificationProps;
class KopeteAddressBookExport;
class KUrlRequester;

namespace KABC { class Addressee; }
namespace Kopete {
	class Contact;
	class Group;
}

class KopeteGVIProps: public KDialog
{
	Q_OBJECT

	public:
		KopeteGVIProps(Kopete::Group *group, QWidget *parent);
		~KopeteGVIProps();

	private:
		CustomNotificationProps * mNotificationProps;
		QWidget *mainWidget;
		Ui::KopeteGVIPropsWidget *ui_mainWidget;
		Kopete::Group *mGroup;
		bool m_dirty;

	private slots:
		void slotOkClicked();
		void slotUseCustomIconsToggled(bool on);
		void slotIconChanged();
};


class KopeteMetaLVIProps: public KDialog
{
	Q_OBJECT

	public:
		KopeteMetaLVIProps(Kopete::MetaContact *metaContact, QWidget *parent);
		~KopeteMetaLVIProps();

	private:
		CustomNotificationProps * mNotificationProps;
		QPushButton *mFromKABC;
		QWidget* mainWidget;
		Ui::KopeteMetaLVIPropsWidget *ui_mainWidget;
		AddressBookLinkWidget *linkWidget;
		Kopete::MetaContact *mMetaContact;
		KopeteAddressBookExport *mExport;
		KABC::Sound mSound;
		int m_countPhotoCapable;
		QMap<int, Kopete::Contact *> m_withPhotoContacts;
		QString mAddressBookUid; // the currently selected addressbook UID
		QString m_photoPath;
		
		void setContactsNameTypes();
		
		Kopete::MetaContact::PropertySource selectedNameSource() const;
		Kopete::MetaContact::PropertySource selectedPhotoSource() const;
		Kopete::Contact* selectedNameSourceContact() const;
		Kopete::Contact* selectedPhotoSourceContact() const;
	private slots:
		void slotOkClicked();
		void slotUseCustomIconsToggled( bool on );
		void slotClearPhotoClicked();
		void slotAddresseeChanged( const KABC::Addressee & );
		void slotExportClicked();
		void slotImportClicked();
		void slotFromKABCClicked();
		void slotOpenSoundDialog( KUrlRequester *requester );
		void slotLoadNameSources();
		void slotLoadPhotoSources();
		void slotSelectPhoto();
		void slotEnableAndDisableWidgets();
};

#endif
