
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kapplication.h>
#include <kjsembed/kjsembedpart.h>
#include <kjsembed/jsconsolewidget.h>
#include <kjsembed/jsproxy.h>
#include <kjsembed/jsfactory.h>
#include <kjsembed/jssecuritypolicy.h>
#include <kjsembed/jsopaqueproxy.h>

#include "kopetemessagemanagerfactory.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopeteview.h"
#include "kopetecontactlist.h"
#include "wrappers/kopetemessage_imp.h"
#include "wrappers/kopeteonlinestatus_imp.h"
#include "javascriptplugin.h"
#include "javascriptconfig.h"

static void addContactToArray( KJSEmbed::KJSEmbedPart *jsEngine, KJS::Object &array,
	KopeteContact *contact, QPtrList<Status> &statusContainer )
{
	KJS::ExecState *state = jsEngine->globalExec();

	KJS::List args;
	KJS::Object object = jsEngine->factory()->createProxy( state, contact );
	Status *statusWrapper = new Status( contact->onlineStatus() );
	statusContainer.append( statusWrapper );
	jsEngine->addObject( statusWrapper, object, "onlineStatus" );
	args.append( object );

	KJS::Object pushMethod = array.get( state, "push" ).toObject( state );
	if( pushMethod.implementsCall() )
		pushMethod.call( state, array, args );
	else
		kdError() << "Array push method cannot be called" << endl;
}

static KJS::Object contactPtrList( KJSEmbed::KJSEmbedPart *jsEngine,
	const KopeteContactPtrList &list, QPtrList<Status> &statusContainer )
{
	KJS::Object arrayObject = jsEngine->interpreter()->builtinArray();
        KJS::Object retVal = arrayObject.construct( jsEngine->globalExec(), KJS::List() );

	for( QPtrListIterator<KopeteContact> it( list ); it.current(); ++it)
	{
        	addContactToArray( jsEngine, retVal, it.current(), statusContainer );
	}

	return retVal;
}

static KJS::Object contactDict( KJSEmbed::KJSEmbedPart *jsEngine,
	const QDict<KopeteContact> &list, QPtrList<Status> &statusContainer )
{
	KJS::Object arrayObject = jsEngine->interpreter()->builtinArray();
        KJS::Object retVal = arrayObject.construct( jsEngine->globalExec(), KJS::List() );

	for( QDictIterator<KopeteContact> it( list ); it.current(); ++it)
	{
		addContactToArray( jsEngine, retVal, it.current(), statusContainer );
	}

	return retVal;
}

static KJS::Object metaContactPtrList( KJSEmbed::KJSEmbedPart *jsEngine,
	const QPtrList<KopeteMetaContact> &list, QPtrList<Status> &statusContainer )
{
	KJS::Object arrayObject = jsEngine->interpreter()->builtinArray();
	KJS::ExecState *state = jsEngine->globalExec();
        KJS::Object retVal = arrayObject.construct( state, KJS::List() );

	for( QPtrListIterator<KopeteMetaContact> it( list ); it.current(); ++it)
	{
		KJS::List args;
		KJS::Object object = jsEngine->factory()->createProxy( state, it.current() );

		object.put( jsEngine->globalExec(), "contacts",
			contactPtrList( jsEngine, it.current()->contacts(), statusContainer )
		);

		KJS::Object pushMethod = retVal.get( state, "push" ).toObject( state );
		if( pushMethod.implementsCall() )
			pushMethod.call( state, retVal, args );
		else
			kdError() << "Array push method cannot be called" << endl;
	}

	return retVal;
}

typedef KGenericFactory<JavaScriptPlugin> JavaScriptPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_javascript, JavaScriptPluginFactory( "kopete_javascript" )  )

JavaScriptPlugin* JavaScriptPlugin::pluginStatic_ = 0L;

JavaScriptPlugin::JavaScriptPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
: KopetePlugin( JavaScriptPluginFactory::instance(), parent, name )
{
	if( !pluginStatic_ )
		pluginStatic_ = this;

	config = JavaScriptConfig::instance();

	//KopeteMessage events
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ),
		this, SLOT( slotDisplayMessage( KopeteMessage & ) ) );

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToReceive( KopeteMessage & ) ),
		this, SLOT( slotIncomingMessage( KopeteMessage & ) ) );

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToSend( KopeteMessage & ) ),
		this, SLOT( slotOutgoingMessage( KopeteMessage & ) ) );

	KJSEmbed::JSSecurityPolicy::setDefaultPolicy( KJSEmbed::JSSecurityPolicy::CapabilityAll );
	jsEngine = new KJSEmbed::KJSEmbedPart();

	//Register types
	jsEngine->factory()->addType( "Message" );
	jsEngine->factory()->addType( "KopeteAccount" );
	jsEngine->factory()->addType( "KopeteAccountManager" );
	jsEngine->factory()->addType( "KopeteContact" );
	jsEngine->factory()->addType( "KopeteMetaContact" );
	jsEngine->factory()->addType( "KopeteContactList" );
	jsEngine->factory()->addType( "KopeteMessageManager" );
	jsEngine->factory()->addType( "KopeteMessageManagerFactory" );

	//jsEngine->factory()->addType( "KopeteView" );

	//Export global objects
	jsEngine->addObject( this, "Kopete" );
	jsEngine->addObject( KopeteMessageManagerFactory::factory(), "MessageManagerFactory" );
	jsEngine->addObject( KopeteAccountManager::manager(), "AccountManager" );
	contactList = jsEngine->addObject( KopeteContactList::contactList(), "ContactList" );

	messageProxy = 0L;

	slotShowConsole();
}

JavaScriptPlugin::~JavaScriptPlugin()
{
	delete jsEngine;
	pluginStatic_ = 0L;
}

JavaScriptPlugin* JavaScriptPlugin::self()
{
	return pluginStatic_ ;
}

void JavaScriptPlugin::slotShowConsole()
{
	jsEngine->view()->show();
}

void JavaScriptPlugin::publishMessage( KopeteMessage &msg, ScriptType type )
{
	KJS::ExecState *exec = jsEngine->globalExec();

	Message msgWrapper( &msg, this );
	KJS::Object message = jsEngine->addObject( &msgWrapper, "Message" );

	Status myStatus( msg.manager()->account()->myself()->onlineStatus() );
	Status fromStatus( msg.from()->onlineStatus() );

	QPtrList<Status> statusContainer;
	statusContainer.setAutoDelete(true);

	//Bind some other objects for convience
	KJS::Object account = jsEngine->addObject( msg.manager()->account(), "Account" );
	account.put( exec, "contacts", contactDict( jsEngine, msg.manager()->account()->contacts(), statusContainer ) );

	KJS::Object myself = jsEngine->addObject( msg.manager()->account()->myself(), "Myself" );
	jsEngine->addObject( &myStatus, myself, "onlineStatus" );
	KJS::Object from = jsEngine->addObject( (QObject*)msg.from(), message, "from" );
	jsEngine->addObject( &fromStatus, from, "onlineStatus" );
	//(QObject*)msg.manager()->view(), "ActiveView" );

	message.put( exec, "to", contactPtrList( jsEngine, msg.to(), statusContainer ) );
	contactList.put( exec, "contacts",
		metaContactPtrList( jsEngine, KopeteContactList::contactList()->metaContacts(), statusContainer )
	);

	runScripts( msg.manager(), type );

	statusContainer.clear();
}

void JavaScriptPlugin::slotIncomingMessage( KopeteMessage &msg )
{
	publishMessage( msg, Incoming );
}

void JavaScriptPlugin::slotOutgoingMessage( KopeteMessage &msg )
{
	publishMessage( msg, Outgoing );
}

void JavaScriptPlugin::slotDisplayMessage( KopeteMessage &msg )
{
	publishMessage( msg, Display );
}

void JavaScriptPlugin::runScripts( KopeteMessageManager *manager, ScriptType type )
{
	jsEngine->execute( config->script( manager->account(), type ) );
	jsEngine->execute( config->script( 0L, type ) );
}

#include "javascriptplugin.moc"

