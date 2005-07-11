/***************************************************************************
                          cryptographyplugin.cpp  -  description
                             -------------------
    begin                : jeu nov 14 2002
    copyright            : (C) 2002-2004 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstylesheet.h>
#include <qtimer.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kaction.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <kdeversion.h>
#include <kaboutdata.h>

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

//This regexp try to match an HTML text,  but only some authorized tags.
// used in slotIncomingMessage
//There are not rules to know if the test should be sent in html or not.
//In Jabber, the JEP says it's not. so we don't use richtext in our message, but some client did.
//We limit the html to some basis tag to limit security problem (bad links)
//    - Olivier
const QRegExp CryptographyPlugin::isHTML( QString::fromLatin1( "^[^<>]*(</?(html|body|br|p|font|center|b|i|u|span|div|pre)(>|[\\s/][^><]*>)[^><]*)+$" ) , false );

typedef KGenericFactory<CryptographyPlugin> CryptographyPluginFactory;
static const KAboutData aboutdata("kopete_cryptography", I18N_NOOP("Cryptography") , "1.0" );
K_EXPORT_COMPONENT_FACTORY( kopete_cryptography, CryptographyPluginFactory( &aboutdata )  )

CryptographyPlugin::CryptographyPlugin( QObject *parent, const char *name, const QStringList & /* args */ )
: Kopete::Plugin( CryptographyPluginFactory::instance(), parent, name ),
		m_cachedPass()
{
	if( !pluginStatic_ )
		pluginStatic_=this;

	m_inboundHandler = new Kopete::SimpleMessageHandlerFactory( Kopete::Message::Inbound,
		Kopete::MessageHandlerFactory::InStageToSent, this, SLOT( slotIncomingMessage( Kopete::Message& ) ) );
	connect( Kopete::ChatSessionManager::self(),
		SIGNAL( aboutToSend( Kopete::Message & ) ),
		SLOT( slotOutgoingMessage( Kopete::Message & ) ) );

	m_cachedPass_timer = new QTimer(this, "m_cachedPass_timer" );
	QObject::connect(m_cachedPass_timer, SIGNAL(timeout()), this, SLOT(slotForgetCachedPass() ));


	KAction *action=new KAction( i18n("&Select Cryptography Public Key..."), "encrypted", 0, this, SLOT (slotSelectContactKey()), actionCollection() , "contactSelectKey");
	connect ( Kopete::ContactList::self() , SIGNAL( metaContactSelected(bool)) , action , SLOT(setEnabled(bool)));
	action->setEnabled(Kopete::ContactList::self()->selectedMetaContacts().count()==1 );

	setXMLFile("cryptographyui.rc");
	loadSettings();
	connect(this, SIGNAL(settingsChanged()), this, SLOT( loadSettings() ) );
	
	connect( Kopete::ChatSessionManager::self(), SIGNAL( chatSessionCreated( Kopete::ChatSession * )) , SLOT( slotNewKMM( Kopete::ChatSession * ) ) );
	//Add GUI action to all already existing kmm (if the plugin is launched when kopete already rining)
	QValueList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
	for (QValueListIterator<Kopete::ChatSession*> it= sessions.begin(); it!=sessions.end() ; ++it)
	{
		slotNewKMM(*it);
	}

}

CryptographyPlugin::~CryptographyPlugin()
{
	delete m_inboundHandler;
	pluginStatic_ = 0L;
}

void CryptographyPlugin::loadSettings()
{
	KConfig *config = KGlobal::config();
	config->setGroup("Cryptography Plugin");

	mPrivateKeyID = config->readEntry("PGP_private_key");
	mAlsoMyKey = config->readBoolEntry("Also_my_key", false);

	if(config->readBoolEntry("Cache_Till_App_Close", false))
	  mCachePassPhrase = Keep;
	if(config->readBoolEntry("Cache_Till_Time", false))
	  mCachePassPhrase = Time;
	if(config->readBoolEntry("Cache_Never", false))
	  mCachePassPhrase = Never;
	mCacheTime = config->readNumEntry("Cache_Time", 15);
	mAskPassPhrase = config->readBoolEntry("No_Passphrase_Handling", false);
}

CryptographyPlugin* CryptographyPlugin::plugin()
{
	return pluginStatic_ ;
}

CryptographyPlugin* CryptographyPlugin::pluginStatic_ = 0L;

QCString CryptographyPlugin::cachedPass()
{
	return pluginStatic_->m_cachedPass;
}

void CryptographyPlugin::setCachedPass(const QCString& p)
{
	if(pluginStatic_->mCacheMode==Never)
		return;
	if(pluginStatic_->mCacheMode==Time)
		pluginStatic_->m_cachedPass_timer->start(pluginStatic_->mCacheTime * 60000, false);

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
		QString plainBody;
		if ( m_cachedMessages.contains( body ) )
		{
			plainBody = m_cachedMessages[ body ];
			m_cachedMessages.remove( body );
		}
		else
		{
			plainBody = KgpgInterface::KgpgDecryptText( body, mPrivateKeyID );
		}

		if( !plainBody.isEmpty() )
		{
			//Check if this is a RTF message before escaping it
			if( !isHTML.exactMatch( plainBody ) )
			{
				plainBody = QStyleSheet::escape( plainBody );

				//this is the same algoritm as in Kopete::Message::escapedBody();
				plainBody.replace( QString::fromLatin1( "\n" ), QString::fromLatin1( "<br/>" ) )
					.replace( QString::fromLatin1( "\t" ), QString::fromLatin1( "&nbsp;&nbsp;&nbsp;&nbsp;" ) )
					.replace( QRegExp( QString::fromLatin1( "\\s\\s" ) ), QString::fromLatin1( "&nbsp; " ) );
			}

			msg.setBody( QString::fromLatin1("<table width=\"100%\" border=0 cellspacing=0 cellpadding=0><tr><td class=\"highlight\"><font size=\"-1\"><b>")
				+ i18n("Outgoing Encrypted Message: ")
				+ QString::fromLatin1("</b></font></td></tr><tr><td class=\"highlight\">")
				+ plainBody
				+ QString::fromLatin1(" </td></tr></table>")
				, Kopete::Message::RichText );
		}

		//if there are too messages in cache, clear the cache
		if(m_cachedMessages.count() > 5)
			m_cachedMessages.clear();

		return;
	}


	//the Message::unescape is there because client like fire replace linebreak by <BR> to work even if the protocol doesn't allow newlines (IRC)
	// cf http://fire.sourceforge.net/forums/viewtopic.php?t=174  and Bug #96052
	if(body.contains("<"))  
		body= Kopete::Message::unescape(body);

	body = KgpgInterface::KgpgDecryptText( body, mPrivateKeyID );

	if( !body.isEmpty() )
	{
		//Check if this is a RTF message before escaping it
		if( !isHTML.exactMatch( body ) )
		{
			body = Kopete::Message::escape( body );
		}

		msg.setBody( QString::fromLatin1("<table width=\"100%\" border=0 cellspacing=0 cellpadding=0><tr><td class=\"highlight\"><font size=\"-1\"><b>")
			+ i18n("Incoming Encrypted Message: ")
			+ QString::fromLatin1("</b></font></td></tr><tr><td class=\"highlight\">")
			+ body
			+ QString::fromLatin1(" </td></tr></table>")
			, Kopete::Message::RichText );
	}

}

void CryptographyPlugin::slotOutgoingMessage( Kopete::Message& msg )
{
	if(msg.direction() != Kopete::Message::Outbound)
		return;

	QStringList keys;
	QPtrList<Kopete::Contact> contactlist = msg.to();
	for( Kopete::Contact *c = contactlist.first(); c; c = contactlist.next() )
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
		//	kdDebug( 14303 ) << "CryptographyPlugin::slotOutgoingMessage: no key selected for one contact" <<endl;
			return;
		}
		keys.append( tmpKey );
	}
	if(mAlsoMyKey) //encrypt also with the self key
		keys.append( mPrivateKeyID );

	QString key = keys.join( " " );

	if(key.isEmpty())
	{
		kdDebug(14303) << "CryptographyPlugin::slotOutgoingMessage: empty key" <<endl;
		return;
	}

	QString original=msg.plainBody();

	/* Code From KGPG */

	//////////////////              encode from editor
	QString encryptOptions="";

	//if (utrust==true)
		encryptOptions+=" --always-trust ";
	//if (arm==true)
		encryptOptions+=" --armor ";

	/* if (pubcryptography==true)
	{
		if (gpgversion<120) encryptOptions+=" --compress-algo 1 --cipher-algo cast5 ";
		else encryptOptions+=" --cryptography6 ";
	}*/

// if (selec==NULL) {KMessageBox::sorry(Kopete::UI::Global::mainWidget(),i18n("You have not chosen an encryption key..."));return;}

	QString resultat=KgpgInterface::KgpgEncryptText(original,key,encryptOptions);
	if (!resultat.isEmpty())
	{
		msg.setBody(resultat,Kopete::Message::PlainText);
		m_cachedMessages.insert(resultat,original);
	}
	else
		kdDebug(14303) << "CryptographyPlugin::slotOutgoingMessage: empty result" <<endl;

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
	m_cachedPass=QCString();
	m_cachedPass_timer->stop();
}

void CryptographyPlugin::slotNewKMM(Kopete::ChatSession *KMM) 
{
	connect(this , SIGNAL( destroyed(QObject*)) ,
			new CryptographyGUIClient(KMM) , SLOT(deleteLater()));
}



#include "cryptographyplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

