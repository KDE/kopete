/***************************************************************************
   Copyright (c) 2002      by y0k0            <bj@altern.org>
   Copyright (c) 2007      by Charles Connell <charles@connells.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3ListViewItem>

#include <k3listview.h>
#include <klocale.h>
#include <qcheckbox.h>
#include <kiconloader.h>
#include <kicon.h>

#include "selectkeydialog.h"

#include <cstdio>

SelectKeyDialog::SelectKeyDialog ( QString &keyId, QWidget *parent )
		: KDialog ( parent )
{
	setCaption ( i18n ( "Private Key List" ) );
	setButtons ( KDialog::Ok | KDialog::Cancel );

	QWidget *page = new QWidget ( this );
	QLabel *labeltxt;
	mKeyId = &keyId;

	setMinimumSize ( 300,200 );
	mKeysListpr = new K3ListView ( page );
	mKeysListpr->setRootIsDecorated ( true );
	mKeysListpr->addColumn ( i18n ( "Name" ) );
	mKeysListpr->setShowSortIndicator ( true );
	mKeysListpr->setFullWidth ( true );

	labeltxt=new QLabel ( i18n ( "Choose secret key:" ),page );
	QVBoxLayout *vbox=new QVBoxLayout ( page );
	vbox->setSpacing ( 3 );

	vbox->addWidget ( labeltxt );
	vbox->addWidget ( mKeysListpr );
	
	fp = new QProcess (this);
	QStringList args;
	args << "--no-tty" << "--with-colon" << "--list-secret-keys";
	fp->setReadChannel (QProcess::StandardOutput);
	connect ( fp, SIGNAL ( readyRead () ), this, SLOT ( slotReadKey () ) );
	fp->start("gpg", args);

	QObject::connect ( mKeysListpr,SIGNAL ( doubleClicked ( Q3ListViewItem *,const QPoint &,int ) ),this,SLOT ( slotOk() ) );
	QObject::connect ( mKeysListpr,SIGNAL ( clicked ( Q3ListViewItem * ) ),this,SLOT ( slotSelect ( Q3ListViewItem * ) ) );

	mKeysListpr->setSelected ( mKeysListpr->firstChild(),true );

	page->show();
	resize ( this->minimumSize() );
	setMainWidget ( page );
}

SelectKeyDialog::~SelectKeyDialog ()
{
	delete fp;
}

void SelectKeyDialog::slotReadKey ( )
{
	QString line, expr, id;
	line = fp->readLine ( 0 );
	if ( line.startsWith ( "sec" ) )
	{
		QString expr = line.section ( ':',6,6 );
		if ( expr.isEmpty() )
			expr = i18nc ( "adj", "Unlimited" );
		QString id = QString ( "0x"+line.section ( ':',4,4 ).right ( 8 ) );
		K3ListViewItem *item = new K3ListViewItem ( mKeysListpr, line.section ( ':', 9, 9 ) );
		K3ListViewItem *sub = new K3ListViewItem ( item,i18n ( "ID: %1, expiration: %2", id, expr ) );
		sub->setSelectable ( false );
		KIconLoader *loader = KIconLoader::global();
		mKeyPair=loader->loadIcon ( "kgpg_key2",K3Icon::Small,20 );
		item->setPixmap ( 0,mKeyPair );

		*mKeyId = id;
	}
//	fp->ackRead();
}

void SelectKeyDialog::slotOk()
{
	if ( mKeysListpr->currentItem() ==NULL )
		reject();
	else
		accept();
}

void SelectKeyDialog::slotSelect ( Q3ListViewItem *item )
{
	if ( item==NULL ) return;
	if ( item->depth() !=0 )
	{
		mKeysListpr->setSelected ( item->parent(),true );
		mKeysListpr->setCurrentItem ( item->parent() );
	}
}

#include "selectkeydialog.moc"
