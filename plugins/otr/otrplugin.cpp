/*************************************************************************
 * Copyright <2007>  <Michael Zanetti> <michael_zanetti@gmx.net>         *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/ 

#include "otrplugin.h"
#include "otrguiclient.h"
#include "otrlchatinterface.h"
#include "kopete_otr.h"

#include <qtimer.h>
#include <qregexp.h>
#include <qfile.h>
#include <qcolor.h>

#include <kdebug.h>
#include <kaction.h>
#include <kconfig.h>
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
#include <ui/kopeteview.h>


/**
  * @author Michael Zanetti
  */


K_PLUGIN_FACTORY ( OTRPluginFactory, registerPlugin<OTRPlugin>(); )
K_EXPORT_PLUGIN ( OTRPluginFactory ( "kopete_otr" ) )


OTRPlugin::OTRPlugin ( QObject *parent, const QVariantList &/*args*/ )
	: Kopete::Plugin ( OTRPluginFactory::componentData(), parent )
{

	kDebug(14318) << "OTR Plugin loading...";

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
	otrPolicyMenu = new KSelectAction( KIcon("object-locked"), i18nc( "@item:inmenu", "&OTR Policy" ), this );
	actionCollection()->addAction( "otr_policy", otrPolicyMenu );

	KAction *separatorAction = new KAction( otrPolicyMenu );
	separatorAction->setSeparator( true );

	otrPolicyMenu->addAction( i18nc( "@item:inmenu Use the default encryption mode specified in settings dialog", "&Default") );
	otrPolicyMenu->addAction( separatorAction );
	otrPolicyMenu->addAction( i18nc( "@item:inmenu Always encrypt messages", "Al&ways" ) );
	otrPolicyMenu->addAction( i18nc( "@item:inmenu Use the opportunistic encryption mode", "&Opportunistic") );
	otrPolicyMenu->addAction( i18nc( "@item:inmenu Use the manual encryption mode", "&Manual") );
	otrPolicyMenu->addAction( i18nc( "@item:inmenu Never encrypt messages", "Ne&ver") );

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
	kDebug(14318) << "Exiting OTR plugin";
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
	//If there is more than one member it is a MUC
	// Also don't add the Button on an IRC window!
	if( KMM->members().count() == 1 && (KMM->protocol()) && ( KMM->protocol()->pluginId() != "IRCProtocol" ) ){
		new OtrGUIClient( KMM );
	}
}


void OTRPlugin::slotOutgoingMessage( Kopete::Message& msg )
{
	if( msg.direction() == Kopete::Message::Outbound ){
//        kDebug(14318) << "Outgoing message: Plain: " << msg.plainBody() << "body:" << msg.escapedBody();
		QString msgBody = msg.parsedBody();
		QString cacheBody = msg.parsedBody();
		QString accountId = msg.manager()->account()->accountId();
		Kopete::Contact *contact = msg.to().first();
		
		int encryptionState = otrlChatInterface->encryptMessage( &msgBody, accountId, msg.manager()->account()->protocol()->displayName(), contact->contactId(), msg.manager() );
		if(encryptionState == -1){
			msg.setPlainBody(i18n("An error occurred while encrypting the message."));
		} else if(encryptionState == 0){
//            kDebug(14318) << "Encrypted successfully";
			msg.setPlainBody( msgBody );
			msg.setType(Kopete::Message::TypeNormal);
			if( !msg.plainBody().isEmpty() ){
				messageCache.insert( msgBody, cacheBody );
			} else {
				messageCache.insert( "!OTR:MsgDelByOTR", cacheBody );
			}
		} else if(encryptionState == 1){
//            kDebug(14318) << "Tagged plaintext!";
			msg.setHtmlBody(msgBody);
			messageCache.insert( msgBody, cacheBody );
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
			msg.setPlainBody( i18nc( "@info:status", "Your policy settings do not allow encrypted sessions to this contact." ));
			msg.setDirection( Kopete::Message::Internal);
			session->appendMessage( msg );
		} else {
			QString body = otrlChatInterface->getDefaultQuery( session->account()->accountId() );
			Kopete::Message msg1( session->account()->myself(), session->members().first());
			msg1.setPlainBody( QString( body ) );
			msg1.setDirection( Kopete::Message::Outbound );
			if( otrlChatInterface->privState( session ) > 0 ){
				body = i18nc( "@info:status", "Attempting to refresh the OTR session with <b>%1</b>...", otrlChatInterface->formatContact( session->members().first()->contactId()) );
			} else {
				body = i18nc( "@info:status", "Attempting to start a private OTR session with <b>%1</b>...", otrlChatInterface->formatContact( session->members().first()->contactId() ));
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
	if ( !plugin ){
		MessageHandler::handleMessage( event );
		return;
	}

	Kopete::Message msg = event->message();
//	Kopete::ChatSession *session = msg.manager();
	QMap<QString, QString> messageCache = plugin->getMessageCache();

	if( msg.direction() == Kopete::Message::Inbound ){
		if( msg.type() == Kopete::Message::TypeFileTransferRequest )
		{
			// file transfers aren't encrypted. Proceed with next plugin
			MessageHandler::handleMessage( event );
			return;
		}
		QString body = msg.plainBody();
		QString accountId = msg.manager()->account()->accountId();
		QString contactId = msg.from()->contactId();
		int retValue = OtrlChatInterface::self()->decryptMessage( &body, accountId, msg.manager()->account()->protocol()->displayName(), contactId, msg.manager() );
		msg.setHtmlBody( body );
		if( (retValue == 2) | OtrlChatInterface::self()->shouldDiscard( msg.plainBody() ) ){
			// internal OTR message
			event->discard();
			return;
		} else if(retValue == 1){
			// plaintext message. Proceed with next plugin
			MessageHandler::handleMessage( event );
			return;
		}
	} else if( msg.direction() == Kopete::Message::Outbound ){
//        kDebug(14318) << "searching cache for" << msg.plainBody();
		if( messageCache.contains( msg.plainBody() ) ){
			msg.setHtmlBody( messageCache[msg.plainBody()] );
			messageCache.remove( messageCache[msg.plainBody()] );
			if(messageCache.count() > 5) messageCache.clear();
		}
		// Check if Message is an OTR message. Should it be discarded or shown?
		if( OtrlChatInterface::self()->shouldDiscard( msg.plainBody() ) ){
			event->discard();
			kDebug(14318) << "OTR: discarding message";
			return;
		}
		// If the message is sent while a Finished state libotr deletes the messagetext.
		// This prevents the empty message from being shown in our chatwindow
		if( msg.plainBody().isEmpty() ){
			event->discard();
			if(messageCache.contains("!OTR:MsgDelByOTR")){
				msg.setPlainBody(messageCache["!OTR:MsgDelByOTR"]);
				msg.manager()->view()->setCurrentMessage(msg);
				messageCache.remove("!OTR:MsgDelByOTR");
			}
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
		otrPolicyMenu->setCurrentItem( policy.toInt( &noerr, 10 ) + 1); // +1 because of the Separator
	else
		otrPolicyMenu->setCurrentItem( 0 );

}

void OTRPlugin::slotSetPolicy(){
	kDebug(14318) << "Setting contact policy";
	Kopete::MetaContact *metaContact = Kopete::ContactList::self()->selectedMetaContacts().first();
	if( metaContact ){
		metaContact->setPluginData( this, "otr_policy", QString::number( otrPolicyMenu->currentItem() - 1 ) ); // -1 because of the Separator
	}
	kDebug(14318) << "Selected policy: " << otrPolicyMenu->currentItem();
}

void OTRPlugin::slotSecuritySate(Kopete::ChatSession *session, int state)
{
	emitGoneSecure(session, state);
}

#include "otrplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

