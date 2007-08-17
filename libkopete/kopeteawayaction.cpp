/*
    kopeteaway.cpp  -  Kopete Away Action

    Copyright (c) 2003     Jason Keirstead   <jason@keirstead.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kinputdialog.h>
#include <kstringhandler.h>
#include <kicon.h>

#include "kopeteawayaction.h"
#include "kopeteaway.h"
#include "kopeteonlinestatus.h"
#include "kopetestatusmessage.h"
#include <kactioncollection.h>


namespace Kopete {

class AwayAction::Private
{
public:
	Private(const OnlineStatus& s) : reasonCount(0) , status(s) {};
	int reasonCount;
	OnlineStatus status;
};


AwayAction::AwayAction( const OnlineStatus& status, const QString &text, const QIcon &pix, const KShortcut &cut,
					   const QObject *receiver, const char *slot )
	: KSelectAction( KIcon(pix), text, 0 ) , d(new Private( status ) )
{
	setShortcut( cut );

	QObject::connect( Kopete::Away::getInstance(), SIGNAL( messagesChanged() ),
					  this, SLOT( slotAwayChanged() ) );

	QObject::connect( this, SIGNAL( awayMessageSelected( const Kopete::OnlineStatus &, const Kopete::StatusMessage & ) ),
					  receiver, slot );

	QObject::connect( this, SIGNAL( triggered( int ) ),
					  this, SLOT( slotSelectAway( int ) ) );

	slotAwayChanged();
}

AwayAction::~AwayAction()
{
	delete d;
}

void AwayAction::slotAwayChanged()
{
	QStringList awayMessages = Kopete::Away::getInstance()->getMessages();
	for( QStringList::iterator it = awayMessages.begin(); it != awayMessages.end(); ++it )
	{
		(*it) = KStringHandler::rsqueeze( *it );
	}
	d->reasonCount = awayMessages.count();
	QStringList menu;
	menu << i18n( "No Message" );
	menu << i18n( "New Message..." );
	menu << QString() ;  //separator
	menu += awayMessages ;
	setItems( menu );
	setCurrentItem( -1 );
}

void AwayAction::slotSelectAway( int index )
{
	//remove that crappy check mark  cf bug 119862
	setCurrentItem( -1 );

	Kopete::Away *mAway = Kopete::Away::getInstance();
	QString awayReason;

	// Index == -1 means this is a result of Global Away all.
	// Use the last entered message (0)
	if( index == -1 )
		index = 0;

	switch(index)
	{
		case 0:
			awayReason.clear();
			break;
		case 1:
			bool ok;
			awayReason = KInputDialog::getText( i18n( "New Away Message" ), i18n( "Please enter your away reason:" ) , QString() , &ok );
			if(!ok) //the user canceled
				return;
			if( !awayReason.isEmpty() )
				Kopete::Away::getInstance()->addMessage( awayReason );
			break;
		case 2:
			//not possible case, that's a separator
			break;
		default:
			if( index-3 < d->reasonCount )
				awayReason = mAway->getMessage( index-3 );
	}

	emit awayMessageSelected( awayReason ) ;
	emit awayMessageSelected( d->status, Kopete::StatusMessage(awayReason) );
}

} //END namespace Kopete

#include "kopeteawayaction.moc"

