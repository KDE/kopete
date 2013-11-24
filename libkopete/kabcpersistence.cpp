/*
    addressbooklink.cpp - Manages operations involving the KDE Address Book

    Copyright (c) 2005 Will Stephenson <wstephenson@kde.org>

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

#include "kabcpersistence.h"

#include <qstring.h>
#include <qtimer.h>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kabc/resource.h>
#include <kabc/stdaddressbook.h>

// UI related includes used for importing from KABC
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "accountselector.h"
#include "kopeteuiglobal.h"

#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"

namespace Kopete
{

/**
 * utility function to merge two QStrings containing individual elements separated by 0xE000
 */
static QString unionContents( const QString& arg1, const QString& arg2 )
{
	const QChar separator( 0xE000 );
	QStringList outList = arg1.split( separator, QString::SkipEmptyParts );
	const QStringList arg2List = arg2.split( separator, QString::SkipEmptyParts );
	for ( QStringList::ConstIterator it = arg2List.constBegin(); it != arg2List.constEnd(); ++it )
		if ( !outList.contains( *it ) )
			outList.append( *it );
	const QString out = outList.join( QString( separator ) );
	return out;
}

class KABCPersistence::Private
{
public:
	Private() 
	 : addrBookWritePending(false)
	{}
	QList<KABC::Resource *> pendingResources;
	bool addrBookWritePending;

	// FIXME: Try to remove that static variable !
	static KABC::AddressBook* s_addressBook;
};

KABC::AddressBook* KABCPersistence::Private::s_addressBook = 0L;

KABCPersistence::KABCPersistence( QObject * parent, const char * name )
  : QObject( parent), d(new Private())
{
	setObjectName( name );
}

KABCPersistence::~KABCPersistence()
{
	delete d;
}

KABCPersistence *KABCPersistence::self()
{
	static KABCPersistence s;
	return &s;
}

KABC::AddressBook* KABCPersistence::addressBook()
{
	if ( Private::s_addressBook == 0L )
	{
		Private::s_addressBook = KABC::StdAddressBook::self();
		KABC::StdAddressBook::setAutomaticSave( false );
	}
	return Private::s_addressBook;
}

void KABCPersistence::write( MetaContact * mc )
{
	// Save any changes in each contact's addressBookFields to KABC
	KABC::AddressBook* ab = addressBook();

	kDebug( 14010 ) << "looking up Addressee for " << mc->displayName() << "...";
	// Look up the address book entry
	KABC::Addressee theAddressee = ab->findByUid( mc->kabcId() );
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
		QList<Contact *> contacts = mc->contacts();
		QListIterator<Contact *> cIt( contacts );
		while ( cIt.hasNext() )
		{
			Contact * c = cIt.next();
			QStringList addresses = addressMap[ c->protocol()->addressBookIndexField() ];
			addresses.append( c->contactId() );
			addressMap.insert( c->protocol()->addressBookIndexField(), addresses );
		}

		// insert a custom field for each protocol
		QMap<QString, QStringList>::ConstIterator it = addressMap.constBegin();
		for ( ; it != addressMap.constEnd(); ++it )
		{
			// read existing data for this key
			const QString currentCustomForProtocol = theAddressee.custom( it.key(), QLatin1String( "All" ) );
			// merge without duplicating
			const QString toWrite = unionContents( currentCustomForProtocol, it.value().join( QString( QChar( 0xE000 ) ) ) );
			// Note if nothing ends up in the KABC data, this is because insertCustom does nothing if any param is empty.
			kDebug( 14010 ) << "Writing: " << it.key() << ", " << "All" << ", " << toWrite;
			theAddressee.insertCustom( it.key(), QLatin1String( "All" ), toWrite );
			const QString check = theAddressee.custom( it.key(), QLatin1String( "All" ) );
		}
		ab->insertAddressee( theAddressee );
		writeAddressBook( theAddressee.resource() );
		//theAddressee.dump();
	}

/*			// Wipe out the existing addressBook entries
			d->addressBook.clear();
	// This causes each Kopete::Protocol subclass to serialise its contacts' data into the metacontact's plugin data and address book data
			emit aboutToSave(this);

			kDebug( 14010 ) << "...FOUND ONE!";
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
					kDebug( 14010 ) << "Writing: " << appIt.key() << ", " << addrIt.key() << ", " << toWrite;
					theAddressee.insertCustom( appIt.key(), addrIt.key(), toWrite );
				}
			}
			ab->insertAddressee( theAddressee );
			writeAddressBook();
		}*/
}

void KABCPersistence::writeAddressBook( KABC::Resource * res)
{
	if ( !d->pendingResources.count( res ) )
		d->pendingResources.append( res );
	if ( !d->addrBookWritePending )
	{
		d->addrBookWritePending = true;
		QTimer::singleShot( 2000, this, SLOT(slotWriteAddressBook()) );
	}
}

void KABCPersistence::slotWriteAddressBook()
{
	//kDebug(  14010 ) ;
	KABC::AddressBook* ab = addressBook();
	QListIterator<KABC::Resource *> it( d->pendingResources );
	while ( it.hasNext() )
	{
		//kDebug(  14010 )  << "Writing resource " << it.current()->resourceName();
		KABC::Ticket *ticket = ab->requestSaveTicket( it.next() );
		if ( !ticket )
			kWarning( 14010 ) << "WARNING: Resource is locked by other application!";
		else
		{
			if ( !ab->save( ticket ) )
			{
				kWarning( 14010 ) << "ERROR: Saving failed!";
				ab->releaseSaveTicket( ticket );
			}
		}
		//kDebug( 14010 ) << "Finished writing KABC";
	}
	d->pendingResources.clear();
	d->addrBookWritePending = false;
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
	if ( !d->kabcId().isEmpty() )
	{
		//kDebug( 14010 ) << "looking up Addressee for " << displayName() << "...";
		// Look up the address book entry
		KABC::Addressee theAddressee = ab->findByUid( d->kabcId() );

		if ( theAddressee.isEmpty() )
		{
			// remove the link
			//kDebug( 14010 ) << "...not found.";
			d->kabcId.clear();
		}
		else
		{
			//kDebug( 14010 ) << "...FOUND ONE!";
			// Remove address book fields
			QMap<QString, QMap<QString, QString> >::ConstIterator appIt = d->addressBook.begin();
			for( ; appIt != d->addressBook.end(); ++appIt )
			{
				QMap<QString, QString>::ConstIterator addrIt = appIt.data().begin();
				for( ; addrIt != appIt.data().end(); ++addrIt )
				{
					// FIXME: This assumes Kopete is the only app writing these fields
					kDebug( 14010 ) << "Removing: " << appIt.key() << ", " << addrIt.key();
					theAddressee.removeCustom( appIt.key(), addrIt.key() );
				}
			}
			ab->insertAddressee( theAddressee );

			writeAddressBook();
		}
	}
//	kDebug(14010) << kBacktrace();*/
}

bool KABCPersistence::syncWithKABC( MetaContact * mc )
{
	kDebug(14010) ;
	bool contactAdded = false;
	// check whether the dontShowAgain was checked
		KABC::AddressBook* ab = addressBook();
		KABC::Addressee addr  = ab->findByUid( mc->kabcId() );

		if ( !addr.isEmpty() ) // if we are associated with KABC
		{
// load the set of addresses from KABC
		const QStringList customs = addr.customs();

		QStringList::ConstIterator it;
		for ( it = customs.constBegin(); it != customs.constEnd(); ++it )
		{
			QString app, name, value;
			splitField( *it, app, name, value );
			kDebug( 14010 ) << "app=" << app << " name=" << name << " value=" << value;

			if ( app.startsWith( QLatin1String( "messaging/" ) ) )
			{
				if ( name == QLatin1String( "All" ) )
				{
					kDebug( 14010 ) << " syncing \"" << app << ":" << name << " with contact list ";
					// Get the protocol name from the custom field
					// by chopping the 'messaging/' prefix from the custom field app name
					QString protocolName = app.right( app.length() - 10 );
					// munge Jabber hack
					if ( protocolName == QLatin1String( "xmpp" ) )
						protocolName = QLatin1String( "jabber" );

					// Check Kopete supports it
					Protocol * proto = dynamic_cast<Protocol*>( PluginManager::self()->loadPlugin( QLatin1String( "kopete_" ) + protocolName ) );
					if ( !proto )
					{
						KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
																					 i18n( "<qt>\"%1\" is not supported by Kopete.</qt>", protocolName ),
																					 i18n( "Could Not Sync with KDE Address Book" )  );
						continue;
					}

					// See if we need to add each contact in this protocol
					QStringList addresses = value.split( QChar( 0xE000 ), QString::SkipEmptyParts );
					QStringList::iterator end = addresses.end();
					for ( QStringList::iterator it = addresses.begin(); it != end; ++it )
					{
						// check whether each one is present in Kopete
						// Is it in the contact list?
						// First discard anything after an 0xE120, this is used by IRC to separate nick and server group name, but
						// IRC doesn't support this properly yet, so the user will have to select an appropriate account manually
						int separatorPos = (*it).indexOf( QChar( 0xE120 ) );
						if ( separatorPos != -1 )
							*it = (*it).left( separatorPos );

						Kopete::MetaContact *otherMc = 0;
						foreach( Kopete::Account *act, Kopete::AccountManager::self()->accounts() )
						{
							if( act->protocol() != proto )
								continue;
							Kopete::Contact *c= act->contacts().value(*it);
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
								kDebug( 14010 ) << *it << " already a child of this metacontact.";
								continue;
							}
							kDebug( 14010 ) << *it << " already exists in OTHER metacontact, move here?";
							// find the Kopete::Contact and attempt to move it to this metacontact.
							otherMc->findContact( proto->pluginId(), QString(), *it )->setMetaContact( mc );
						}
						else
						{
							// if not, prompt to add it
							kDebug( 14010 ) << proto->pluginId() << "://" << *it << " was not found in the contact list.  Prompting to add...";
							if ( KMessageBox::Yes == KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(),
									 i18n( "<qt>An address was added to this contact by another application.<br />Would you like to use it in Kopete?<br /><b>Protocol:</b> %1<br /><b>Address:</b> %2</qt>", proto->displayName(), *it ), i18n( "Import Address From Address Book" ), KGuiItem( i18n("Use") ), KGuiItem( i18n("Do Not Use") ), QLatin1String( "ImportFromKABC" ) ) )
							{
								// Check the accounts for this protocol are all connected
								// Most protocols do not allow you to add contacts while offline
								// Would be better to have a virtual bool Kopete::Account::readyToAddContact()
								int accountcount=0;
								bool allAccountsConnected = true;
								Kopete::Account *chosen = 0;
								foreach( Kopete::Account *act, Kopete::AccountManager::self()->accounts() )
								{
									if( act->protocol() == proto) 
									{
										accountcount++;
										if(!act->isConnected())
										{
											allAccountsConnected=false;
											break;
										}
										chosen=act;
									}
								}

								if ( !allAccountsConnected )
								{
									KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
											i18n( "<qt>One or more of your accounts using %1 are offline.  Most systems have to be connected to add contacts.  Please connect these accounts and try again.</qt>", protocolName ),
											i18n( "Not Connected" )  );
									continue;
								}

								// we have got a contact to add, our accounts are connected, so add it.
								// Do we need to choose an account
								if ( accountcount > 1 )
								{	// if we have >1 account in this protocol, prompt for the protocol.
									KDialog *chooser = new KDialog(0);
									chooser->setCaption( i18n("Choose Account") );
									chooser->setButtons( KDialog::Ok | KDialog::Cancel );

									AccountSelector *accSelector = new AccountSelector(proto, chooser);
									accSelector->setObjectName( QLatin1String("accSelector") );
									chooser->setMainWidget(accSelector);
									if ( chooser->exec() == QDialog::Rejected )
										continue;
									chosen = accSelector->selectedItem();

									delete chooser;
								}
								else if ( accountcount == 0 )
								{
									KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
											i18n( "<qt>You do not have an account configured for <b>%1</b> yet.  Please create an account, connect it, and try again.</qt>", protocolName ),
											i18n( "No Account Found" )  );
									continue;
								}

								// add the contact to the chosen account
								if ( chosen )
								{
									kDebug( 14010 ) << "Adding " << *it << " to " << chosen->accountId();
									if ( chosen->addContact( *it, mc ) )
										contactAdded = true;
									else
										KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
											i18n( "<qt>It was not possible to add the contact.</qt>" ),
											i18n( "Could Not Add Contact") ) ;
								}
							}
							else
								kDebug( 14010 ) << " user declined to add " << *it << " to contact list ";
						}
					}
					kDebug( 14010 ) << " all " << addresses.count() << " contacts in " << proto->pluginId() << " checked ";
				}
				else
					kDebug( 14010 ) << "not interested in name=" << name;

			}
			else
				kDebug( 14010 ) << "not interested in app=" << app;
		}
	}
	return contactAdded;
	return false;
}

// FIXME: Remove when IM address API is in KABC (KDE 4)
void KABCPersistence::splitField( const QString &str, QString &app, QString &name, QString &value )
{
	int colon = str.indexOf( ':' );
	if ( colon != -1 ) {
		const QString tmp = str.left( colon );
		value = str.mid( colon + 1 );

		int dash = tmp.indexOf( '-' );
		if ( dash != -1 ) {
			app = tmp.left( dash );
			name = tmp.mid( dash + 1 );
		}
	}
}

} // end namespace Kopete

		// dump addressbook contents

#include "kabcpersistence.moc"
