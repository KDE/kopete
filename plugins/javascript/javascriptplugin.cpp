
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qtimer.h>
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
#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopeteview.h"
#include "kopetecontactlist.h"
#include "wrappers/kopetemessage_imp.h"
#include "javascriptplugin.h"
#include "javascriptconfig.h"

class JavaScriptIdentifier
{
	public:
		static QString id( KopeteContact *c )
		{
			return c->contactId();
		}

		static QString id( KopeteMetaContact *c )
		{
			return c->displayName();
		}

		static QString id( KopeteGroup *c )
		{
			return c->displayName();
		}
};

template< class ObjectT >
class ObjectList
{
	public:
        	ObjectList( KJSEmbed::KJSEmbedPart *engine ) : exec( engine->globalExec() ), jsEngine( engine )
		{
			init();
		}

		ObjectList( KJSEmbed::KJSEmbedPart *engine, const QPtrList<ObjectT> &list )
			: exec( engine->globalExec() ), jsEngine( engine )
		{
			init();

			for( QPtrListIterator<ObjectT> it( list ); it.current(); ++it )
			{
				addObject( JavaScriptIdentifier::id( it.current() ), it.current() );
			}
		}

		ObjectList( KJSEmbed::KJSEmbedPart *engine, const QDict<ObjectT> &list )
			: exec( engine->globalExec() ), jsEngine( engine )
		{
			init();

			for( QDictIterator<ObjectT> it( list ); it.current(); ++it )
			{
				addObject( JavaScriptIdentifier::id( it.current() ), it.current() );
			}
		}

		void addObject( const QString &name, ObjectT *object )
		{
			kdDebug() << k_funcinfo << "Adding object " << name << endl;
			if( !objectList[name] )
			{
				KJS::List arg;

				KJS::Object jsObject = jsEngine->factory()->createProxy( exec, object );
				addChildObjects( jsObject, object );
				arg.append( jsObject );
				pushMethod.call( exec, jsArray, arg );
				objectList.insert( name, object );
			}
		}

		void addChildObjects( KJS::Object &jsObject, KopeteMetaContact *object )
		{
			ObjectList<KopeteContact> *contacts = new ObjectList<KopeteContact>(
				jsEngine, object->contacts()
			);

			wrapperList.insert( JavaScriptIdentifier::id( object ), contacts );
			jsObject.put( exec, "contacts", contacts->array() );

			/*ObjectList<KopeteGroup> *groups = new ObjectList<KopeteGroup>(
				jsEngine, object->groups()
			);

			wrapperList.insert( JavaScriptIdentifier::id( object ), groups );
			jsObject.put( exec, "groups", groups->array() );*/
		}

		void addChildObjects( KJS::Object &jsObject, KopeteContact *object )
		{
			Status* status = new Status( object->onlineStatus() );
			wrapperList.insert( JavaScriptIdentifier::id( object ), status );
			KJS::Object statusObject = jsEngine->factory()->createProxy( exec, status );
			jsObject.put( exec, "onlineStatus", statusObject );
		}

		void addChildObjects( KJS::Object &, KopeteGroup * ){}

		void removeObject( const QString &name )
		{
			KJS::Object jsObject = object( name );
			byNameObject.deleteProperty( exec, name );
			objectList.remove( name );
			wrapperList.remove( name );
		}

		KJS::Object &array()
		{
			return jsArray;
		}

		ObjectT *objectPointer( const QString &name ) const
		{
			return objectList[ name ];
		}

	private:
		void init()
		{
			KJS::List arg;
			jsArray = jsEngine->interpreter()->builtinArray().construct( exec, arg );
			pushMethod = jsArray.get( exec, "push" ).toObject( exec );

			wrapperList.setAutoDelete(true);
		}

		KJS::Object jsArray;
		KJS::Object pushMethod;
		KJS::ExecState *exec;
		KJSEmbed::KJSEmbedPart *jsEngine;
		QDict<ObjectT>	objectList;
		QDict<void>	wrapperList;
};

typedef KGenericFactory<JavaScriptPlugin> JavaScriptPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_javascript, JavaScriptPluginFactory( "kopete_javascript" )  )

JavaScriptPlugin* JavaScriptPlugin::pluginStatic_ = 0L;

class JavaScriptPluginPrivate
{
	public:
		JavaScriptPluginPrivate( KJSEmbed::KJSEmbedPart *engine ) :
			jsEngine( engine ), contactList( engine ), groupList( engine ),
			config( JavaScriptConfig::instance() )
		{}

		KJSEmbed::KJSEmbedPart *jsEngine;
		ObjectList<KopeteMetaContact> contactList;
		ObjectList<KopeteGroup> groupList;
		JavaScriptConfig *config;
};

JavaScriptPlugin::JavaScriptPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
: KopetePlugin( JavaScriptPluginFactory::instance(), parent, name )
{
	if( !pluginStatic_ )
		pluginStatic_ = this;

	//KopeteMessage events
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ),
		this, SLOT( slotDisplayMessage( KopeteMessage & ) ) );

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToReceive( KopeteMessage & ) ),
		this, SLOT( slotIncomingMessage( KopeteMessage & ) ) );

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToSend( KopeteMessage & ) ),
		this, SLOT( slotOutgoingMessage( KopeteMessage & ) ) );

	KJSEmbed::JSSecurityPolicy::setDefaultPolicy( KJSEmbed::JSSecurityPolicy::CapabilityAll );

	d = new JavaScriptPluginPrivate(new KJSEmbed::KJSEmbedPart());

	//Register types
	d->jsEngine->factory()->addType( "Message" );
	d->jsEngine->factory()->addType( "KopeteAccount" );
	d->jsEngine->factory()->addType( "KopeteAccountManager" );
	d->jsEngine->factory()->addType( "KopeteContact" );
	d->jsEngine->factory()->addType( "KopeteMetaContact" );
	d->jsEngine->factory()->addType( "KopeteContactList" );
	d->jsEngine->factory()->addType( "KopeteMessageManager" );
	d->jsEngine->factory()->addType( "KopeteMessageManagerFactory" );

	//d->jsEngine->factory()->addType( "KopeteView" );

	//Export global objects
	d->jsEngine->addObject( this, "Kopete" );
	d->jsEngine->addObject( KopeteMessageManagerFactory::factory(), "MessageManagerFactory" );
	d->jsEngine->addObject( KopeteAccountManager::manager(), "AccountManager" );

	slotShowConsole();

	QTimer::singleShot( 1000, this, SLOT( slotInitContacts() ) );
}

void JavaScriptPlugin::slotInitContacts()
{
	KJS::Object contactList = d->jsEngine->addObject( KopeteContactList::contactList(), "ContactList" );
	d->contactList = ObjectList<KopeteMetaContact>( d->jsEngine, KopeteContactList::contactList()->metaContacts() );
	d->groupList = ObjectList<KopeteGroup>( d->jsEngine, KopeteContactList::contactList()->groups() );
	contactList.put( d->jsEngine->globalExec(), "metaContacts", d->contactList.array() );
	contactList.put( d->jsEngine->globalExec(), "groups", d->groupList.array() );
}

JavaScriptPlugin::~JavaScriptPlugin()
{
	delete d->jsEngine;
	delete d;
	pluginStatic_ = 0L;
}

JavaScriptPlugin* JavaScriptPlugin::self()
{
	return pluginStatic_ ;
}

void JavaScriptPlugin::slotShowConsole()
{
	d->jsEngine->view()->show();
}

void JavaScriptPlugin::publishMessage( KopeteMessage &msg, ScriptType type )
{
	KJS::ExecState *exec = d->jsEngine->globalExec();

	Message msgWrapper( &msg, this );
	KJS::Object message = d->jsEngine->addObject( &msgWrapper, "Message" );

	Status myStatus( msg.manager()->account()->myself()->onlineStatus() );
	Status fromStatus( msg.from()->onlineStatus() );

	KJS::Object account = d->jsEngine->addObject( msg.manager()->account(), "Account" );
	ObjectList<KopeteContact> accountContacts( d->jsEngine, msg.manager()->account()->contacts() );
	account.put( exec, "contacts", accountContacts.array() );

	KJS::Object myself = d->jsEngine->addObject( msg.manager()->account()->myself(), "Myself" );
	d->jsEngine->addObject( &myStatus, myself, "onlineStatus" );

	KJS::Object from = d->jsEngine->addObject( (QObject*)msg.from(), message, "from" );
	d->jsEngine->addObject( &fromStatus, from, "onlineStatus" );
	//(QObject*)msg.manager()->view(), "ActiveView" );

	ObjectList<KopeteContact> toContacts( d->jsEngine, msg.to() );
	message.put( exec, "to", toContacts.array() );

	runScripts( msg.manager(), type );
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
	d->jsEngine->execute( d->config->script( manager->account(), type ) );
	d->jsEngine->execute( d->config->script( 0L, type ) );
}

#include "javascriptplugin.moc"

