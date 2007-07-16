/*
    cryptographyplugin.cpp  -  description

	Copyright (c) 2002-2004 by Olivier Goffart <ogoffart@kde.org>

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

#include <QTextDocument>
#include <qtimer.h>
#include <qregexp.h>
#include <QList>
#include <QByteArray>

#include <kdebug.h>
#include <kaction.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <kdeversion.h>
#include <kaboutdata.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopetesimplemessagehandler.h"
#include "kopeteuiglobal.h"
#include "kopetecontact.h"

#include "cryptographyplugin.h"
#include "cryptographyselectuserkey.h"
#include "cryptographyguiclient.h"

#include "kgpginterface.h"
#include <kactioncollection.h>

//This regexp try to match an HTML text,  but only some authorized tags.
// used in slotIncomingMessage
//There are not rules to know if the test should be sent in html or not.
//In Jabber, the JEP says it's not. so we don't use richtext in our message, but some client did.
//We limit the html to some basis tag to limit security problem (bad links)
//    - Olivier
const QRegExp CryptographyPlugin::isHTML( QString::fromLatin1( "^[^<>]*(</?(html|body|br|p|font|center|b|i|u|span|div|pre)(>|[\\s/][^><]*>)[^><]*)+$" ) , Qt::CaseInsensitive );

typedef KGenericFactory<CryptographyPlugin> CryptographyPluginFactory;
static const KAboutData aboutdata("kopete_cryptography", 0, ki18n("Cryptography") , "1.0" );
K_EXPORT_COMPONENT_FACTORY( kopete_cryptography, CryptographyPluginFactory( &aboutdata )  )

CryptographyPlugin::CryptographyPlugin( QObject *parent, const QStringList & /* args */ )
: Kopete::Plugin( CryptographyPluginFactory::componentData(), parent ),
		m_cachedPass()
{
	if( !pluginStatic_ )
		pluginStatic_=this;

	m_inboundHandler = new Kopete::SimpleMessageHandlerFactory( Kopete::Message::Inbound,
		Kopete::MessageHandlerFactory::InStageToSent, this, SLOT( slotIncomingMessage( Kopete::Message& ) ) );
	connect( Kopete::ChatSessionManager::self(),
		SIGNAL( aboutToSend( Kopete::Message & ) ),
		SLOT( slotOutgoingMessage( Kopete::Message & ) ) );

	m_cachedPass_timer = new QTimer(this);
	m_cachedPass_timer->setObjectName("m_cachedPass_timer");
	QObject::connect(m_cachedPass_timer, SIGNAL(timeout()), this, SLOT(slotForgetCachedPass() ));


	KAction *action=new KAction( KIcon("encrypted"), i18n("&Select Cryptography Public Key..."), this );
        actionCollection()->addAction( "contactSelectKey", action );
	connect( action, SIGNAL(triggered(bool)), this, SLOT(slotSelectContactKey()) );
	connect ( Kopete::ContactList::self() , SIGNAL( metaContactSelected(bool)) , action , SLOT(setEnabled(bool)));
	action->setEnabled(Kopete::ContactList::self()->selectedMetaContacts().count()==1 );

	setXMLFile("cryptographyui.rc");
	loadSettings();
	connect(this, SIGNAL(settingsChanged()), this, SLOT( loadSettings() ) );

	connect( Kopete::ChatSessionManager::self(), SIGNAL( chatSessionCreated( Kopete::ChatSession * )) , SLOT( slotNewKMM( Kopete::ChatSession * ) ) );
	//Add GUI action to all already existing kmm (if the plugin is launched when kopete already rining)
	QList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
	foreach(Kopete::ChatSession *session, sessions)
	{
		slotNewKMM(session);
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
	mAskPassPhrase = c.askPassPhrase();
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

void CryptographyPlugin::setCachedPass(const QByteArray& p)
{
	if(pluginStatic_->mCachePassPhrase == CryptographyConfig::Never)
		return;
	if(pluginStatic_->mCachePassPhrase == CryptographyConfig::Time)
	{
		pluginStatic_->m_cachedPass_timer->setSingleShot( false );
		pluginStatic_->m_cachedPass_timer->start(pluginStatic_->mCacheTime * 60000);
	}

	pluginStatic_->m_cachedPass=p;
}

bool CryptographyPlugin::passphraseHandling()
{
	return !pluginStatic_->mAskPassPhrase;
}


/*KActionCollection *CryptographyPlugin::customChatActions(Kopete::ChatSession *KMM)
{
	delete m_actionCollection;

	m_actionCollection = new KActionCollection(this);
	KAction *actionTranslate = new KAction( i18n ("Translate"), 0,
		this, SLOT( slotTranslateChat() ), m_actionCollection, "actionTranslate" );
	m_actionCollection->insert( actionTranslate );

	m_currentChatSession=KMM;
	return m_actionCollection;
}*/

void CryptographyPlugin::slotIncomingMessage( Kopete::Message& msg )
{
 	QString body = msg.plainBody();
	if( !body.startsWith( QString::fromLatin1("-----BEGIN PGP MESSAGE----") )
		 || !body.contains( QString::fromLatin1("-----END PGP MESSAGE----") ) )
		return;

	if( msg.direction() != Kopete::Message::Inbound )
	{
/*		if ( m_cachedMessages.contains( body ) )
		{
			plainBody = m_cachedMessages[ body ];
			m_cachedMessages.remove( body );
		}
		else
		{
		*/			body = GpgInterface::decryptText( msg.plainBody(), mPrivateKeyID );
//		}

		if( !body.isEmpty() )
		{
			body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ("encrypted", K3Icon::Small) + "\">" );
			kDebug (14303) << k_funcinfo << "body is " << body << endl;
			msg.setHtmlBody(body);
			msg.addClass("cryptography:encrypted");
		}

		//if there are too many messages in cache, clear the cache
		if(m_cachedMessages.count() > 5)
			m_cachedMessages.clear();

		return;
	}


	//the Message::unescape is there because client like fire replace linebreak with <BR> to work even if the protocol doesn't allow newlines (IRC)
	// cf http://fire.sourceforge.net/forums/viewtopic.php?t=174  and Bug #96052
	// Note: I believe Message's smart HTML handling makes this irrelvant. We shall see. (C Connell)
	// TODO remove this code
//	if(body.contains("<"))
//		body= Kopete::Message::unescape(body);

	body = GpgInterface::decryptText( msg.plainBody(), mPrivateKeyID );

	if( !body.isEmpty() )
	{
		body.prepend ( "<img src=\"" + KIconLoader::global()->iconPath ("encrypted", K3Icon::Small) + "\">" );
		kDebug (14303) << k_funcinfo << "body is " << body << endl;
		msg.setHtmlBody(body);
		msg.addClass("cryptography:encrypted");
	}
}

void CryptographyPlugin::slotOutgoingMessage( Kopete::Message& msg )
{
	if(msg.direction() != Kopete::Message::Outbound)
		return;

	QStringList keys;
	QList<Kopete::Contact*> contactlist = msg.to();
	foreach(Kopete::Contact *c, contactlist)
	{
		QString tmpKey;
		if( c->metaContact() )
		{
			if(c->metaContact()->pluginData( this, "encrypt_messages"  ) == "off" )
				return;
			tmpKey = c->metaContact()->pluginData( this, "gpgKey" );
		}
		if( tmpKey.isEmpty() )
		{
			return;
		}
		keys.append( tmpKey );
	}
	// encrypt to self so we can decrypt during slotIncomingMessage()
	keys.append( mPrivateKeyID );
	
	QString key = keys.join( " " );

	if(key.isEmpty())
	{
		kDebug(14303) << k_funcinfo << "empty key" << endl;
		KMessageBox::sorry (Kopete::UI::Global::mainWidget(), i18n("You have not chosen an encryption key for one or more recipients") );
		return;
	}

	QString original=msg.plainBody();

	QString encryptOptions="";
	encryptOptions+=" --always-trust ";
	encryptOptions+=" --armor ";

	QString resultat = GpgInterface::encryptText(original,key,encryptOptions);
	if (!resultat.isEmpty())
	{
		msg.setPlainBody(resultat);
		m_cachedMessages.insert(resultat,original);
	}
	else
		kDebug(14303) << "CryptographyPlugin::slotOutgoingMessage: empty result" <<endl;
}

void CryptographyPlugin::slotSelectContactKey()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	if(!m)
		return;
	QString key = m->pluginData( this, "gpgKey" );
	CryptographySelectUserKey *opts = new CryptographySelectUserKey( key, m );
	opts->exec();
	if( opts->result() )
	{
		key = opts->publicKey();
		m->setPluginData( this, "gpgKey", key );
	}
	delete opts;
}

void CryptographyPlugin::slotForgetCachedPass()
{
	m_cachedPass=QByteArray();
	m_cachedPass_timer->stop();
}

void CryptographyPlugin::slotNewKMM(Kopete::ChatSession *KMM)
{
	connect(this , SIGNAL( destroyed(QObject*)) ,
			new CryptographyGUIClient(KMM) , SLOT(deleteLater()));
}

#include "cryptographyplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

