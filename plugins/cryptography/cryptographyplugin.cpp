/***************************************************************************
                          cryptographyplugin.cpp  -  description
                             -------------------
    begin                : jeu nov 14 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
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
 
#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "kopete.h"
#include "kopetemessage.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"

#include "cryptographyplugin.h"
#include "cryptographypreferences.h"

#include "kgpginterface.h"


K_EXPORT_COMPONENT_FACTORY( kopete_cryptography, KGenericFactory<CryptographyPlugin> );

CryptographyPlugin::CryptographyPlugin( QObject *parent, const char *name,
	const QStringList &/*args*/ )
: KopetePlugin( parent, name ) ,
		m_cachedPass()
{
	if( !pluginStatic_ )
		pluginStatic_=this;

	//TODO: found a pixmap
	m_prefs = new CryptographyPreferences ( "kgpg", this );

	connect( kopeteapp, SIGNAL(aboutToDisplay(KopeteMessage&)),
		 SLOT(slotIncomingMessage(KopeteMessage&)) );
	connect( kopeteapp, SIGNAL(aboutToSend(KopeteMessage&)),
		 SLOT(slotOutgoingMessage(KopeteMessage&)) );

	m_collection=0l;
	m_currentMetaContact=0L;

}

CryptographyPlugin::~CryptographyPlugin()
{
	pluginStatic_ = 0L;
	delete m_collection;
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
	pluginStatic_->m_cachedPass=p;
}


void CryptographyPlugin::init()
{
}

bool CryptographyPlugin::unload()
{
	return true;
}

bool CryptographyPlugin::serialize( KopeteMetaContact *metaContact,
			  QStringList &strList  ) const
{
	if ( !m_keyMap.contains( metaContact ) )
		return false;

	strList<<  m_keyMap[ metaContact ] ;
	return true;
}

void CryptographyPlugin::deserialize( KopeteMetaContact *metaContact, const QStringList& data )
{
	m_keyMap[ metaContact ] = data.first();
}

KActionCollection *CryptographyPlugin::customContextMenuActions(KopeteMetaContact *m)
{
	delete m_collection;

	m_collection = new KActionCollection(this);

	KAction *action=new KAction( i18n("&Select Cryptography public key"), "kgpg", 0, this, SLOT (slotSelectContactKey()), m_collection);

	m_collection->insert(action);
	m_currentMetaContact=m;
	return m_collection;
}

/*KActionCollection *CryptographyPlugin::customChatActions(KopeteMessageManager *KMM)
{
	delete m_actionCollection;

	m_actionCollection = new KActionCollection(this);
	KAction *actionTranslate = new KAction( i18n ("Translate"), 0,
		this, SLOT( slotTranslateChat() ), m_actionCollection, "actionTranslate" );
	m_actionCollection->insert( actionTranslate );

	m_currentMessageManager=KMM;
	return m_actionCollection;
}*/

void CryptographyPlugin::slotIncomingMessage( KopeteMessage& msg )
{
	QString body=msg.plainBody();
	if(!body.startsWith("-----BEGIN PGP MESSAGE----"))
		return;

	if(msg.direction() != KopeteMessage::Inbound)
	{
		msg.setBody("<table width=\"100%\" border=0 cellspacing=0 cellpadding=0><tr bgcolor=\"#41FFFF\"><td><font size=\"-1\"><b>"+i18n("Outgoing Encrypted Message")+"</b></font></td></tr><tr bgcolor=\"#DDFFFF\"><td>"+i18n("TODO: Show original Message")+"</td></tr></table>"
				,KopeteMessage::RichText);
		return;
	}

	body=KgpgInterface::KgpgDecryptText(body, m_prefs->privateKey());

	body="<table width=\"100%\" border=0 cellspacing=0 cellpadding=0><tr bgcolor=\"#41FF41\"><td><font size=\"-1\"><b>"+i18n("Incomming Encrypted Message")+"</b></font></td></tr><tr bgcolor=\"#DDFFDD\"><td>"+QStyleSheet::escape(body)+"</td></tr></table>";
	msg.setBody(body,KopeteMessage::RichText);

}

void CryptographyPlugin::slotOutgoingMessage( KopeteMessage& msg )
{
	if(msg.direction() != KopeteMessage::Outbound)
		return;
	if(msg.to().count()!=1)
	{
		kdDebug() << "CryptographyPlugin::slotOutgoingMessage: TODO: group chat not yet implemented" <<endl;
		return;
	}
	KopeteMetaContact *m=msg.to().first()->metaContact();

	if(!m_keyMap.contains(m))
	{
		kdDebug() << "CryptographyPlugin::slotOutgoingMessage: no key selected for this contact" <<endl;
		return;
	}


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

// if (selec==NULL) {KMessageBox::sorry(0,i18n("You have not choosen an encryption key..."));return;}

	QString resultat=KgpgInterface::KgpgEncryptText(msg.plainBody(),m_keyMap[m],encryptOptions);
	if (resultat!="")
		msg.setBody(resultat,KopeteMessage::PlainText);
}

#include "popuppublic.h"

void CryptographyPlugin::slotSelectContactKey()
{
	popupPublic *dialogue=new popupPublic(0L/*this*/, "public_keys", 0,false);
	connect(dialogue,SIGNAL(selectedKey(QString &,bool,bool,bool,bool)),this,SLOT(setKey(QString &)));
	dialogue->exec();
	delete dialogue;
}

void CryptographyPlugin::setKey(QString &keyId)
{
	if(  m_currentMetaContact)
	{
		m_keyMap[ m_currentMetaContact ]= keyId;
	}
}

#include "cryptographyplugin.moc"

