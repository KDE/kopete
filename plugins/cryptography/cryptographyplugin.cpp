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
#include <kleo/ui/keylistview.h>
#include <kabc/addressbook.h>
#include "kabcpersistence.h"

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopetesimplemessagehandler.h"
#include "kopeteuiglobal.h"
#include "kopetecontact.h"
#include "kopeteprotocol.h"

#include "cryptographyplugin.h"
#include "cryptographyselectuserkey.h"
#include "cryptographyguiclient.h"

#include "gpginterface.h"
#include <kactioncollection.h>

typedef KGenericFactory<CryptographyPlugin> CryptographyPluginFactory;
static const KAboutData aboutdata ( "kopete_cryptography", 0, ki18n ( "Cryptography" ) , "1.1" );
K_EXPORT_COMPONENT_FACTORY ( kopete_cryptography, CryptographyPluginFactory ( &aboutdata ) )

CryptographyPlugin::CryptographyPlugin ( QObject *parent, const QStringList & /* args */ )
		: Kopete::Plugin ( CryptographyPluginFactory::componentData(), parent ),
		m_cachedPass()
{
	if ( !pluginStatic_ )
		pluginStatic_=this;

	m_inboundHandler = new Kopete::SimpleMessageHandlerFactory ( Kopete::Message::Inbound,
	                   Kopete::MessageHandlerFactory::InStageToSent, this, SLOT ( slotIncomingMessage ( Kopete::Message& ) ) );
	connect ( Kopete::ChatSessionManager::self(),
	          SIGNAL ( aboutToSend ( Kopete::Message & ) ),
	          SLOT ( slotOutgoingMessage ( Kopete::Message & ) ) );

	m_cachedPass_timer = new QTimer ( this );
	m_cachedPass_timer->setObjectName ( "m_cachedPass_timer" );
	QObject::connect ( m_cachedPass_timer, SIGNAL ( timeout() ), this, SLOT ( slotForgetCachedPass() ) );

	CryptographyPlugin::plugin()->addAddressBookField ( "key" );

	KAction *action=new KAction ( KIcon ( "encrypted" ), i18n ( "&Select Public Key..." ), this );
	actionCollection()->addAction ( "contactSelectKey", action );
	connect ( action, SIGNAL ( triggered ( bool ) ), this, SLOT ( slotSelectContactKey() ) );
	connect ( Kopete::ContactList::self() , SIGNAL ( metaContactSelected ( bool ) ) , action , SLOT ( setEnabled ( bool ) ) );
	action->setEnabled ( Kopete::ContactList::self()->selectedMetaContacts().count() ==1 );

	setXMLFile ( "cryptographyui.rc" );
	loadSettings();
	connect ( this, SIGNAL ( settingsChanged() ), this, SLOT ( loadSettings() ) );

	connect ( Kopete::ChatSessionManager::self(), SIGNAL ( chatSessionCreated ( Kopete::ChatSession * ) ) , SLOT ( slotNewKMM ( Kopete::ChatSession * ) ) );

	slotAskPassphraseOnStartup();

	//Add GUI action to all already existing kmm (if the plugin is launched when kopete already rining)
	QList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
	foreach ( Kopete::ChatSession *session, sessions )
	{
		slotNewKMM ( session );
	}

}

CryptographyPlugin::~CryptographyPlugin()
{
	delete m_inboundHandler;
	pluginStatic_ = 0L;
}

void CryptographyPlugin::loadSettings()
{
	CryptographyConfig c;

	mPrivateKeyID = c.fingerprint();
	mAskPassPhraseOnStartup = c.askPassphraseOnStartup();
	mCachePassPhrase = c.cacheMode();
	mCacheTime = c.cacheTime();
}

CryptographyPlugin* CryptographyPlugin::plugin()
{
	return pluginStatic_ ;
}

CryptographyPlugin* CryptographyPlugin::pluginStatic_ = 0L;

QString CryptographyPlugin::cachedPass()
{
	return pluginStatic_->m_cachedPass;
}

void CryptographyPlugin::setCachedPass ( const QString& p )
{
	if ( pluginStatic_->mCachePassPhrase == CryptographyConfig::Never )
		return;
	if ( pluginStatic_->mCachePassPhrase == CryptographyConfig::Time )
	{
		pluginStatic_->m_cachedPass_timer->setSingleShot ( false );
		pluginStatic_->m_cachedPass_timer->start ( pluginStatic_->mCacheTime * 60000 );
	}

	pluginStatic_->m_cachedPass=p;
}

void CryptographyPlugin::slotIncomingMessage ( Kopete::Message& msg )
{
	QString body = msg.plainBody();

	int opState;
	if ( !body.startsWith ( QString::fromLatin1 ( "-----BEGIN PGP MESSAGE----" ) )
	        || !body.contains ( QString::fromLatin1 ( "-----END PGP MESSAGE----" ) ) )
	{
		if ( body.contains ( QRegExp ( KIconLoader::global()->iconPath ( "signature", K3Icon::Small ).remove ( QRegExp ( "signature.*" ) ) + "signature\\..*|bad_signature\\..*|encrypted\\..*" ) ) )
		{
			msg.setHtmlBody ( i18n ( "Cryptography plugin refuses to show the most recent message because it contains an attempt to forge an encrypted or signed message" ) );
			msg.setType ( Kopete::Message::TypeAction );
			msg.addClass ( "cryptography:encrypted" );
		}
		return;
	}

	body = GpgInterface::decryptText ( body, mPrivateKeyID, opState );

	if ( !body.isEmpty() )
	{
		if ( body.contains ( QRegExp ( KIconLoader::global()->iconPath ( "signature", K3Icon::Small ).remove ( QRegExp ( "signature.*" ) ) + "signature\\..*|bad_signature\\..*|encrypted\\..*" ) ) )
		{
			msg.setHtmlBody ( i18n ( "Cryptography plugin refuses to show the most recent message because it contains an attempt to forge an encrypted or signed message" ) );
			msg.setType ( Kopete::Message::TypeAction );
			msg.addClass ( "cryptography:encrypted" );
			return;
		}

		if ( opState & GpgInterface::GoodSig )
			body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "signature", K3Icon::Small ) + "\">&nbsp;&nbsp;" );

		if ( ( opState & GpgInterface::ErrorSig ) || ( opState & GpgInterface::BadSig ) )
			body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "bad_signature", K3Icon::Small ) + "\">&nbsp;&nbsp;" );

		if ( opState & GpgInterface::Decrypted )
			body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ( "encrypted", K3Icon::Small ) + "\">&nbsp;&nbsp;" );

		msg.setHtmlBody ( body );

		msg.addClass ( "cryptography:encrypted" );
	}
	return;
}

void CryptographyPlugin::slotOutgoingMessage ( Kopete::Message& msg )
{
	if ( msg.direction() != Kopete::Message::Outbound )
		return;

	QStringList keys;
	QList<Kopete::Contact*> contactlist = msg.to();
	bool signing = mGui->signing();
	bool encrypting = mGui->encrypting();

	if ( encrypting )
	{
		foreach ( Kopete::Contact *c, contactlist )
		{
			QString tmpKey;
			if ( c->metaContact() )
			{
				if ( c->metaContact()->pluginData ( this, "encrypt_messages" ) == "off" )
					return;
				tmpKey = c->metaContact()->pluginData ( this, "gpgKey" );
			}
			if ( tmpKey.isEmpty() )
			{
				kDebug ( 14303 ) << "empty key";
				KMessageBox::sorry ( Kopete::UI::Global::mainWidget(), i18n ( "You have not chosen an encryption key for one or more recipients" ) );
				return;
			}
			keys.append ( tmpKey );
		}
		// encrypt to self so we can decrypt during slotIncomingMessage()
		keys.append ( mPrivateKeyID );
		QString key = keys.join ( " " );

		if ( key.isEmpty() )
			return;

		QString original = msg.plainBody();

		QString encryptOptions;
		encryptOptions+=" --always-trust ";
		encryptOptions+=" --armor ";

		QString resultat = GpgInterface::encryptText ( original,key,encryptOptions, signing, mPrivateKeyID );
		if ( !resultat.isEmpty() )
			msg.setPlainBody ( resultat );
		else
			kDebug ( 14303 ) << "empty result";
	}
	else if ( signing )
	{
		QString result = GpgInterface::signText ( msg.plainBody(), mPrivateKeyID );
		if ( !result.isEmpty() )
			msg.setPlainBody ( result );
	}
}

void CryptographyPlugin::slotSelectContactKey()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	if ( !m )
		return;
	QString key = m->pluginData ( this, "gpgKey" );
	CryptographySelectUserKey opts ( key, m );
	opts.exec();
	if ( opts.result() )
	{
		key = opts.publicKey();
		m->setPluginData ( this, "gpgKey", key );
	}
}

void CryptographyPlugin::slotForgetCachedPass()
{
	m_cachedPass=QString();
	m_cachedPass_timer->stop();
}

void CryptographyPlugin::slotAskPassphraseOnStartup()
{
	if ( mAskPassPhraseOnStartup && !mPrivateKeyID.isEmpty() )
	{
		KPasswordDialog dialog ( Kopete::UI::Global::mainWidget() );
		dialog.setPrompt ( i18n ( "Enter password for GPG encryption and signing key" ) );
		dialog.exec ();
		setCachedPass ( dialog.password() );
	}
}

void CryptographyPlugin::slotNewKMM ( Kopete::ChatSession *KMM )
{
	connect ( this , SIGNAL ( destroyed ( QObject* ) ) ,
	          mGui = new CryptographyGUIClient ( KMM ) , SLOT ( deleteLater() ) );

	QString protocol ( KMM->protocol()->metaObject()->className() );
	if ( mGui->m_encAction->isChecked() )
		if ( ! supportedProtocols().contains ( protocol ) )
			KMessageBox::information ( 0, i18n ( "This protocol may not work with messages that are encrypted. This is because encrypted messages are very long, and the server or peer may reject them due to their length. To avoid being signed off or your account being warned or temporarily suspended, turn off encryption." ), i18n ( "Cryptography Unsupported Protocol" ), "Warn about unsupported " + QString ( KMM->protocol()->metaObject()->className() ) );
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
	if ( keys.at(0) == keys.at(1) )
		keys.removeAt (1);
	
	kDebug ( 14303 ) << "keys found in address book for contact " << addressee.assembledName() << ": " << keys << endl;
	
	return keys;
}

#include "cryptographyplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

