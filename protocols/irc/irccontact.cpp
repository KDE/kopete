/***************************************************************************
                          irccontact.cpp  -  description
                             -------------------
    begin                : Thu Feb 20 2003
    copyright            : (C) 2003 by nbetcher
    email                : nbetcher@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <qstringlist.h>
#include <qregexp.h>

#include "ircchannelcontact.h"
#include "ircidentity.h"

#include "kirc.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "irccontact.h"

IRCContact::IRCContact(IRCIdentity *identity, KopeteMetaContact *metac) :
	KopeteContact((KopeteProtocol *)identity->protocol(), identity->mySelf()->nickName(), metac )
{
	mIdentity = identity;
	mEngine = mIdentity->engine();
	mMetaContact = metac;
	mMsgManager = 0L;
}

bool IRCContact::processMessage( const KopeteMessage &msg )
{
	QStringList commandLine = QStringList::split( QRegExp( QString::fromLatin1("\\s+") ), msg.plainBody() );
	QString commandArgs = msg.plainBody().section( QRegExp( QString::fromLatin1("\\s+") ), 1 );

	if( commandLine.first().startsWith( QString::fromLatin1("/") ) )
	{
		QString command = commandLine.first().right( commandLine.first().length() - 1 );

		if( mEngine->isLoggedIn() )
		{
			// These commands only work when we are connected
			if( command == QString::fromLatin1("nick") && commandLine.count() > 1 )
				mIdentity->successfullyChangedNick( QString::null, *commandLine.at(1) );

			else if( command == QString::fromLatin1("me") && commandLine.count() > 1 )
				mEngine->actionContact( displayName(), commandArgs );
		}

		// A /command returns false to stop further processing
		return false;
	}

	return true;
}
