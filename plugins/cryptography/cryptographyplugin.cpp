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
#include <QTextDocument>

#include <kapplication.h>
#include <kdebug.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kmessagebox.h>
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

#include <assert.h>
#include <kleo/cryptobackendfactory.h>
#include <kleo/decryptverifyjob.h>
#include <kleo/decryptjob.h>
#include <kleo/verifyopaquejob.h>
#include <kleo/encryptjob.h>
#include <kleo/signencryptjob.h>
#include <kleo/signjob.h>
#include <kleo/keylistjob.h>
#include <kleo/job.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/keylistresult.h>
#include <gpgme++/signingresult.h>
#include <gpgme++/encryptionresult.h>

#include "cryptographyplugin.h"
#include "cryptographyselectuserkey.h"
#include "cryptographyguiclient.h"
#include "exportkeys.h"

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

	if ( !body.startsWith ( QString::fromLatin1 ( "-----BEGIN PGP MESSAGE----" ) )
	        || !body.contains ( QString::fromLatin1 ( "-----END PGP MESSAGE----" ) ) )
		return;

	kDebug ( 14303 ) << "processing " << body;

	// launch a crypto job, and store it with the message, so that when the job finishes, we know what message it was for
	const Kleo::CryptoBackendFactory *cpf = Kleo::CryptoBackendFactory::instance();
	assert ( cpf );
	const Kleo::CryptoBackend::Protocol *proto = cpf->openpgp();
	assert ( proto );

	Kleo::DecryptVerifyJob * decryptVerifyJob = proto->decryptVerifyJob();
	connect ( decryptVerifyJob, SIGNAL ( result ( const GpgME::DecryptionResult &, const GpgME::VerificationResult &, const QByteArray & ) ), this, SLOT ( slotIncomingMessageContinued ( const GpgME::DecryptionResult &, const GpgME::VerificationResult &, const QByteArray & ) ) );
	mCurrentJobs.insert ( decryptVerifyJob, msg );
	decryptVerifyJob->start ( body.toLatin1() );

	msg.setPlainBody ( "Cryptography processing..." );
	msg.setType ( Kopete::Message::TypeAction );
	
}

// this is called when the incoming crypto job is done
void CryptographyPlugin::slotIncomingMessageContinued ( const GpgME::DecryptionResult &  decryptionResult, const GpgME::VerificationResult &verificationResult, const QByteArray &plainText )
{
	Kopete::Message msg = mCurrentJobs.take ( static_cast<Kleo::Job*> ( sender() ) );

	QString body = plainText;

	if ( !body.isEmpty() )
	{
		// if was signed *and* encrypted, this will be true
		if ( verificationResult.signatures().size() ){
			if ( decryptionResult.numRecipients() >= 1 )
				finalizeMessage ( msg, body, verificationResult, true );
		}
		
		// was not signed *and* encrypted, may be one or the other. launch a job to see about both possibilities
		else {
			const Kleo::CryptoBackendFactory *cpf = Kleo::CryptoBackendFactory::instance();
			assert ( cpf );
			const Kleo::CryptoBackend::Protocol *proto = cpf->openpgp();
			assert ( proto );

			Kleo::DecryptJob * decryptJob = proto->decryptJob();
			connect ( decryptJob, SIGNAL ( result ( const GpgME::DecryptionResult &, const QByteArray & ) ), this, SLOT ( slotIncomingEncryptedMessageContinued ( const GpgME::DecryptionResult &, const QByteArray & ) ) );
			mCurrentJobs.insert ( decryptJob, msg );
			decryptJob->start ( msg.plainBody().toLatin1() );

			Kleo::VerifyOpaqueJob * verifyJob = proto->verifyOpaqueJob();
			connect ( verifyJob, SIGNAL ( result ( const GpgME::VerificationResult &, const QByteArray & ) ), this, SLOT ( slotIncomingSignedMessageContinued ( const GpgME::VerificationResult &, const QByteArray & ) ) );
			mCurrentJobs.insert ( verifyJob, msg );
			verifyJob->start ( msg.plainBody().toLatin1() );
		}
	}
}

// if was only encrypted, this will be called
void CryptographyPlugin::slotIncomingEncryptedMessageContinued ( const GpgME::DecryptionResult & decryptionResult, const QByteArray &plainText )
{
	kDebug (14303);
	Kopete::Message msg = mCurrentJobs.take ( static_cast<Kleo::Job*> ( sender() ) );

	QString body = plainText;

	if ( !body.isEmpty() )
	{
		if ( decryptionResult.numRecipients() >= 1 ) 
			finalizeMessage ( msg, body, GpgME::VerificationResult(), true );
	}
}

// if was only signed, this will be called
void CryptographyPlugin::slotIncomingSignedMessageContinued ( const GpgME::VerificationResult &verificationResult, const QByteArray &plainText )
{
	kDebug ( 14303 );
	Kopete::Message msg = mCurrentJobs.take ( static_cast<Kleo::Job*> ( sender() ) );

	QString body = plainText;

	if ( ( !body.isEmpty() ) && ( verificationResult.signatures().size() ) )
		finalizeMessage ( msg, body, verificationResult, false );
}

// apply signature icons and put message in chat window
void CryptographyPlugin::finalizeMessage ( Kopete::Message & msg, QString intendedBody, const GpgME::VerificationResult & validity, bool encrypted )
{
	msg.addClass ( "cryptography:encrypted" );
	
	// turn our plaintext body into html, so then it makes sense to stick HTML tags in it
//	msg.setPlainBody ( intendedBody );
	kDebug (14303) << intendedBody;
	
	if ( ! Qt::mightBeRichText ( intendedBody ) )
		intendedBody = Qt::convertFromPlainText( intendedBody, Qt::WhiteSpaceNormal );
	intendedBody = intendedBody.remove ( QRegExp ( "<p[^>]*>", Qt::CaseInsensitive ) );
	intendedBody = intendedBody.remove ( QRegExp ( "</p>", Qt::CaseInsensitive ) );

	// apply crypto state icons
	if ( ! ( validity.signature ( 0 ).summary() == GpgME::Signature::None ) ) {
		if ( validity.signature ( 0 ).summary() & GpgME::Signature::Valid ) {
			intendedBody.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "signature", KIconLoader::Small ) + "\">&nbsp;" );
			kDebug ( 14303 ) << "message has verified signature";
		}
		else  {
			intendedBody.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "badsignature", KIconLoader::Small ) + "\">&nbsp;" );
			kDebug ( 14303 ) << "message has unverified signature";
		}
	}
	if ( encrypted ) {
		intendedBody.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "encrypted", KIconLoader::Small ) + "\">&nbsp;" );
		kDebug ( 14303 ) << "message was encrypted";
	}
	
	msg.setHtmlBody ( intendedBody );
	kDebug ( 14303 ) << "result is " << intendedBody;
	msg.manager()->appendMessage ( msg );
}

// encrypt and or sign a message to be sent
void CryptographyPlugin::slotOutgoingMessage ( Kopete::Message& msg )
{
	if ( msg.direction() != Kopete::Message::Outbound )
		return;

	QStringList encryptingKeysPattern;
	std::vector<GpgME::Key> encryptingKeys;
	std::vector<GpgME::Key> signingKeys;
	QList<Kopete::Contact*> contactlist = msg.to();
	bool signing = ( ( msg.to().first()->metaContact()->pluginData ( this, "sign_messages" ) ) == "on" );
	bool encrypting = ( ( msg.to().first()->metaContact()->pluginData ( this, "encrypt_messages" ) ) == "on" );
	QByteArray result;

	kDebug ( 14303 ) << ( signing ? "signing" : "" ) << ( encrypting ? "encrypting" : "" ) << "message " << msg.plainBody();

	const Kleo::CryptoBackendFactory *cpf = Kleo::CryptoBackendFactory::instance();
	assert ( cpf );
	const Kleo::CryptoBackend::Protocol *proto = cpf->openpgp();
	assert ( proto );

	// create std::vector with list of signing keys (really just the one the user has set)
	if ( signing )
	{
		Kleo::KeyListJob * listJob = proto->keyListJob();
		listJob->exec ( QStringList ( CryptographyConfig::self()->fingerprint() ), true, signingKeys );
	}

	// create QStringList of recpients' keys, then convert that (using a KeyListJob) into a std::vector
	if ( encrypting )
	{
		foreach ( Kopete::Contact *c, contactlist )
		{
			QString tmpKey;
			if ( c->metaContact() )
				tmpKey = c->metaContact()->pluginData ( this, "gpgKey" );
			encryptingKeysPattern.append ( tmpKey );
		}
		// encrypt to self so we can decrypt during slotIncomingMessage()
		encryptingKeysPattern.append ( CryptographyConfig::self()->fingerprint() );
		Kleo::KeyListJob * listJob = proto->keyListJob();
		listJob->exec ( encryptingKeysPattern, false, encryptingKeys );
	}
	// do both signing and encrypting at once
	if ( signing && encrypting )
	{
		Kleo::SignEncryptJob * job = proto->signEncryptJob ( true );
		job->exec ( signingKeys, encryptingKeys, msg.plainBody().toLatin1(), true, result );
	}
	// sign message body
	else if ( signing )
	{
		Kleo::SignJob * job = proto->signJob ( true );
		job->exec ( signingKeys, msg.plainBody().toLatin1(), GpgME::NormalSignatureMode, result );
	}
	// encrypt message body to all recipients
	else if ( encrypting )
	{
		Kleo::EncryptJob * job = proto->encryptJob ( true );
		job->exec ( encryptingKeys, msg.plainBody().toLatin1(), true, result );
	}

	if ( !result.isEmpty() )
		msg.setPlainBody ( result );
	else
		kDebug ( 14303 ) << "empty result";
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
	if ( KMM->protocol() ) {
		QString protocol ( KMM->protocol()->metaObject()->className() );
		if ( gui->m_encAction->isChecked() ){
			if ( ! supportedProtocols().contains ( protocol ) ){
				KMessageBox::information ( 0, i18n ( "This protocol may not work with messages that\
						are encrypted. This is because encrypted messages are very long, and\
								the server or peer may reject them due to their\
								length. To avoid being signed off or your account\
								being warned or temporarily suspended, turn off\
								encryption." ),
				                           i18n ( "Cryptography Unsupported Protocol" ), "Warn about unsupported " + QString ( KMM->protocol()->metaObject()->className() ) );
			}
		}
	}
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

// show dialog to allow user to check which key they want
QString CryptographyPlugin::KabcKeySelector ( QString displayName, QString addresseeName, QStringList keys, QWidget *parent )
{
	// just a Yes/No about whether to accept the key
	if ( keys.count() == 1 ) {
		if ( KMessageBox::questionYesNo ( parent, i18n ( QString ( "Cryptography plugin has found an\
				   encryption key for "+ displayName+ " (" + addresseeName + ")" + " in your KDE\
						   address book. Do you want to use key "+ keys.first().right( 8 ).
						   prepend ( "0x" ) + 
						   " as this contact's public key?" ).toLocal8Bit() ),
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
		ui.label->setText ( i18n ( QString ( "Cryptography plugin has found multiple encryption keys for "
				+ displayName + " (" + addresseeName + ")" +
				" in your KDE address book. To use one of these keys, select it and choose OK." ).
				toLocal8Bit() ) );
		for ( int i = 0; i < keys.count(); i++ )
			ui.keyList->addItem ( new QListWidgetItem ( KIconLoader::global()->loadIconSet ( "kgpg-key1-kopete", KIconLoader::Small ), keys[i].right ( 8 ).prepend ( "0x" ), ui.keyList ) );
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

