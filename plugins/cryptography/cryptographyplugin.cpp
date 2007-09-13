/*
    cryptographyplugin.cpp  -  description

    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2007      by Charles Connell        <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include <QList>
#include <QTimer>

#include <kapplication.h>
#include <kdebug.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpassworddialog.h>
#include <kactioncollection.h>
#include <kleo/ui/keylistview.h>
#include <kabc/addressbook.h>
#include <kopete/kabcpersistence.h>

#include <kopete/kopetemetacontact.h>
#include <kopete/kopetecontactlist.h>
#include <kopete/kopetechatsessionmanager.h>
#include <kopete/kopetesimplemessagehandler.h>
#include <kopete/kopeteuiglobal.h>
#include <kopete/kopetecontact.h>
#include <kopete/kopeteprotocol.h>

#include "cryptographyplugin.h"
#include "cryptographyselectuserkey.h"
#include "cryptographyguiclient.h"
#include "exportkeys.h"
#include "gpginterface.h"

#include "ui_kabckeyselectorbase.h"

CryptographyPlugin* CryptographyPlugin::mPluginStatic = 0L;

K_PLUGIN_FACTORY ( CryptographyPluginFactory, registerPlugin<CryptographyPlugin>(); )
K_EXPORT_PLUGIN ( CryptographyPluginFactory ( "kopete_cryptography" ) )

CryptographyPlugin::CryptographyPlugin ( QObject *parent, const QVariantList &/*args*/ )
		: Kopete::Plugin ( CryptographyPluginFactory::componentData(), parent )
{
	if ( !mPluginStatic )
		mPluginStatic=this;

	// set up slots to handle incoming and outgoing messages
	mInboundHandler = new Kopete::SimpleMessageHandlerFactory ( Kopete::Message::Inbound,
	        Kopete::MessageHandlerFactory::InStageToSent, this, SLOT ( slotIncomingMessage ( Kopete::Message& ) ) );
	connect ( Kopete::ChatSessionManager::self(),
	          SIGNAL ( aboutToSend ( Kopete::Message & ) ),
	          SLOT ( slotOutgoingMessage ( Kopete::Message & ) ) );

	// actions in the contact list
	KAction *action = new KAction ( KIcon ( "encrypted" ), i18n ( "&Select Public Key..." ), this );
	actionCollection()->addAction ( "contactSelectKey", action );
	connect ( action, SIGNAL ( triggered ( bool ) ), this, SLOT ( slotSelectContactKey() ) );
	connect ( Kopete::ContactList::self() , SIGNAL ( metaContactSelected ( bool ) ) , action , SLOT ( setEnabled ( bool ) ) );
	action->setEnabled ( Kopete::ContactList::self()->selectedMetaContacts().count() == 1 );

	mExportKeys = new KAction ( KIcon ( "kgpg-export-kgpg" ), i18n ( "&Export Public Keys To Address Book..." ), this );
	actionCollection()->addAction ( "exportKey", mExportKeys );
	connect ( mExportKeys, SIGNAL ( triggered ( bool ) ), this, SLOT ( slotExportSelectedMetaContactKeys() ) );
	connect ( Kopete::ContactList::self() , SIGNAL ( selectionChanged ( bool ) ) , mExportKeys , SLOT ( slotContactSelectionChanged() () ) );
	slotContactSelectionChanged();

	setXMLFile ( "cryptographyui.rc" );
	connect ( this, SIGNAL ( settingsChanged() ), this, SLOT ( loadSettings() ) );

	// add functionality to chat window when one opens
	connect ( Kopete::ChatSessionManager::self(), SIGNAL ( chatSessionCreated ( Kopete::ChatSession * ) ) , SLOT ( slotNewKMM ( Kopete::ChatSession * ) ) );

	// when selected metacontacts changes, actions may need to be enabled/disabled
	connect ( Kopete::ContactList::self(), SIGNAL ( selectionChanged() ), this, SLOT ( slotContactSelectionChanged() ) );

	//Add GUI action to all already existing kmm (if the plugin is launched when kopete already running)
	QList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
	foreach ( Kopete::ChatSession *session, sessions )
	{
		slotNewKMM ( session );
	}

}

CryptographyPlugin::~CryptographyPlugin()
{
	delete mInboundHandler;
	mPluginStatic = 0L;
}

// just like ::self(). runturn pointer to singleton
CryptographyPlugin* CryptographyPlugin::plugin()
{
	return mPluginStatic ;
}

// handling an "incoming" message. this means any message to be displayed in the chat window, including ones sent by the user
void CryptographyPlugin::slotIncomingMessage ( Kopete::Message& msg )
{
	QString body = msg.plainBody();
	// iconFolder is the folder with the little lock and pen icons in it
	QString iconFolder = KIconLoader::global()->iconPath ( "signature", K3Icon::Small );
	iconFolder = iconFolder.remove ( iconFolder.lastIndexOf ( "/" ) +1, 100 );

	int opState;
	// if we detect the lock or pen icons in the message, a forgery attempt has occurred, so we just replace the message body with a warning
	if ( !body.startsWith ( QString::fromLatin1 ( "-----BEGIN PGP MESSAGE----" ) )
	        || !body.contains ( QString::fromLatin1 ( "-----END PGP MESSAGE----" ) ) )
	{
		if ( body.contains ( QRegExp ( iconFolder + "signature\\..*|" + iconFolder + "bad_signature\\..*|" + iconFolder + "encrypted\\..*" ) ) )
		{
			msg.setHtmlBody ( i18n ( "Cryptography plugin refuses to show the most recent message because it contains an attempt to forge an encrypted or signed message" ) );
			msg.setType ( Kopete::Message::TypeAction );
			msg.addClass ( "cryptography:encrypted" );
		}
		return;
	}
	
	kDebug ( 14303 ) << "processing " << body;

	body = GpgInterface::decryptText ( body, opState );

	if ( !body.isEmpty() )
	{
		// same story as this exact same code from above
		if ( body.contains ( QRegExp ( iconFolder + "signature\\..*|" + iconFolder + "bad_signature\\..*|" + iconFolder + "encrypted\\..*" ) ) )
		{
			msg.setHtmlBody ( i18n ( "Cryptography plugin refuses to show the most recent message because it contains an attempt to forge an encrypted or signed message" ) );
			msg.setType ( Kopete::Message::TypeAction );
			msg.addClass ( "cryptography:encrypted" );
			return;
		}

		// apply crypto state icons
		if ( opState & GpgInterface::GoodSig ) {
			body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "signature", K3Icon::Small ) + "\">&nbsp;&nbsp;" );
			kDebug (14303) << "message has verified signature";
		}

		if ( ( opState & GpgInterface::ErrorSig ) || ( opState & GpgInterface::BadSig ) ){
			body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "bad_signature", K3Icon::Small ) + "\">&nbsp;&nbsp;" );
			kDebug (14303) << "message has unverified signature";
		}

		if ( opState & GpgInterface::Decrypted ){
			body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "encrypted", K3Icon::Small ) + "\">&nbsp;&nbsp;" );
			kDebug (14303) << "message has been decrypted";
		}

		msg.setHtmlBody ( body );

		msg.addClass ( "cryptography:encrypted" );
		
		kDebug (14303) << "result is " << body;
	}
	return;
}

// encrypt and or sign a message to be sent
void CryptographyPlugin::slotOutgoingMessage ( Kopete::Message& msg )
{
	if ( msg.direction() != Kopete::Message::Outbound )
		return;

	QStringList keys;
	QList<Kopete::Contact*> contactlist = msg.to();
	bool signing = ( ( msg.to().first()->metaContact()->pluginData ( this, "sign_messages" ) ) == "on" );
	bool encrypting = ( ( msg.to().first()->metaContact()->pluginData ( this, "encrypt_messages" ) ) == "on" );

	kDebug ( 14303 ) << ( signing ? "signing" : "" ) << ( encrypting ? "encrypting" : "" ) << "message " << msg.plainBody();

	if ( encrypting )
	{
		foreach ( Kopete::Contact *c, contactlist )
		{
			QString tmpKey;
			if ( c->metaContact() )
				tmpKey = c->metaContact()->pluginData ( this, "gpgKey" );
			if ( tmpKey.isEmpty() )
			{
				kDebug ( 14303 ) << "empty key";
				KMessageBox::sorry ( Kopete::UI::Global::mainWidget(), i18n ( "You have not chosen an encryption key for one or more recipients" ) );
				return;
			}
			keys.append ( tmpKey );
		}
		// encrypt to self so we can decrypt during slotIncomingMessage()
		keys.append ( CryptographyConfig::self()->fingerprint() );
		QString key = keys.join ( " " );

		if ( key.isEmpty() )
			return;

		QString result = GpgInterface::encryptText ( msg.plainBody(), key, signing, CryptographyConfig::self()->fingerprint() );
		if ( !result.isEmpty() )
			msg.setPlainBody ( result );
		else
			kDebug ( 14303 ) << "empty result";
	}
	else if ( signing )
	{
		QString result = GpgInterface::signText ( msg.plainBody(), CryptographyConfig::self()->fingerprint() );
		if ( !result.isEmpty() )
			msg.setPlainBody ( result );
	}
}

// Contruct dialog to show/edit metacontact's public key.
void CryptographyPlugin::slotSelectContactKey()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	if ( !m )
		return;
	// key we already have
	QString key = m->pluginData ( this, "gpgKey" );
	CryptographySelectUserKey opts ( key, m );
	opts.exec();
	if ( opts.result() )
	{
		key = opts.publicKey();
		m->setPluginData ( this, "gpgKey", key );
	}
}

// construct crytography toolbars/buttons in a newly opened chat window
void CryptographyPlugin::slotNewKMM ( Kopete::ChatSession *KMM )
{
	CryptographyGUIClient * gui = new CryptographyGUIClient ( KMM );
	connect ( this , SIGNAL ( destroyed ( QObject* ) ), gui, SLOT ( deleteLater() ) );

	// warn about unfriendly protocols
	QString protocol ( KMM->protocol()->metaObject()->className() );
	if ( gui->m_encAction->isChecked() )
		if ( ! supportedProtocols().contains ( protocol ) )
			KMessageBox::information ( 0, i18n ( "This protocol may not work with messages that are encrypted. This is because encrypted messages are very long, and the server or peer may reject them due to their length. To avoid being signed off or your account being warned or temporarily suspended, turn off encryption." ),
			                           i18n ( "Cryptography Unsupported Protocol" ), "Warn about unsupported " + QString ( KMM->protocol()->metaObject()->className() ) );
}

QStringList CryptographyPlugin::getKabcKeys ( QString uid )
{
	KABC::Addressee addressee = Kopete::KABCPersistence::self()->addressBook()->findByUid ( uid );
	QStringList keys;

	// each 'if' block here is one way of getting a key.

	// this is the field used by kaddressbook
	if ( ! ( addressee.custom ( "KADDRESSBOOK", "OPENPGPFP" ) ).isEmpty() )
		keys << addressee.custom ( "KADDRESSBOOK", "OPENPGPFP" );

	// this is the standard key field in Addressee, (which no app seems to use, but here it is anyways)
	if ( ! ( addressee.key ( KABC::Key::PGP ).textData() ).isEmpty() )
		keys << addressee.key ( KABC::Key::PGP ).textData();

	// remove duplicates
	if ( keys.count() >= 2 )
		if ( keys.at ( 0 ) == keys.at ( 1 ) )
			keys.removeAt ( 1 );

	kDebug ( 14303 ) << "keys found in address book for contact " << addressee.assembledName() << ": " << keys << endl;

	return keys;
}

QString CryptographyPlugin::KabcKeySelector ( QString displayName, QString addresseeName, QStringList keys, QWidget *parent )
{
	// just a Yes/No about whether to accept the key
	if ( keys.count() == 1 ) {
		if ( KMessageBox::questionYesNo ( parent,
		                                  i18n ( QString ( "Cryptography plugin has found an encryption key for " + displayName + " (" + addresseeName + ")" + " in your KDE address book. Do you want to use key "+ keys.first().right ( 8 ).prepend ( "0x" ) + " as this contact's public key?" ).toLocal8Bit() ),
		                                  i18n ( "Public Key Found" ) ) == KMessageBox::Yes ) {
			return keys.first();
		}
	}
	// allow for selection of key out of many
	if ( keys.count() > 1 )
	{
		KDialog dialog ( parent );
		QWidget w ( &dialog );
		Ui::KabcKeySelectorUI ui;
		ui.setupUi ( &w );
		dialog.setCaption ( i18n ( "Public Keys Found" ) );
		dialog.setButtons ( KDialog::Ok | KDialog::Cancel );
		dialog.setMainWidget ( &w );
		ui.label->setText ( i18n ( QString ( "Cryptography plugin has found multiple encryption keys for " + displayName + " (" + addresseeName + ")" + " in your KDE address book. To use one of these keys, select it and choose OK." ).toLocal8Bit() ) );
		for ( int i = 0; i < keys.count(); i++ )
			ui.keyList->addItem ( new QListWidgetItem ( KIconLoader::global()->loadIconSet ( "kgpg-key1-kopete", K3Icon::Small ), keys[i].right ( 8 ).prepend ( "0x" ), ui.keyList ) );
		ui.keyList->addItems ( keys );
		if ( dialog.exec() )
			return ui.keyList->currentItem()->text();
	}
	return QString();
}

// export the public keys of the chat session's contacts
void CryptographyPlugin::slotExportSelectedMetaContactKeys()
{
	ExportKeys dialog ( Kopete::ContactList::self()->selectedMetaContacts(), Kopete::UI::Global::mainWidget() );
	dialog.exec();
}

// enable or disable "Export Keys" depending on key whether we have a key for the selected metacontacts
void CryptographyPlugin::slotContactSelectionChanged()
{
	kDebug ( 14303 );
	bool keyFound = false;
	foreach ( Kopete::MetaContact * mc, Kopete::ContactList::self()->selectedMetaContacts() ) {
		if ( mc->pluginData ( CryptographyPlugin::plugin(), "gpgKey" ) != QString() ) {
			keyFound = true;
			kDebug ( 14303 ) << "metacontact " << mc->displayName() << "has a key";
		}
	}
	if ( keyFound )
		mExportKeys->setEnabled ( true );
	else
		mExportKeys->setEnabled ( false );
}

#include "cryptographyplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

