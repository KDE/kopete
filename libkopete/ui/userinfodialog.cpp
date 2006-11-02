/*
    userinfodialog.h

    Copyright (c) 2003 by Zack Rusin <zack@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "userinfodialog.h"
#include "kopeteuiglobal.h"

#include <khtml_part.h>
#include <ktextbrowser.h>
#include <kapplication.h>
#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qvbox.h>
#include <qlayout.h>

namespace Kopete {

struct UserInfoDialog::UserInfoDialogPrivate {
	QString name;
	QString id;
	QString awayMessage;
	QString status;
	QString warningLevel;
	QString onlineSince;
	QString info;
	QString address;
	QString phone;
	QMap<QString,QString> customFields;
	QVBoxLayout *topLayout;
	QWidget     *page;
	DialogStyle style;
	KHTMLPart   *htmlPart;

	KLineEdit *nameEdit;
	KLineEdit *idEdit;
	KLineEdit *statusEdit;
	KLineEdit *warningEdit;
	KLineEdit *onlineEdit;
	KLineEdit *addressEdit;
	KLineEdit *phoneEdit;
	KTextBrowser *awayBrowser;
	KTextBrowser *infoBrowser;
};

UserInfoDialog::UserInfoDialog( const QString& descr )
: KDialogBase( Kopete::UI::Global::mainWidget(), "userinfodialog", true, i18n( "User Info for %1" ).arg( descr ), KDialogBase::Ok )
{
	d = new UserInfoDialogPrivate;
	d->page = new QWidget( this );
	setMainWidget( d->page );
	d->topLayout = new QVBoxLayout( d->page, 0, spacingHint() );
	d->style = Widget;
}

UserInfoDialog::~UserInfoDialog()
{
	delete d; d=0;
}

void UserInfoDialog::setStyle( DialogStyle style )
{
	d->style = style;
}

void UserInfoDialog::setName( const QString& name )
{
	d->name = name;
}

void UserInfoDialog::setId( const QString& id )
{
	d->id = id;
}

void UserInfoDialog::setAwayMessage( const QString& msg )
{
	d->awayMessage = msg;
}

void UserInfoDialog::setStatus( const QString& status )
{
	d->status = status;
}

void UserInfoDialog::setWarningLevel(const QString& level )
{
	d->warningLevel = level;
}

void UserInfoDialog::setOnlineSince( const QString& since )
{
	d->onlineSince = since;
}

void UserInfoDialog::setInfo( const QString& info )
{
	d->info = info;
}

void UserInfoDialog::setAddress( const QString& addr )
{
	d->address = addr;
}

void UserInfoDialog::setPhone( const QString& phone )
{
	d->phone = phone;
}

void UserInfoDialog::addCustomField( const QString& /*name*/, const QString& /*txt*/ )
{

}

void UserInfoDialog::addHTMLText( const QString& /*str*/ )
{

}

QHBox* UserInfoDialog::addLabelEdit( const QString& label, const QString& text, KLineEdit*& edit )
{
	QHBox *box = new QHBox( d->page );
	new QLabel( label, box );
	edit = new KLineEdit( box );
  edit->setAlignment( Qt::AlignHCenter );
	edit->setText( text );
	edit->setReadOnly( true );
	return box;
}

void UserInfoDialog::fillHTML()
{
	d->htmlPart = new KHTMLPart( this );

	QString text;
	/*
	if ( d->name.isEmpty() ) {
		text.append( QString("<div id=\"name\"><b>") + i18n("Name : ") +
								 QString("</b>") );
		text.append( d->name + QString("</div><br>") );
	}

	if ( d->id.isEmpty() ) {
		text.append( "<div id=\"id\"><b>" + i18n("Id : ") + "</b>" );
		text.append( d->id + "</div><br>" );
	}

	if ( d->warningLevel.isEmpty() ) {
		text.append( "<div id=\"warningLevel\"><b>" + i18n("Warning Level : ") + "</b>" );
		text.append( d->warningLevel + "</div><br>" );
	}

	if ( d->onlineSince.isEmpty() ) {
		text.append( "<div id=\"onlineSince\"><b>" + i18n("Online Since : ") + "</b>" );
		text.append( d->onlineSince + "</div><br>" );
	}

	if ( d->address.isEmpty() ) {
		text.append( "<div id=\"address\"><b>" + i18n("Address : ") + "</b>" );
		text.append( d->address + "</div><br>" );
	}

	if ( d->phone.isEmpty() ) {
		text.append( "<div id=\"phone\"><b>" + i18n("Phone : ") + "</b>" );
		text.append( d->phone + "</div><br>" );
	}

	if ( d->status.isEmpty() ) {
		text.append( "<div id=\"status\"><b>" + i18n("Status : ") + "</b>" );
		text.append( d->status + "</div><br>" );
	}

	if ( d->awayMessage.isEmpty() ) {
		text.append( "<div id=\"awayMessage\"><b>" + i18n("Away Message : ") + "</b>" );
		text.append( d->awayMessage + "</div><br>" );
	}

	if ( d->info.isEmpty() ) {
		text.append( "<div id=\"info\"><b>" + i18n("Info : ") + "</b>" );
		text.append( d->info + "</div><br>" );
	}
*/
	d->htmlPart->setOnlyLocalReferences( true );
	d->htmlPart->begin();
	d->htmlPart->write( text );
	d->htmlPart->end();
}

void UserInfoDialog::fillWidgets()
{
	kdDebug(14010)<<"Creating widgets"<<endl;
	if ( !d->name.isEmpty() ) {
		d->topLayout->addWidget( addLabelEdit( i18n("Name:"), d->name, d->nameEdit ) );
	}

	if ( !d->id.isEmpty() ) {
		d->topLayout->addWidget( addLabelEdit( i18n("Contact ID:"), d->id, d->idEdit ) );
	}

	if ( !d->status.isEmpty() ) {
		d->topLayout->addWidget( addLabelEdit( i18n("Status:"), d->status, d->statusEdit ) );
	}

	if ( !d->warningLevel.isEmpty() ) {
		d->topLayout->addWidget( addLabelEdit( i18n("Warning level:"), d->warningLevel, d->warningEdit ) );
	}

	if ( !d->onlineSince.isEmpty() ) {
		d->topLayout->addWidget( addLabelEdit( i18n("Online since:"), d->onlineSince, d->onlineEdit ) );
	}

	if ( !d->address.isEmpty() ) {
		d->topLayout->addWidget( addLabelEdit( i18n("Address:"), d->address, d->addressEdit ) );
	}

	if ( !d->phone.isEmpty() ) {
		d->topLayout->addWidget( addLabelEdit( i18n("Phone:"), d->phone, d->phoneEdit ) );
	}

	if ( !d->awayMessage.isEmpty() ) {
		QVBox *awayBox = new QVBox( d->page );
		new QLabel( i18n("Away message:"), awayBox );
		d->awayBrowser = new KTextBrowser( awayBox );
		d->awayBrowser->setText( d->awayMessage );
		d->topLayout->addWidget( awayBox );
	}

	if ( !d->info.isEmpty() ) {
		QVBox *infoBox = new QVBox( d->page );
		new QLabel( i18n("User info:"), infoBox );
		d->infoBrowser = new KTextBrowser( infoBox );
		d->infoBrowser->setText( d->info );
		d->topLayout->addWidget( infoBox );
	}
}

void UserInfoDialog::setStyleSheet( const QString& /*css*/ )
{
}

void UserInfoDialog::create()
{
	if ( d->style == HTML ) {
		fillHTML();
	} else {
		fillWidgets();
	}
}

void UserInfoDialog::show()
{
	create();
	KDialogBase::show();
}

}

#include "userinfodialog.moc"

// vim: set noet ts=4 sts=4 sw=4:

