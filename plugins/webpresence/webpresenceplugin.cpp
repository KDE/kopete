/*
    webpresenceplugin.cpp

    Kopete Web Presence plugin

    Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                    	*
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qtimer.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <ktempfile.h>
#include <kio/job.h>
#include <knotifyclient.h>
#include <kaction.h>
#include <kstdguiitem.h>

#include "kopetecontactlist.h"
#include "pluginloader.h"
#include "kopeteprotocol.h"
#include "kopetemetacontact.h"

#include "webpresenceplugin.h"
#include "webpresencepreferences.h"

K_EXPORT_COMPONENT_FACTORY( kopete_webpresence, KGenericFactory<WebPresencePlugin> );

WebPresencePlugin::WebPresencePlugin( QObject *parent, const char *name, const QStringList& /*args*/ )
	: KopetePlugin( parent, name )
{
	m_actionCollection = 0L;
	m_actionWantsAdvert = 0L;
	m_currentMetaContact = 0L;
	m_prefsDialog = new WebPresencePreferences( "", this );
	connect ( m_prefsDialog, SIGNAL( saved() ), this, SLOT( slotSettingsChanged() ) );
	m_timer = new QTimer();
	connect ( m_timer, SIGNAL( timeout() ), this, SLOT( slotWriteFile() ) );
	m_timer->start( m_prefsDialog->frequency() * 1000 * 60);
}

WebPresencePlugin::~WebPresencePlugin()
{}

KActionCollection *WebPresencePlugin::customContextMenuActions(  KopeteMetaContact *m )
{
	kdDebug() << "WebPresencePlugin::customContextMenuActions() - for " << 
		m->displayName() << endl;
	delete m_actionCollection;
	m_actionCollection = new KActionCollection( this );
	m_actionWantsAdvert = new KToggleAction( i18n( "Web Presence" ), 0, this, 
			SLOT( slotContactWantsToggled() ), m_actionCollection, 
			"actionWantsAdvert" );
	if ( !m_prefsDialog->showMyContacts() )
	{
		m_actionWantsAdvert->setEnabled( false );
	}
	m_actionWantsAdvert->setChecked( m->pluginData( this ).first() == "true" );
	m_actionCollection->insert( m_actionWantsAdvert );
	m_currentMetaContact = m;
	return m_actionCollection;
}

void WebPresencePlugin::slotContactWantsToggled()
{
	kdDebug() << "WebPresencePlugin::slotContactsWantsToggled()" << endl;
	if (  m_actionWantsAdvert && m_currentMetaContact )
	{
		m_currentMetaContact->setPluginData(  this, ( m_actionWantsAdvert->isChecked()
					? "true" : "false" ) );
	}
	return;
}

void WebPresencePlugin::slotWriteFile()
{
	// generate the (temporary) file representing the current contactlist
	KTempFile* theFile = generateFile();
	theFile->setAutoDelete( false );
	kdDebug() << "WebPresencePlugin::slotWriteFile() : " << theFile->name() << endl;
	// upload it to the specified URL
	KURL src( theFile->name() );
	KURL dest( m_prefsDialog->url() );
	KIO::FileCopyJob *job = KIO::file_copy( src, dest, -1, true, false, false );
	connect(  job, SIGNAL(  result(  KIO::Job * ) ),
			SLOT(  slotUploadJobResult(  KIO::Job * ) ) );
	// get rid of the local file
	delete theFile;
}

void WebPresencePlugin::slotUploadJobResult( KIO::Job *job )
{
	if (  job->error() ) {
		kdDebug() << "Error uploading presence info." << endl;
		job->showErrorDialog( 0 );
		return;
	}
}

KTempFile* WebPresencePlugin::generateFile()
{
	// generate the (temporary) file representing the current contactlist
	KTempFile* theFile = new KTempFile();
	QTextStream* qout =  theFile->textStream() ;

	KopeteContactList* theList = KopeteContactList::contactList();
	QPtrList<KopeteMetaContact> metaContacts( theList->metaContacts() );
	
	if ( !m_prefsDialog->uploadHtml() )
	{
		// write a text file
		// first line indicates if protocol addresses are included
		if ( m_prefsDialog->showAddresses() )
			*qout << "WITHADDRESSES" << endl;
		// line beginning with MYSTATUS introduces your own info
		if ( m_prefsDialog->showMyself() )
		{
			*qout << "MYSTATUS" << endl ;
			QPtrList<WebPresencePlugin::ProtoContactStatus> *states = myStatus();
			for ( ProtoContactStatus *p = states->first(); p; p = states->next() )
			{
				//kdDebug() << p->name << " : " << p->status << " : " << p->id << endl;
				*qout << p->name << endl << statusAsString( p->status ) << endl;
				if ( m_prefsDialog->showAddresses() )
					*qout << p->id << endl;
			}
		}
		// line beginning with NEXTCONTACT introduces your contacts' info
		if ( m_prefsDialog->showMyContacts() )
		{
			for ( KopeteMetaContact* i = metaContacts.first(); i; i = metaContacts.next() )
			{
				// should we include this contact?
				if ( i->pluginData( this ).first() == "true" )
				{
					*qout << "NEXTCONTACT" << endl;
					*qout << i->displayName() << endl;
					QPtrList< KopeteContact > itsContacts = i->contacts ();
					for ( KopeteContact *j = itsContacts.first(); j; j = itsContacts.next() )
					{
						*qout << j->protocol()->pluginId() << endl
							<< statusAsString( j->status() ) << endl;
						if ( m_prefsDialog->showAddresses() )
							*qout << j->contactId() << endl;
					}
				}
			}
		}
	}
	else
	{
		// write some very ugly HTMl.  NB This isn't meant to be a 
		// standalone page, hopefully just include it into another page
		// server side
		if ( m_prefsDialog->showMyself() )
		{
			// generate table showing MY status
			*qout << "<table>\n  <tr>\n    <td>" << i18n( "My Status:" ) << 
				"</td>\n    <td><table>";
			QPtrList<WebPresencePlugin::ProtoContactStatus> *states = myStatus();
			for ( ProtoContactStatus *p = states->first(); p; p = states->next() )
			{
				*qout << "<tr><td>" << p->name << "<td>" << statusAsString( p->status );
				if ( m_prefsDialog->showAddresses() )
					*qout << "<td>" << p->id;
				*qout << "</tr>" << endl;
			}
			*qout << "</table>\n  </tr>\n</table>" << endl;
		}
		if ( m_prefsDialog->showMyContacts() )
		{
			// generate table showing CONTACTS STATUS
			*qout << "<table>\n  <tr>\n    <td>" << i18n( "My Contacts' Status:" ) <<
				" </td>\n  </tr>";
			for ( KopeteMetaContact* i = metaContacts.first(); i; i = metaContacts.next() )
			{
				// should we include this contact?
				if ( i->pluginData( this ).first() == "true" )
				{
					*qout << "  <tr><td>" << i->displayName() << "<td><table>" << endl;
					QPtrList< KopeteContact > itsContacts = i->contacts ();
					for ( KopeteContact *j = itsContacts.first(); j; 
							j = itsContacts.next() )
					{
						*qout << "<tr><td>" << j->protocol()->pluginId() << "<td>"
							<< statusAsString( j->status() );
						if ( m_prefsDialog->showAddresses() )
							*qout << "<td>" << j->contactId();
						*qout << "</tr>" << endl;
					}
					*qout << "</table>" << endl;
				}
			}
			*qout << "\n  </tr>\n</table>" << endl;
		}
	}
	theFile->close();
	return theFile;
}

QPtrList<WebPresencePlugin::ProtoContactStatus> *WebPresencePlugin::myStatus()
{
	QPtrList<WebPresencePlugin::ProtoContactStatus> *myStates = new QPtrList<ProtoContactStatus>;
	QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();
	for(  KopetePlugin *p = plugins.first() ; p ; p = plugins.next() )
	{
		KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>(  p );
		if(  !proto )
			continue;
		ProtoContactStatus *p = new ProtoContactStatus;
		p->name = proto->pluginId();
		p->id = proto->myself()->contactId().latin1();
		p->status = proto->myself()->status();
		myStates->append( p );
	}
	return myStates;
}

QString WebPresencePlugin::statusAsString( KopeteContact::ContactStatus c )
{
	QString status;
	switch ( c )
	{
		case KopeteContact::Online:
			status = i18n( "ONLINE" );
			break;
		case KopeteContact::Away:
			status = i18n( "AWAY" );
			break;
		case KopeteContact::Offline:
			status = i18n( "OFFLINE" );
			break;
		default:
			status = i18n( "UNKNOWN" );
	}
	return status;
}
	
void WebPresencePlugin::slotSettingsChanged()
{
	m_timer->start( m_prefsDialog->frequency() * 1000 * 60);
}

//#include "webpresenceplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:
