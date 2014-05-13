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
#include "cryptographyplugin.h"

// qt stuff
#include <QList>
#include <QTextDocument>

// kde stuff
#include <kapplication.h>
#include <kdebug.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

// kopete stuff
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopetesimplemessagehandler.h"
#include "kopeteuiglobal.h"
#include "kopetecontact.h"
#include "kopeteprotocol.h"
#include "kopetemessageevent.h"
#include "kabcpersistence.h"

// crypto stuff
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
#include <gpgme++/key.h>

// kabc stuff
#include <kabc/addressbook.h>

// our own stuff
#include "cryptographyselectuserkey.h"
#include "cryptographyguiclient.h"
#include "exportkeys.h"
#include "cryptographymessagehandler.h"
#include "cryptographysettings.h"
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
	mInboundHandler = new CryptographyMessageHandlerFactory ( Kopete::Message::Inbound,
	        Kopete::MessageHandlerFactory::InStageToSent, this, SLOT (slotIncomingMessage(Kopete::MessageEvent*)) );
	connect ( Kopete::ChatSessionManager::self(),
	          SIGNAL (aboutToSend(Kopete::Message&)),
	          SLOT (slotOutgoingMessage(Kopete::Message&)) );

	// actions in the contact list
	KAction *action = new KAction ( KIcon ( "document-encrypt" ), i18nc ( "@action", "&Select Public Key..." ), this );
	actionCollection()->addAction ( "contactSelectKey", action );
	connect ( action, SIGNAL (triggered(bool)), this, SLOT (slotSelectContactKey()) );
	connect ( Kopete::ContactList::self() , SIGNAL (metaContactSelected(bool)) , action , SLOT (setEnabled(bool)) );
	action->setEnabled ( Kopete::ContactList::self()->selectedMetaContacts().count() == 1 );

	action = new KAction ( KIcon ( "document-export-key" ), i18nc ( "@action", "&Export Public Keys To Address Book..." ), this );
	actionCollection()->addAction ( "exportKey", action );
	connect ( action, SIGNAL (triggered(bool)), this, SLOT (slotExportSelectedMetaContactKeys()) );
	connect ( Kopete::ContactList::self() , SIGNAL (metaContactSelected(bool)) , action , SLOT (setEnabled(bool)) );
	action->setEnabled ( Kopete::ContactList::self()->selectedMetaContacts().count() == 1 );

	setXMLFile ( "cryptographyui.rc" );

	// add functionality to chat window when one opens
	connect ( Kopete::ChatSessionManager::self(), SIGNAL (chatSessionCreated(Kopete::ChatSession*)) , SLOT (slotNewKMM(Kopete::ChatSession*)) );

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

// just like ::self(). return pointer to singleton
CryptographyPlugin* CryptographyPlugin::plugin()
{
	return mPluginStatic ;
}

// handling an "incoming" message. this means any message to be displayed in the chat window, including ones sent by the user
void CryptographyPlugin::slotIncomingMessage ( Kopete::MessageEvent *messageEvent )
{
	Kopete::Message msg = messageEvent->message();
	QString body = msg.plainBody();

	if ( ! ( (
	        body.startsWith ( QString::fromLatin1 ( "-----BEGIN PGP MESSAGE----" ) )
	        || body.startsWith ( QString::fromLatin1 ( "-----BEGIN PGP SIGNED MESSAGE-----" ) )
	     ) && (
	        body.contains ( QString::fromLatin1 ( "-----END PGP MESSAGE----" ) )
	        || body.contains ( QString::fromLatin1 ( "-----END PGP SIGNATURE-----" ) )
         ) ) )
		return;

	kDebug ( 14303 ) << "processing " << body;

	// launch a crypto job, and store it with the message, so that when the job finishes, we know what message it was for
	const Kleo::CryptoBackendFactory *cpf = Kleo::CryptoBackendFactory::instance();
	Q_ASSERT ( cpf );
	const Kleo::CryptoBackend::Protocol *proto = cpf->openpgp();
	Q_ASSERT ( proto );

	Kleo::DecryptVerifyJob * decryptVerifyJob = proto->decryptVerifyJob();
	connect ( decryptVerifyJob, SIGNAL (result(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)), this, SLOT (slotIncomingMessageContinued(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)) );
	mCurrentJobs.insert ( decryptVerifyJob, msg );
	decryptVerifyJob->start ( body.toLatin1() );

	// message is no longer needed, it is killed. We will see it again when the crypto job is done and it comes out of mCurrentJobs
	messageEvent->discard();
}

// this is called when the incoming crypto job is done
void CryptographyPlugin::slotIncomingMessageContinued ( const GpgME::DecryptionResult &  decryptionResult, const GpgME::VerificationResult &verificationResult, const QByteArray &plainText )
{
	Kopete::Message msg = mCurrentJobs.take ( static_cast<Kleo::Job*> ( sender() ) );

	QString body = plainText;

	if ( !body.isEmpty() )
	{
		// if was signed *and* encrypted, this will be true
		if ( verificationResult.numSignatures() && decryptionResult.numRecipients() ) {
			finalizeMessage ( msg, body, verificationResult, true/*encrytped*/ );
		}
		// was not signed *and* encrypted, may be one or the other. launch a job to see about both possibilities
		else {
			const Kleo::CryptoBackendFactory *cpf = Kleo::CryptoBackendFactory::instance();
			Q_ASSERT ( cpf );
			const Kleo::CryptoBackend::Protocol *proto = cpf->openpgp();
			Q_ASSERT ( proto );

			Kleo::DecryptJob * decryptJob = proto->decryptJob();
			connect ( decryptJob, SIGNAL (result(GpgME::DecryptionResult,QByteArray)), this, SLOT (slotIncomingEncryptedMessageContinued(GpgME::DecryptionResult,QByteArray)) );
			mCurrentJobs.insert ( decryptJob, msg );
			decryptJob->start ( msg.plainBody().toLatin1() );

			Kleo::VerifyOpaqueJob * verifyJob = proto->verifyOpaqueJob();
			connect ( verifyJob, SIGNAL (result(GpgME::VerificationResult,QByteArray)), this, SLOT (slotIncomingSignedMessageContinued(GpgME::VerificationResult,QByteArray)) );
			mCurrentJobs.insert ( verifyJob, msg );
			verifyJob->start ( msg.plainBody().toLatin1() );
		}
	}
}

// if message was only encrypted, this will be called
void CryptographyPlugin::slotIncomingEncryptedMessageContinued ( const GpgME::DecryptionResult & decryptionResult, const QByteArray &plainText )
{
	Kopete::Message msg = mCurrentJobs.take ( static_cast<Kleo::Job*> ( sender() ) );

	QString body = plainText;

	if ( !body.isEmpty() )
	{
		if ( decryptionResult.numRecipients() ) {
			finalizeMessage ( msg, body, GpgME::VerificationResult(), true/*encrypted*/ );
		}
	}
}

// if message was only signed, this will be called
void CryptographyPlugin::slotIncomingSignedMessageContinued ( const GpgME::VerificationResult &verificationResult, const QByteArray &plainText )
{
	Kopete::Message msg = mCurrentJobs.take ( static_cast<Kleo::Job*> ( sender() ) );

	QString body = plainText;

	if ( ( !body.isEmpty() ) && ( verificationResult.numSignatures() ) ) {
		finalizeMessage ( msg, body, verificationResult, false/*encrypted*/ );
	}
}

// apply signature icons and put message in chat window
void CryptographyPlugin::finalizeMessage ( Kopete::Message & msg, const QString &intendedBody, const GpgME::VerificationResult & verificationResult, bool encrypted )
{
	QString body = intendedBody;
	// turn our plaintext body into html, so then it makes sense to stick HTML tags in it
	if ( ! Qt::mightBeRichText ( body ) )
		body = Qt::convertFromPlainText ( body, Qt::WhiteSpaceNormal );
	body = body.remove ( QRegExp ( "<p[^>]*>", Qt::CaseInsensitive ) );
	body = body.remove ( QRegExp ( "</p>", Qt::CaseInsensitive ) );

	// apply crypto state icons
	// use lowest common denominator; entire message is considered invalid if one signature is
	GpgME::Signature::Validity validity = ( GpgME::Signature::Validity ) GpgME::Signature::Unknown;
	bool firstTime = true;
	std::vector<GpgME::Signature> signatures = verificationResult.signatures();
	for ( size_t i = 0 ; i < verificationResult.signatures().size() ; i++ )
	{
		kDebug ( 14303 ) << "signature" << i << "validity is" << signatures[i].validityAsString() << "from" << signatures[i].fingerprint();
		if ( validity > signatures[i].validity() || firstTime ) {
			validity = signatures[i].validity();
			firstTime = false;
		}
	}

	if ( verificationResult.numSignatures() ) {
		if ( validity == GpgME::Signature::Ultimate || validity == GpgME::Signature::Full )
		{
			body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "security-high", KIconLoader::Small ) + "\">&nbsp;" );
			msg.addClass ( "cryptography:signedvalid" );
			kDebug ( 14303 ) << "message has fully valid signatures";
		}
		else if ( validity == GpgME::Signature::Marginal ) {
			body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "security-medium", KIconLoader::Small ) + "\">&nbsp;" );
			msg.addClass ( "cryptography:signedmarginal" );
			kDebug ( 14303 ) << "message has marginally signatures";
		}
		else  {
			body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "security-low", KIconLoader::Small ) + "\">&nbsp;" );
			msg.addClass ( "crytography:signedinvalid" );
			kDebug ( 14303 ) << "message has unverified signatures";
		}
	}

	if ( encrypted ) {
		body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "object-locked", KIconLoader::Small ) + "\">&nbsp;" );
		msg.addClass ( "cryptography:encrypted" );
		kDebug ( 14303 ) << "message was encrypted";
	}

	msg.setHtmlBody ( body );
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
	bool signing = ( ( msg.to().first()->pluginData ( this, "sign_messages" ) ) == "on" );
	bool encrypting = ( ( msg.to().first()->pluginData ( this, "encrypt_messages" ) ) == "on" );
	QByteArray result;

	if ( ! ( signing || encrypting ) )
		return;

	kDebug ( 14303 ) << ( signing ? "signing" : "" ) << ( encrypting ? "encrypting" : "" ) << "message " << msg.plainBody();

	const Kleo::CryptoBackendFactory *cpf = Kleo::CryptoBackendFactory::instance();
	Q_ASSERT ( cpf );
	const Kleo::CryptoBackend::Protocol *proto = cpf->openpgp();
	Q_ASSERT ( proto );

	// create std::vector with list of signing keys (really just the one the user has set)
	if ( signing )
	{
		Kleo::KeyListJob * listJob = proto->keyListJob();
		listJob->exec ( QStringList ( CryptographySettings::privateKeyFingerprint() ), true/*secretOnly*/, signingKeys );
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
		encryptingKeysPattern.append ( CryptographySettings::privateKeyFingerprint() );
		Kleo::KeyListJob * listJob = proto->keyListJob();
		listJob->exec ( encryptingKeysPattern, false/*secretOnly*/, encryptingKeys );
	}
	// do both signing and encrypting at once
	if ( signing && encrypting )
	{
		Kleo::SignEncryptJob * job = proto->signEncryptJob ( true/*armor*/ );
		job->exec ( signingKeys, encryptingKeys, msg.plainBody().toLatin1(), true, result );
	}
	// sign message body
	else if ( signing )
	{
		kDebug ( 14303 ) << "clearsign:" << (CryptographySettings::clearSignMode() ? "true" : "false");
		Kleo::SignJob * job = proto->signJob ( true/*armor*/ );
		job->exec ( signingKeys, msg.plainBody().toLatin1(), CryptographySettings::clearSignMode() ? GpgME::Clearsigned : GpgME::NormalSignatureMode, result );
	}
	// encrypt message body to all recipients
	else if ( encrypting )
	{
		Kleo::EncryptJob * job = proto->encryptJob ( true/*armor*/ );
		job->exec ( encryptingKeys, msg.plainBody().toLatin1(), true, result );
	}

	if ( !result.isEmpty() )
	{
		msg.setPlainBody ( result );
		if ( encrypting )
			msg.addClass ( "encrypted" );
		if ( signing )
			msg.addClass ( "signed" );
	}
	else
		kDebug ( 14303 ) << "empty result";
}

// Contruct dialog to show/edit metacontact's public key.
void CryptographyPlugin::slotSelectContactKey()
{
	Kopete::MetaContact *m = Kopete::ContactList::self()->selectedMetaContacts().first();
	if ( !m )
		return;
	// key we already have
	QString key = m->pluginData ( this, "gpgKey" );
	QPointer <CryptographySelectUserKey> opts = new CryptographySelectUserKey ( key, m );
	opts->exec();
	if ( opts && opts->result() )
	{
		key = opts->publicKey();
		m->setPluginData ( this, "gpgKey", key );
	}
	delete opts;
}


// construct crytography toolbars/buttons in a newly opened chat window
void CryptographyPlugin::slotNewKMM ( Kopete::ChatSession *KMM )
{
	CryptographyGUIClient * gui = new CryptographyGUIClient ( KMM );
	connect ( this , SIGNAL (destroyed(QObject*)), gui, SLOT (deleteLater()) );

	// warn about unfriendly protocols
	if ( KMM->protocol() ) {
		QString protocol ( KMM->protocol()->metaObject()->className() );
		if ( protocol != "Kopete::Protocol" ) {
			if ( ! supportedProtocols().contains ( protocol ) ) {
				KMessageBox::information ( 0, i18nc ( "@info", "This protocol %1 may not work with messages that are encrypted. This is because encrypted messages are very long, and the server or peer may reject them due to their length. To avoid being signed off or your account being warned or temporarily suspended, turn off encryption.", protocol ),
				                           i18n ( "Cryptography Unsupported Protocol %1", protocol ), "Warn about unsupported " + protocol );
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

	kDebug ( 14303 ) << "keys found in address book for contact " << addressee.assembledName() << ": " << keys;

	return keys;
}

// show dialog to allow user to check which key they want
QString CryptographyPlugin::kabcKeySelector ( QString displayName, QString addresseeName, QStringList keys, QWidget *parent )
{
	// just a Yes/No about whether to accept the key
	if ( keys.count() == 1 ) {
		if ( KMessageBox::questionYesNo ( parent, i18nc ( "@info", "Cryptography plugin has found an encryption key for %1 (%2) in your KDE address book. Do you want to use key %3 as this contact's public key? ", displayName, addresseeName, keys.first().right ( 8 ).prepend ( "0x" ) ),
		                                  i18n ( "Public Key Found" ) ) == KMessageBox::Yes ) {
			return keys.first();
		}
	}
	// allow for selection of key out of many
	if ( keys.count() > 1 )
	{
		QPointer <KDialog> dialog = new KDialog ( parent );
		QPointer <QWidget> w = new QWidget ( dialog );
		Ui::KabcKeySelectorUI ui;
		ui.setupUi ( w );
		dialog->setCaption ( i18n ( "Public Keys Found" ) );
		dialog->setButtons ( KDialog::Ok | KDialog::Cancel );
		dialog->setMainWidget ( w );
		ui.label->setText ( i18nc ( "@info", "Cryptography plugin has found multiple encryption keys for %1 (%2) in your KDE address book. To use one of these keys, select it and choose OK.",
		                           displayName, addresseeName ) );
		for ( int i = 0; i < keys.count(); i++ )
			ui.keyList->addItem ( new QListWidgetItem ( KIcon ( "application-pgp-keys" ), keys[i].right ( 8 ).prepend ( "0x" ), ui.keyList ) );
		ui.keyList->addItems ( keys );
		QString ret;
		if ( dialog->exec() )
			ret = ui.keyList->currentItem()->text();
		delete dialog;
		if ( ! ret.isEmpty() )
			return ret;
	}
	return QString();
}

// export the public keys of the chat session's contacts
void CryptographyPlugin::slotExportSelectedMetaContactKeys()
{
	QPointer <ExportKeys> dialog = new ExportKeys ( Kopete::ContactList::self()->selectedMetaContacts(), Kopete::UI::Global::mainWidget() );
	dialog->exec();
	delete dialog;
}

#include "cryptographyplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

