
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

#include <kdebug.h>
#include <klocale.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <qtable.h>
#include <qlabel.h>
#include "jabberprotocol.h"
#include "dlgjabberchooseserver.h"
#include "jabberregisteraccount.h"

JabberChooseServer::JabberChooseServer ( JabberRegisterAccount *parent, const char *name )
 : KDialogBase ( parent, name, true, i18n("Choose Jabber Server"),
 				 KDialogBase::Ok | KDialogBase::Cancel )
{

	mParentWidget = parent;
	mSelectedRow = -1;

	mMainWidget = new DlgJabberChooseServer ( this );
	setMainWidget ( mMainWidget );

	mMainWidget->lblStatus->setText ( i18n ( "Retrieving server list...") );

	mMainWidget->listServers->setLeftMargin ( 0 );

	// retrieve server list
	mTransferJob = KIO::get ( "http://www.jabber.org/servers.xml" );

	connect ( mTransferJob, SIGNAL ( result ( KIO::Job* ) ), this, SLOT ( slotTransferResult ( KIO::Job* ) ) );
	connect ( mTransferJob, SIGNAL ( data ( KIO::Job*, const QByteArray& ) ), this, SLOT ( slotTransferData ( KIO::Job*, const QByteArray& ) ) );

	connect ( mMainWidget->listServers, SIGNAL ( pressed ( int, int, int, const QPoint & ) ), this, SLOT ( slotSetSelection ( int ) ) );
	connect ( mMainWidget->listServers, SIGNAL ( doubleClicked ( int, int, int, const QPoint & ) ), this, SLOT ( slotOk () ) );

	enableButtonOK ( false );

}

JabberChooseServer::~JabberChooseServer()
{
}

void JabberChooseServer::slotOk ()
{

	if ( mSelectedRow != -1 )
	{
		mParentWidget->setServer ( mMainWidget->listServers->text ( mSelectedRow, 0 ) );
	}

	deleteLater ();

}

void JabberChooseServer::slotCancel ()
{

	deleteLater ();

}

void JabberChooseServer::slotSetSelection ( int row )
{

	mSelectedRow = row;
	mMainWidget->listServers->selectRow ( row );
	enableButtonOK ( true );

}

void JabberChooseServer::slotTransferData ( KIO::Job */*job*/, const QByteArray &data )
{

	unsigned oldSize = xmlServerList.size ();

	xmlServerList.resize ( oldSize + data.size () );

	memcpy ( &xmlServerList.data()[oldSize], data.data (), data.size () );

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Server list now " << xmlServerList.size () << endl;

}

void JabberChooseServer::slotTransferResult ( KIO::Job *job )
{

	if ( job->error () || mTransferJob->isErrorPage () )
	{
		mMainWidget->lblStatus->setText ( i18n ( "Could not retrieve server list." ) );
		return;
	}
	else
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Received server list ok!" << endl;

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

		mMainWidget->listServers->setNumRows ( docElement.childNodes().count () );

		int listIndex = 0;
		for( QDomNode node = docElement.firstChild (); !node.isNull (); node = node.nextSibling (), listIndex++ )
		{
			QDomNamedNodeMap attributes = node.attributes ();
			mMainWidget->listServers->setText ( listIndex, 0, attributes.namedItem ( "jid" ).nodeValue () );
			mMainWidget->listServers->setText ( listIndex, 1, attributes.namedItem ( "name" ).nodeValue () );
		}

		mMainWidget->listServers->adjustColumn ( 0 );
		mMainWidget->listServers->adjustColumn ( 1 );
	}

}


#include "jabberchooseserver.moc"
