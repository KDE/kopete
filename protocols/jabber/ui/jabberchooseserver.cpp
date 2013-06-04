
/***************************************************************************
                   jabberchooseserver.cpp  -  Server list for Jabber
                             -------------------
    begin                : Mon Jul 12 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

		Kopete (C) 2001-2004 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabberchooseserver.h"

#include <QDomDocument>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <kdebug.h>
#include <klocale.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <qlabel.h>
#include "jabberprotocol.h"
#include "ui_dlgjabberchooseserver.h"
#include "jabberregisteraccount.h"

JabberChooseServer::JabberChooseServer ( JabberRegisterAccount *parent )
 : KDialog ( parent )
{
	setCaption( i18n("Choose Jabber Server") );
	setButtons( KDialog::Ok | KDialog::Cancel );

	mParentWidget = parent;

	QWidget* w = new QWidget( this );
	mMainWidget = new Ui::DlgJabberChooseServer;
	mMainWidget->setupUi( w );
	setMainWidget ( w );

	mMainWidget->listServers->verticalHeader ()->setVisible ( false );
	mMainWidget->listServers->horizontalHeader ()->setClickable ( false );

	mMainWidget->lblStatus->setText ( i18n ( "Retrieving server list...") );

	// retrieve server list
	mTransferJob = KIO::get ( KUrl("http://xmpp.net/services.xml") );

	connect ( mTransferJob, SIGNAL (result(KJob*)), this, SLOT (slotTransferResult(KJob*)) );
	connect ( mTransferJob, SIGNAL (data(KIO::Job*,QByteArray)), this, SLOT (slotTransferData(KIO::Job*,QByteArray)) );

	connect ( mMainWidget->listServers, SIGNAL (cellClicked(int,int)), this, SLOT (slotListServerClicked()) );
	connect ( mMainWidget->listServers, SIGNAL (cellDoubleClicked(int,int)), this, SLOT (slotOk()) );

	connect ( this, SIGNAL (okClicked()), this, SLOT(slotOk()) );
        connect ( this,SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
	enableButtonOk ( false );

}

JabberChooseServer::~JabberChooseServer()
{
	delete mMainWidget;
}

void JabberChooseServer::slotOk ()
{
	QList<QTableWidgetItem *> mySelectedItem = mMainWidget->listServers->selectedItems();

	if ( !mySelectedItem.empty () )
	{
		mParentWidget->setServer ( mySelectedItem[0]->text() );
	}

	deleteLater ();

}

void JabberChooseServer::slotCancel ()
{

	deleteLater ();

}

void JabberChooseServer::slotListServerClicked ( )
{

	enableButtonOk ( true );

}

void JabberChooseServer::slotTransferData ( KIO::Job */*job*/, const QByteArray &data )
{

	unsigned oldSize = xmlServerList.size ();

	xmlServerList.resize ( oldSize + data.size () );

	memcpy ( &xmlServerList.data()[oldSize], data.data (), data.size () );

	kDebug ( JABBER_DEBUG_GLOBAL ) << "Server list now " << xmlServerList.size ();

}

void JabberChooseServer::slotTransferResult ( KJob *kJob )
{

	KIO::Job *job = static_cast<KIO::Job*>(kJob);
	if ( job->error () || mTransferJob->isErrorPage () )
	{
		mMainWidget->lblStatus->setText ( i18n ( "Could not retrieve server list." ) );
		return;
	}
	else
	{
		kDebug ( JABBER_DEBUG_GLOBAL ) << "Received server list ok!";

		// clear status message
		mMainWidget->lblStatus->setText ( "" );

		// parse XML list
		QDomDocument doc;

		if ( !doc.setContent ( xmlServerList ) )
		{
			mMainWidget->lblStatus->setText ( i18n ( "Could not parse the server list.") );
			return;
		}

		QDomElement docElement = doc.documentElement ();

		int listIndex = 0;
		QTableWidgetItem *newItem;
		for( QDomNode node = docElement.firstChild (); !node.isNull (); node = node.nextSibling (), listIndex++ )
		{
			mMainWidget->listServers->insertRow ( listIndex );
			QDomNamedNodeMap attributes = node.attributes ();

			newItem = new QTableWidgetItem ( attributes.namedItem ( "jid" ).nodeValue () );
			mMainWidget->listServers->setItem ( listIndex, 0, newItem );
			newItem = new QTableWidgetItem ( attributes.namedItem ( "name" ).nodeValue () );
			mMainWidget->listServers->setItem ( listIndex, 1, newItem );
		}

		mMainWidget->listServers->adjustSize();
	}

}


#include "jabberchooseserver.moc"
