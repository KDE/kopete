/*
    addressbooklink.cpp - Manages operations involving the KDE Address Book

    Copyright (c) 2005 Will Stephenson <lists@stevello.free-online.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qstring.h>
#include <qtimer.h>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kabc/resource.h>
#include <kabc/stdaddressbook.h>

// UI related includes used for importing from KABC
#include <kdialogbase.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "accountselector.h"
#include "kopeteuiglobal.h"

#include <kstaticdeleter.h>

#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"

#include "kabcpersistence.h"

namespace Kopete
{
	
/**
 * utility function to merge two QStrings containing individual elements separated by 0xE000
 */
static QString unionContents( QString arg1, QString arg2 )
{
	QChar separator( 0xE000 );
	QStringList outList = QStringList::split( separator, arg1 );
	QStringList arg2List = QStringList::split( separator, arg2 );
	for ( QStringList::iterator it = arg2List.begin(); it != arg2List.end(); ++it )
		if ( !outList.contains( *it ) )
			outList.append( *it );
	QString out = outList.join( separator );
	return out;
}

KABCPersistence::KABCPersistence( QObject * parent, const char * name ) : QObject( parent, name )
{
	s_pendingResources.setAutoDelete( false );
}

KABCPersistence::~KABCPersistence()
{
}

KABCPersistence *KABCPersistence::s_self = 0L;

bool KABCPersistence::s_addrBookWritePending = false;

QPtrList<KABC::Resource> KABCPersistence::s_pendingResources;

KABC::AddressBook* KABCPersistence::s_addressBook = 0;

KABCPersistence *KABCPersistence::self()
{
	static KStaticDeleter<KABCPersistence> deleter;
	if(!s_self)
		deleter.setObject( s_self, new KABCPersistence() );
	return s_self;	
}

KABC::AddressBook* KABCPersistence::addressBook()
{
	if ( s_addressBook == 0L )
	{
		s_addressBook = KABC::StdAddressBook::self();
		KABC::StdAddressBook::setAutomaticSave( false );
	}
	return s_addressBook;
}

void KABCPersistence::write( MetaContact * mc )
{
	// Save any changes in each contact's addressBookFields to KABC
	KABC::AddressBook* ab = addressBook();

	kdDebug( 14010 ) << k_funcinfo << "looking up Addressee for " << mc->displayName() << "..." << endl;
	// Look up the address book entry
	KABC::Addressee theAddressee = ab->findByUid( mc->metaContactId() );
	// Check that if addressee is not deleted or if the link is spurious
	// (inherited from Kopete < 0.8, where all metacontacts had random ids)
	if ( theAddressee.isEmpty() )
	{
		// not found in currently enabled addressbooks - may be in a disabled resource...
		return;
	}
	else
	{
		// collate the instant messaging data to be inserted into the address book
		QMap<QString, QStringList> addressMap;
		QPtrList<Contact> contacts = mc->contacts();
		QPtrListIterator<Contact> cIt( contacts );
		while ( Contact * c = cIt.current() )
		{
			QStringList addresses = addressMap[ c->protocol()->addressBookIndexField() ];
			addresses.append( c->contactId() );
			addressMap.insert( c->protocol()->addressBookIndexField(), addresses );
			++cIt;
		}
		
		// insert a custom field for each protocol
		QMap<QString, QStringList>::ConstIterator it = addressMap.begin();
		for ( ; it != addressMap.end(); ++it )
		{
			// read existing data for this key
			QString currentCustomForProtocol = theAddressee.custom( it.key(), QString::fromLatin1( "All" ) );
			// merge without duplicating
			QString toWrite = unionContents( currentCustomForProtocol, it.data().join( QChar( 0xE000 ) ) );
			// Note if nothing ends up in the KABC data, this is because insertCustom does nothing if any param is empty.
			kdDebug( 14010 ) << k_funcinfo << "Writing: " << it.key() << ", " << "All" << ", " << toWrite << endl;
			theAddressee.insertCustom( it.key(), QString::fromLatin1( "All" ), toWrite );
			QString check = theAddressee.custom( it.key(), QString::fromLatin1( "All" ) );
		}
		ab->insertAddressee( theAddressee );
		//kdDebug( 14010 ) << k_funcinfo << "dumping addressbook before write " << endl;
		//dumpAB();
		writeAddressBook( theAddressee.resource() );
		//theAddressee.dump();
	}
	
/*			// Wipe out the existing addressBook entries
			d->addressBook.clear();
	// This causes each Kopete::Protocol subclass to serialise its contacts' data into the metacontact's plugin data and address book data
			emit aboutToSave(this);

			kdDebug( 14010 ) << k_funcinfo << "...FOUND ONE!" << endl;
	// Store address book fields
			QMap<QString, QMap<QString, QString> >::ConstIterator appIt = d->addressBook.begin();
			for( ; appIt != d->addressBook.end(); ++appIt )
			{
				QMap<QString, QString>::ConstIterator addrIt = appIt.data().begin();
				for( ; addrIt != appIt.data().end(); ++addrIt )
				{
			// read existing data for this key
					QString currentCustom = theAddressee.custom( appIt.key(), addrIt.key() );
			// merge without duplicating
					QString toWrite = unionContents( currentCustom, addrIt.data() );
			// write the result
			// Note if nothing ends up in the KABC data, this is because insertCustom does nothing if any param is empty.
					kdDebug( 14010 ) << k_funcinfo << "Writing: " << appIt.key() << ", " << addrIt.key() << ", " << toWrite << endl;
					theAddressee.insertCustom( appIt.key(), addrIt.key(), toWrite );
				}
			}
			ab->insertAddressee( theAddressee );
			writeAddressBook();
		}*/
}

void KABCPersistence::writeAddressBook( const KABC::Resource * res)
{
	if ( !s_pendingResources.containsRef( res ) )
		s_pendingResources.append( res );
	if ( !s_addrBookWritePending )
	{
		s_addrBookWritePending = true;
		QTimer::singleShot( 2000, this, SLOT( slotWriteAddressBook() ) );
	}
}

void KABCPersistence::slotWriteAddressBook()
{
	//kdDebug(  14010 ) << k_funcinfo << endl;
	KABC::AddressBook* ab = addressBook();
	QPtrListIterator<KABC::Resource> it( s_pendingResources );
	for ( ; it.current(); ++it )
	{
		//kdDebug(  14010 )  << "Writing resource " << it.current()->resourceName() << endl;
		KABC::Ticket *ticket = ab->requestSaveTicket( it.current() );
		if ( !ticket )
			kdWarning( 14010 ) << "WARNING: Resource is locked by other application!" << endl;
		else
		{
			if ( !ab->save( ticket ) )
			{
				kdWarning( 14010 ) << "ERROR: Saving failed!" << endl;
				ab->releaseSaveTicket( ticket );
			}
		}
		//kdDebug( 14010 ) << "Finished writing KABC" << endl;
	}
	s_pendingResources.clear();
	s_addrBookWritePending = false;
}

void KABCPersistence::removeKABC( MetaContact *)
{
/*	// remove any data this KMC has written to the KDE address book
	// Save any changes in each contact's addressBookFields to KABC
	KABC::AddressBook* ab = addressBook();

	// Wipe out the existing addressBook entries
	d->addressBook.clear();
	// This causes each Kopete::Protocol subclass to serialise its contacts' data into the metacontact's plugin data and address book data
	emit aboutToSave(this);

	// If the metacontact is linked to a kabc entry
	if ( !d->metaContactId.isEmpty() )
	{
		//kdDebug( 14010 ) << k_funcinfo << "looking up Addressee for " << displayName() << "..." << endl;
		// Look up the address book entry
		KABC::Addressee theAddressee = ab->findByUid( metaContactId() );

		if ( theAddressee.isEmpty() )
		{
			// remove the link
			//kdDebug( 14010 ) << k_funcinfo << "...not found." << endl;
			d->metaContactId=QString::null;
		}
		else
		{
			//kdDebug( 14010 ) << k_funcinfo << "...FOUND ONE!" << endl;
			// Remove address book fields
			QMap<QString, QMap<QString, QString> >::ConstIterator appIt = d->addressBook.begin();
			for( ; appIt != d->addressBook.end(); ++appIt )
			{
				QMap<QString, QString>::ConstIterator addrIt = appIt.data().begin();
				for( ; addrIt != appIt.data().end(); ++addrIt )
				{
					// FIXME: This assumes Kopete is the only app writing these fields
					kdDebug( 14010 ) << k_funcinfo << "Removing: " << appIt.key() << ", " << addrIt.key() << endl;
					theAddressee.removeCustom( appIt.key(), addrIt.key() );
				}
			}
			ab->insertAddressee( theAddressee );

			writeAddressBook();
		}
	}
//	kdDebug(14010) << k_funcinfo << kdBacktrace() <<endl;*/
}

bool KABCPersistence::syncWithKABC( MetaContact * mc )
{
	kdDebug(14010) << k_funcinfo << endl;
	bool contactAdded = false;
	// check whether the dontShowAgain was checked
		KABC::AddressBook* ab = addressBook();
		KABC::Addressee addr  = ab->findByUid( mc->metaContactId() );
		
		if ( !addr.isEmpty() ) // if we are associated with KABC
		{
// load the set of addresses from KABC
		QStringList customs = addr.customs();

		QStringList::ConstIterator it;
		for ( it = customs.begin(); it != customs.end(); ++it )
		{
			QString app, name, value;
			splitField( *it, app, name, value );
			kdDebug( 14010 ) << "app=" << app << " name=" << name << " value=" << value << endl;

			if ( app.startsWith( QString::fromLatin1( "messaging/" ) ) )
			{
				if ( name == QString::fromLatin1( "All" ) )
				{
					kdDebug( 14010 ) << " syncing \"" << app << ":" << name << " with contactlist " << endl;
					// Get the protocol name from the custom field
					// by chopping the 'messaging/' prefix from the custom field app name
					QString protocolName = app.right( app.length() - 10 );
					// munge Jabber hack
					if ( protocolName == QString::fromLatin1( "xmpp" ) )
						protocolName = QString::fromLatin1( "jabber" );

					// Check Kopete supports it
					Protocol * proto = dynamic_cast<Protocol*>( PluginManager::self()->loadPlugin( QString::fromLatin1( "kopete_" ) + protocolName ) );
					if ( !proto )
					{
						KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
																					 i18n( "<qt>\"%1\" is not supported by Kopete.</qt>" ).arg( protocolName ),
																					 i18n( "Could Not Sync with KDE Address Book" )  );
						continue;
					}

					// See if we need to add each contact in this protocol
					QStringList addresses = QStringList::split( QChar( 0xE000 ), value );
					QStringList::iterator end = addresses.end();
					for ( QStringList::iterator it = addresses.begin(); it != end; ++it )
					{
						// check whether each one is present in Kopete
						// Is it in the contact list?
						// First discard anything after an 0xE120, this is used by IRC to separate nick and server group name, but
						// IRC doesn't support this properly yet, so the user will have to select an appropriate account manually
						int separatorPos = (*it).find( QChar( 0xE120 ) );
						if ( separatorPos != -1 )
							*it = (*it).left( separatorPos );

						QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts( proto );
						QDictIterator<Kopete::Account> acs(accounts);
						Kopete::MetaContact *otherMc = 0;
						for ( acs.toFirst(); acs.current(); ++acs )
						{
							Kopete::Contact *c= acs.current()->contacts()[*it];
							if(c)
							{
								otherMc=c->metaContact();
								break;
							}
						}

						if ( otherMc ) // Is it in another metacontact?
						{
							// Is it already in this metacontact? If so, we needn't do anything
							if ( otherMc == mc )
							{
								kdDebug( 14010 ) << *it << " already a child of this metacontact." << endl;
								continue;
							}
							kdDebug( 14010 ) << *it << " already exists in OTHER metacontact, move here?" << endl;
							// find the Kopete::Contact and attempt to move it to this metacontact.
							otherMc->findContact( proto->pluginId(), QString::null, *it )->setMetaContact( mc );
						}
						else
						{
							// if not, prompt to add it
							kdDebug( 14010 ) << proto->pluginId() << "://" << *it << " was not found in the contact list.  Prompting to add..." << endl;
							if ( KMessageBox::Yes == KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(),
									 i18n( "<qt>An address was added to this contact by another application.<br>Would you like to use it in Kopete?<br><b>Protocol:</b> %1<br><b>Address:</b> %2</qt>" ).arg( proto->displayName() ).arg( *it ), i18n( "Import Address From Address Book" ), i18n("Use"), i18n("Do Not Use"), QString::fromLatin1( "ImportFromKABC" ) ) )
							{
								// Check the accounts for this protocol are all connected
								// Most protocols do not allow you to add contacts while offline
								// Would be better to have a virtual bool Kopete::Account::readyToAddContact()
								bool allAccountsConnected = true;
								for ( acs.toFirst(); acs.current(); ++acs )
									if ( !acs.current()->isConnected() )
								{	allAccountsConnected = false;
								break;
								}
								if ( !allAccountsConnected )
								{
									KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
											i18n( "<qt>One or more of your accounts using %1 are offline.  Most systems have to be connected to add contacts.  Please connect these accounts and try again.</qt>" ).arg( protocolName ),
											i18n( "Not Connected" )  );
									continue;
								}

								// we have got a contact to add, our accounts are connected, so add it.
								// Do we need to choose an account
								Kopete::Account *chosen = 0;
								if ( accounts.count() > 1 )
								{	// if we have >1 account in this protocol, prompt for the protocol.
									KDialogBase *chooser = new KDialogBase(0, "chooser", true,
											i18n("Choose Account"), KDialogBase::Ok|KDialogBase::Cancel,
											KDialogBase::Ok, false);
									AccountSelector *accSelector = new AccountSelector(proto, chooser,
											"accSelector");
									chooser->setMainWidget(accSelector);
									if ( chooser->exec() == QDialog::Rejected )
										continue;
									chosen = accSelector->selectedItem();

									delete chooser;
								}
								else if ( accounts.isEmpty() )
								{
									KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
											i18n( "<qt>You do not have an account configured for <b>%1</b> yet.  Please create an account, connect it, and try again.</qt>" ).arg( protocolName ),
											i18n( "No Account Found" )  );
									continue;
								}
								else // if we have 1 account in this protocol, choose it
								{
									chosen = acs.toFirst();
								}

								// add the contact to the chosen account
								if ( chosen )
								{
									kdDebug( 14010 ) << "Adding " << *it << " to " << chosen->accountId() << endl;
									if ( chosen->addContact( *it, mc ) )
										contactAdded = true;
									else
										KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
											i18n( "<qt>It was not possible to add the contact.</qt>" ),
											i18n( "Could Not Add Contact") ) ;
								}
							}
							else
								kdDebug( 14010 ) << " user declined to add " << *it << " to contactlist " << endl;
						}
					}
					kdDebug( 14010 ) << " all " << addresses.count() << " contacts in " << proto->pluginId() << " checked " << endl;
				}
				else
					kdDebug( 14010 ) << "not interested in name=" << name << endl;

			}
			else
				kdDebug( 14010 ) << "not interested in app=" << app << endl;
		}
	}
	return contactAdded;
	return false;
}

// FIXME: Remove when IM address API is in KABC (KDE 4)
void KABCPersistence::splitField( const QString &str, QString &app, QString &name, QString &value )
{
	int colon = str.find( ':' );
	if ( colon != -1 ) {
		QString tmp = str.left( colon );
		value = str.mid( colon + 1 );

		int dash = tmp.find( '-' );
		if ( dash != -1 ) {
			app = tmp.left( dash );
			name = tmp.mid( dash + 1 );
		}
	}
}

void KABCPersistence::dumpAB()
{
	KABC::AddressBook * ab = addressBook();
	kdDebug( 14010 ) << k_funcinfo << " DUMPING ADDRESSBOOK" << endl;
	KABC::AddressBook::ConstIterator dumpit = ab->begin();
	for ( ; dumpit != ab->end(); ++dumpit )
	{
		(*dumpit).dump();
	}
}


} // end namespace Kopete

		// dump addressbook contents 

#include "kabcpersistence.moc"
