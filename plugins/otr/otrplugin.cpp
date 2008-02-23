/***************************************************************************
 *   Copyright (C) 2007 by Michael Zanetti                                 *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <kgenericfactory.h>
#include <kselectaction.h>
#include <kactioncollection.h>

#include <kopetemetacontact.h>
#include <kopetecontactlist.h>
#include <kopetechatsessionmanager.h>
#include <kopetesimplemessagehandler.h>
#include <kopeteuiglobal.h>
#include <kopetecontact.h>
#include <kopetemessage.h>
#include <kopeteaccount.h>
#include <kopeteaccountmanager.h>
#include <kopetemessageevent.h>
#include <kopeteprotocol.h>

#include "otrplugin.h"
#include "otrguiclient.h"
#include "otrlchatinterface.h"
#include "kopete_otr.h"

/**
  * @author Michael Zanetti
  */


K_PLUGIN_FACTORY ( OTRPluginFactory, registerPlugin<OTRPlugin>(); )
K_EXPORT_PLUGIN ( OTRPluginFactory ( "kopete_otr" ) )


OTRPlugin::OTRPlugin ( QObject *parent, const QVariantList &/*args*/ )
	: Kopete::Plugin ( OTRPluginFactory::componentData(), parent )
{

	kDebug() << "OTR Plugin loading...";

	if( !pluginStatic_ )
		pluginStatic_=this;
	
	m_inboundHandler = new OtrMessageHandlerFactory(this);

	connect( Kopete::ChatSessionManager::self(), SIGNAL( aboutToSend( Kopete::Message & ) ),
		SLOT( slotOutgoingMessage( Kopete::Message & ) ) );

	connect( Kopete::ChatSessionManager::self(), SIGNAL( chatSessionCreated( Kopete::ChatSession * ) ),
			 this, SLOT( slotNewChatSessionWindow( Kopete::ChatSession * ) ) );

	connect( this, SIGNAL( settingsChanged() ), this, SLOT( slotSettingsChanged() ) );



	//initialize the otrlib and create the interface object
	otrlChatInterface = OtrlChatInterface::self();
	otrlChatInterface->setPlugin(this);


	// Checking file Permissions
	OtrlChatInterface::self()->checkFilePermissions( QString( KGlobal::dirs()->saveLocation( "data", "kopete_otr/", true ) ) + "privkeys" );
	OtrlChatInterface::self()->checkFilePermissions( QString( KGlobal::dirs()->saveLocation( "data", "kopete_otr/", true ) ) + "fingerprints" );

	//setting the policy
	slotSettingsChanged();

	//adding menu to contaclists menubar and contacts popup menu
	otrPolicyMenu = new KSelectAction( KIcon("object-locked"), i18n( "&OTR Policy" ), this );
	actionCollection()->addAction( "otr_policy", otrPolicyMenu );

	KAction *separatorAction = new KAction( otrPolicyMenu );
	separatorAction->setSeparator( true );

	otrPolicyMenu->addAction( i18n("&Default") );
	otrPolicyMenu->addAction( separatorAction );
	otrPolicyMenu->addAction( i18n("Al&ways") );
	otrPolicyMenu->addAction( i18n("&Opportunistic") );
	otrPolicyMenu->addAction( i18n("&Manual") );
	otrPolicyMenu->addAction( i18n("Ne&ver") );

	otrPolicyMenu->setEnabled( false );

	connect( otrPolicyMenu, SIGNAL( triggered( int ) ), this, SLOT( slotSetPolicy() ) );
	connect( Kopete::ContactList::self(), SIGNAL( metaContactSelected( bool ) ), this, SLOT( slotSelectionChanged( bool ) ) );

	setXMLFile( "otrui.rc" );

	//Add GUI action to all already existing kmm 
	// (if the plugin is launched when kopete already runing)
	QList<Kopete::ChatSession*> sessions =
		 Kopete::ChatSessionManager::self()->sessions();
	QListIterator<Kopete::ChatSession*> it(sessions);
	while (it.hasNext()){
		slotNewChatSessionWindow(it.next());
	}
}

OTRPlugin::~OTRPlugin()
{
	delete m_inboundHandler;
	pluginStatic_ = 0L;
	kDebug() << "Exiting plugin";
}


OTRPlugin* OTRPlugin::plugin()
{
	return pluginStatic_ ;
}

OTRPlugin* OTRPlugin::pluginStatic_ = 0L;


void OTRPlugin::slotNewChatSessionWindow( Kopete::ChatSession *KMM )
{
	//Check if there is another user in the session.
	//If not it could be a Jabber-MUC
	//If there are more then one members it is a MUC
	// Also don't add the Button on an IRC window!
	if( KMM->members().count() == 1 && (KMM->protocol()) && ( KMM->protocol()->pluginId() != "IRCProtocol" ) ){
		new OtrGUIClient( KMM );
	}
}


void OTRPlugin::slotOutgoingMessage( Kopete::Message& msg )
{
	if( msg.direction() == Kopete::Message::Outbound ){
		QString plainBody = msg.plainBody();
		QString accountId = msg.manager()->account()->accountId();
		Kopete::Contact *contact = msg.to().first();
		
		QString encBody = otrlChatInterface->encryptMessage( plainBody, accountId, msg.manager()->account()->protocol()->displayName(), contact->contactId(), msg.manager() );
		msg.setPlainBody( encBody );
		msg.setType(Kopete::Message::TypeNormal);
		if( !msg.plainBody().isEmpty() ){
			messageCache.insert( encBody, plainBody  );
		}
	}
}

void  OTRPlugin::slotEnableOtr( Kopete::ChatSession *session, bool enable ){


	if( enable ){
		QString policy = session->members().first()->metaContact()->pluginData( OTRPlugin::plugin(), "otr_policy" );
		bool noerr;
		KopeteOtrKcfg::self()->readConfig();
		if( policy.toInt( &noerr, 10 ) == 4 || ( policy.toInt( &noerr, 10 ) == 0 && KopeteOtrKcfg::self()->rbNever() ) ){
			Kopete::Message msg(  session->account()->myself(), session->members());
			msg.setPlainBody( i18n( "Your policy settings do not allow encrypted sessions to this contact." ));
			msg.setDirection( Kopete::Message::Internal);
			session->appendMessage( msg );
		} else {
			QString body = otrlChatInterface->getDefaultQuery( session->account()->accountId() );
			Kopete::Message msg1( session->account()->myself(), session->members().first());
			msg1.setPlainBody( QString( body ) );
			msg1.setDirection( Kopete::Message::Outbound );
			if( otrlChatInterface->privState( session ) > 0 ){
				body = i18n("Attempting to refresh the OTR session with <b>%1</b>...", otrlChatInterface->formatContact( session->members().first()->contactId()) );
			} else {
				body = i18n("Attempting to start a private OTR session with <b>%1</b>...", otrlChatInterface->formatContact( session->members().first()->contactId() ));
			}
			Kopete::Message msg2( session->account()->myself(), session->members().first());
			msg2.setHtmlBody( body );
			msg2.setDirection( Kopete::Message::Internal );

			session->sendMessage( msg1 );
			session->appendMessage( msg2 );
		}
	} else {
		otrlChatInterface->disconnectSession( session );
	}

}

void OTRPlugin::slotVerifyFingerprint( Kopete::ChatSession *session ){
	otrlChatInterface->verifyFingerprint( session );
}

void OTRPlugin::slotSettingsChanged(){

	KopeteOtrKcfg::self()->readConfig();
	if( KopeteOtrKcfg::self()->rbAlways() ){
		otrlChatInterface->setPolicy( OTRL_POLICY_ALWAYS );
	} else if( KopeteOtrKcfg::self()->rbOpportunistic() ){
		otrlChatInterface->setPolicy( OTRL_POLICY_OPPORTUNISTIC );
	} else if( KopeteOtrKcfg::self()->rbManual() ){
		otrlChatInterface->setPolicy( OTRL_POLICY_MANUAL );
	} else if( KopeteOtrKcfg::self()->rbNever() ){
		otrlChatInterface->setPolicy( OTRL_POLICY_NEVER );
	} else {
		otrlChatInterface->setPolicy( OTRL_POLICY_DEFAULT );
	}
}

void OTRPlugin::emitGoneSecure( Kopete::ChatSession *session, int status){
	emit goneSecure( session, status );
}

QMap<QString, QString> OTRPlugin::getMessageCache(){
	return messageCache;
}

void OtrMessageHandler::handleMessage( Kopete::MessageEvent *event ){
	Kopete::Message msg = event->message();
//	Kopete::ChatSession *session = msg.manager();
	QMap<QString, QString> messageCache = OTRPlugin::plugin()->getMessageCache();
	
	if( msg.direction() == Kopete::Message::Inbound ){
		QString body = msg.plainBody();
		QString accountId = msg.manager()->account()->accountId();
		QString contactId = msg.from()->contactId();
		int ignoremessage = OtrlChatInterface::self()->decryptMessage( &body, accountId, msg.manager()->account()->protocol()->displayName(), contactId, msg.manager() );
		msg.setHtmlBody( body );
		if( ignoremessage | OtrlChatInterface::self()->shouldDiscard( msg.plainBody() ) ){
			event->discard();
			return;
		}
	} else if( msg.direction() == Kopete::Message::Outbound ){
		if( messageCache.contains( msg.plainBody() ) ){
			msg.setPlainBody( messageCache[msg.plainBody()] );
			messageCache.remove( messageCache[msg.plainBody()] );
			if(messageCache.count() > 5) messageCache.clear();
		}
		// Check if Message is an OTR message. Should it be discarded or shown?
		if( OtrlChatInterface::self()->shouldDiscard( msg.plainBody() ) ){
			event->discard();
			kDebug() << "OTR: discarding message";
			return;
		}
		// If the message is sent while a Finished state libotr deletes the messagetext.
		// This prevents the empty message from beeing shown in our chatwindow
		if( msg.plainBody().isEmpty() ){
			event->discard();
			return;
		}
	}
	
	event->setMessage( msg );

	MessageHandler::handleMessage( event );
}


void OTRPlugin::slotSelectionChanged( bool single){
	otrPolicyMenu->setEnabled( single );

	if ( !single )
		return;

	Kopete::MetaContact *metaContact = Kopete::ContactList::self()->selectedMetaContacts().first();

	QString policy = metaContact->pluginData( this, "otr_policy" );

	bool noerr;
	if ( !policy.isEmpty() && policy != "null" )
		otrPolicyMenu->setCurrentItem( policy.toInt( &noerr, 10 ));
	else
		otrPolicyMenu->setCurrentItem( 0 );

}

void OTRPlugin::slotSetPolicy(){
	kDebug() << "Setting contact policy";
	Kopete::MetaContact *metaContact = Kopete::ContactList::self()->selectedMetaContacts().first();
	if( metaContact ){
		metaContact->setPluginData( this, "otr_policy", QString::number( otrPolicyMenu->currentItem() ) );		
	}
	kDebug() << "Selected policy: " << otrPolicyMenu->currentItem();
}

void OTRPlugin::slotSecuritySate(Kopete::ChatSession *session, int state)
{
	emitGoneSecure(session, state);
}

#include "otrplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

