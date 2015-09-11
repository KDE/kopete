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

#include <QString>
#include <QTimer>

#include <kcontacts/addressee.h>

// UI related includes used for importing from KABC
#include <QDialog>
#include <klocale.h>
#include <kmessagebox.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
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
	QList<KContacts::Resource *> pendingResources;
	bool addrBookWritePending;

	// FIXME: Try to remove that static variable !
	// UPDATE : Deprecated in KF5, Fuck off !
	//static KContacts::AddressBook* s_addressBook;
};

KContacts::AddressBook* KABCPersistence::Private::s_addressBook = 0L;

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

KContacts::AddressBook* KABCPersistence::addressBook()
{
	if ( Private::s_addressBook == 0L )
	{
		Private::s_addressBook = KContacts::StdAddressBook::self();
		KContacts::StdAddressBook::setAutomaticSave( false );
	}
	return Private::s_addressBook;
}

void KABCPersistence::write( MetaContact * mc )
{
	// Save any changes in each contact's addressBookFields to KABC
	KContacts::AddressBook* ab = addressBook();

	qCDebug(LIBKOPETE_LOG) << "looking up Addressee for " << mc->displayName() << "...";
	// Look up the address book entry
	KContacts::Addressee theAddressee = ab->findByUid( mc->kabcId() );
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
			qCDebug(LIBKOPETE_LOG) << "Writing: " << it.key() << ", " << "All" << ", " << toWrite;
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

			qCDebug(LIBKOPETE_LOG) << "...FOUND ONE!";
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
					qCDebug(LIBKOPETE_LOG) << "Writing: " << appIt.key() << ", " << addrIt.key() << ", " << toWrite;
					theAddressee.insertCustom( appIt.key(), addrIt.key(), toWrite );
				}
			}
			ab->insertAddressee( theAddressee );
			writeAddressBook();
		}*/
}

void KABCPersistence::writeAddressBook( KContacts::Resource * res)
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
	//qCDebug(LIBKOPETE_LOG) ;
	KContacts::AddressBook* ab = addressBook();
	QListIterator<KContacts::Resource *> it( d->pendingResources );
	while ( it.hasNext() )
	{
		//qCDebug(LIBKOPETE_LOG)  << "Writing resource " << it.current()->resourceName();
		KContacts::Ticket *ticket = ab->requestSaveTicket( it.next() );
		if ( !ticket )
			qCWarning(LIBKOPETE_LOG) << "WARNING: Resource is locked by other application!";
		else
		{
			if ( !ab->save( ticket ) )
			{
				qCWarning(LIBKOPETE_LOG) << "ERROR: Saving failed!";
				ab->releaseSaveTicket( ticket );
			}
		}
		//qCDebug(LIBKOPETE_LOG) << "Finished writing KABC";
	}
	d->pendingResources.clear();
	d->addrBookWritePending = false;
}

void KABCPersistence::removeKABC( MetaContact *)
{
/*	// remove any data this KMC has written to the KDE address book
	// Save any changes in each contact's addressBookFields to KABC
	KContacts::AddressBook* ab = addressBook();

	// Wipe out the existing addressBook entries
	d->addressBook.clear();
	// This causes each Kopete::Protocol subclass to serialise its contacts' data into the metacontact's plugin data and address book data
	emit aboutToSave(this);

	// If the metacontact is linked to a kabc entry
	if ( !d->kabcId().isEmpty() )
	{
		//qCDebug(LIBKOPETE_LOG) << "looking up Addressee for " << displayName() << "...";
		// Look up the address book entry
		KContacts::Addressee theAddressee = ab->findByUid( d->kabcId() );

		if ( theAddressee.isEmpty() )
		{
			// remove the link
			//qCDebug(LIBKOPETE_LOG) << "...not found.";
			d->kabcId.clear();
		}
		else
		{
			//qCDebug(LIBKOPETE_LOG) << "...FOUND ONE!";
			// Remove address book fields
			QMap<QString, QMap<QString, QString> >::ConstIterator appIt = d->addressBook.begin();
			for( ; appIt != d->addressBook.end(); ++appIt )
			{
				QMap<QString, QString>::ConstIterator addrIt = appIt.data().begin();
				for( ; addrIt != appIt.data().end(); ++addrIt )
				{
					// FIXME: This assumes Kopete is the only app writing these fields
					qCDebug(LIBKOPETE_LOG) << "Removing: " << appIt.key() << ", " << addrIt.key();
					theAddressee.removeCustom( appIt.key(), addrIt.key() );
				}
			}
			ab->insertAddressee( theAddressee );

			writeAddressBook();
		}
	}
//	qCDebug(LIBKOPETE_LOG) << kBacktrace();*/
}

bool KABCPersistence::syncWithKABC( MetaContact * mc )
{
	qCDebug(LIBKOPETE_LOG) ;
	bool contactAdded = false;
	// check whether the dontShowAgain was checked
		KContacts::AddressBook* ab = addressBook();
		KContacts::Addressee addr  = ab->findByUid( mc->kabcId() );

		if ( !addr.isEmpty() ) // if we are associated with KABC
		{
// load the set of addresses from KABC
		const QStringList customs = addr.customs();

		QStringList::ConstIterator it;
		for ( it = customs.constBegin(); it != customs.constEnd(); ++it )
		{
			QString app, name, value;
			splitField( *it, app, name, value );
			qCDebug(LIBKOPETE_LOG) << "app=" << app << " name=" << name << " value=" << value;

			if ( app.startsWith( QLatin1String( "messaging/" ) ) )
			{
				if ( name == QLatin1String( "All" ) )
				{
					qCDebug(LIBKOPETE_LOG) << " syncing \"" << app << ":" << name << " with contact list ";
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
								qCDebug(LIBKOPETE_LOG) << *it << " already a child of this metacontact.";
								continue;
							}
							qCDebug(LIBKOPETE_LOG) << *it << " already exists in OTHER metacontact, move here?";
							// find the Kopete::Contact and attempt to move it to this metacontact.
							otherMc->findContact( proto->pluginId(), QString(), *it )->setMetaContact( mc );
						}
						else
						{
							// if not, prompt to add it
							qCDebug(LIBKOPETE_LOG) << proto->pluginId() << "://" << *it << " was not found in the contact list.  Prompting to add...";
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
									QDialog *chooser = new QDialog(0);
									chooser->setWindowTitle( i18n("Choose Account") );
									QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
									QWidget *mainWidget = new QWidget(this);
									QVBoxLayout *mainLayout = new QVBoxLayout;
									chooser->setLayout(mainLayout);
									mainLayout->addWidget(mainWidget);
									QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
									okButton->setDefault(true);
									okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
									chooser->connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
									chooser->connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
									//PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
									mainLayout->addWidget(buttonBox);

									AccountSelector *accSelector = new AccountSelector(proto, chooser);
									accSelector->setObjectName( QLatin1String("accSelector") );
									mainLayout->addWidget(accSelector);
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
									qCDebug(LIBKOPETE_LOG) << "Adding " << *it << " to " << chosen->accountId();
									if ( chosen->addContact( *it, mc ) )
										contactAdded = true;
									else
										KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
											i18n( "<qt>It was not possible to add the contact.</qt>" ),
											i18n( "Could Not Add Contact") ) ;
								}
							}
							else
								qCDebug(LIBKOPETE_LOG) << " user declined to add " << *it << " to contact list ";
						}
					}
					qCDebug(LIBKOPETE_LOG) << " all " << addresses.count() << " contacts in " << proto->pluginId() << " checked ";
				}
				else
					qCDebug(LIBKOPETE_LOG) << "not interested in name=" << name;

			}
			else
				qCDebug(LIBKOPETE_LOG) << "not interested in app=" << app;
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
