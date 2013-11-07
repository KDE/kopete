/*
    AddressBookLinkWidget

    A compact widget for showing and changing which address book item a
    particular Kopete::MetaContact is related to.

    Comprises a label showing the contact's name, a Clear button, and a Change
    button that usually invokes the AddressBookSelectorWidget.

    Copyright (c) 2006 by Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "addressbooklinkwidget.h"

#include <qapplication.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>

#include <kiconloader.h>

#include <kopetemetacontact.h>

#include "addressbookselectordialog.h"
#include "addressbookselectorwidget.h"

namespace Kopete {
namespace UI {


AddressBookLinkWidget::AddressBookLinkWidget( QWidget * parent, const char * name ) : QWidget(parent), Ui::AddressBookLinkWidgetBase(), mMetaContact( 0 )
{
	setObjectName(name);
	setupUi(this);

	btnClear->setIcon( KIcon( (QApplication::layoutDirection() == Qt::RightToLeft) ? QString::fromLatin1( "edit-clear-locationbar-ltr" ) : QString::fromLatin1( "edit-clear-locationbar-rtl") ) );
	connect( btnClear, SIGNAL(clicked()), this, SLOT(slotClearAddressee()) );
	connect( btnSelectAddressee, SIGNAL(clicked()), SLOT(slotSelectAddressee()) );
}

void AddressBookLinkWidget::setAddressee( const KABC::Addressee& addr )
{
	edtAddressee->setText( addr.realName() );
	btnClear->setEnabled( !addr.isEmpty() );
}

void AddressBookLinkWidget::setMetaContact( const Kopete::MetaContact * mc )
{
	mMetaContact = mc;
}

QString AddressBookLinkWidget::uid() const
{
	return mSelectedUid;
}

void AddressBookLinkWidget::slotClearAddressee()
{
	edtAddressee->clear();
	btnClear->setEnabled( false );
	KABC::Addressee mrEmpty;
	mSelectedUid.clear();
	emit addresseeChanged( mrEmpty );
}

void AddressBookLinkWidget::slotSelectAddressee()
{
	QString message;
	if ( mMetaContact )
		message = i18n("Choose the corresponding entry for '%1'", mMetaContact->displayName() );
 	else
		message = i18n("Choose the corresponding entry in the address book" );

	QString assocDisplayText;
	if ( mMetaContact )
	{
		assocDisplayText = mMetaContact->kabcId();
	}
	QPointer <Kopete::UI::AddressBookSelectorDialog> dialog = new Kopete::UI::AddressBookSelectorDialog( i18n("Address Book Association"), message,
	                                              assocDisplayText, this );
	int result = dialog->exec();

	KABC::Addressee addr;
	if ( result == QDialog::Accepted && dialog )
	{
		addr = dialog->addressBookSelectorWidget()->addressee();

		edtAddressee->setText( addr.realName() );
		btnClear->setEnabled( !addr.isEmpty() );
		mSelectedUid = ( addr.isEmpty() ? QString() : addr.uid() );
		emit addresseeChanged( addr );
	}
	delete dialog;
}

} // end namespace UI
} // end namespace Kopete

#include "addressbooklinkwidget.moc"
