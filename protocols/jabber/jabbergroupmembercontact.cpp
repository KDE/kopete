 /*
  * jabbergroupmembercontact.cpp  -  Regular Kopete Jabber protocol contact
  *
  * Copyright (c) 2002-2004 by Till Gerken <till@tantalo.net>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#include "jabbergroupmembercontact.h"

#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabberfiletransfer.h"
#include "jabbergroupchatmanager.h"
#include "jabbercontactpool.h"
#include "kopetemetacontact.h"

/**
 * JabberGroupMemberContact constructor
 */
JabberGroupMemberContact::JabberGroupMemberContact (const XMPP::RosterItem &rosterItem,
													JabberAccount *account, KopeteMetaContact * mc)
													: JabberBaseContact ( rosterItem, account, mc)
{

	setDisplayName ( rosterItem.jid().resource() );

	setFileCapable ( true );

}

QPtrList<KAction> *JabberGroupMemberContact::customContextMenuActions ()
{

	return 0;

}

void JabberGroupMemberContact::rename ( const QString &/*newName*/ )
{

}

KopeteMessageManager *JabberGroupMemberContact::manager ( bool /*canCreate*/ )
{

	return 0;

}

void JabberGroupMemberContact::handleIncomingMessage ( const XMPP::Message &/*message*/ )
{

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "WARNING: Message passed on to group member contact, ignoring!" << endl;

}

void JabberGroupMemberContact::sendFile ( const KURL &sourceURL, const QString &/*fileName*/, uint /*fileSize*/ )
{
	QString filePath;

	// if the file location is null, then get it from a file open dialog
	if ( !sourceURL.isValid () )
		filePath = KFileDialog::getOpenFileName( QString::null , "*", 0L, i18n ( "Kopete File Transfer" ) );
	else
		filePath = sourceURL.path(-1);

	QFile file ( filePath );

	if ( file.exists () )
	{
		// send the file
		new JabberFileTransfer ( account (), this, filePath );
	}

}

void JabberGroupMemberContact::slotUserInfo ()
{

}

#include "jabbergroupmembercontact.moc"
