
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qptrlist.h>
#include <qregexp.h>
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
#include "kopetecommandhandler.h"
#include "kopetecontact.h"
#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopeteuiglobal.h"
#include "kopeteview.h"
#include "kopetecontactlist.h"
#include "wrappers/kopetemessage_imp.h"
#include "wrappers/kopetecontact_imp.h"
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
			//kdDebug() << k_funcinfo << "Adding object " << name << endl;
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
			JSStatus* status = new JSStatus( object->onlineStatus() );
			wrapperList.insert( JavaScriptIdentifier::id( object ), status );
			jsObject.put( exec, "onlineStatus", KJS::Object( status ) );
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

class JavaScriptEnginePrivate
{
	public:
		JavaScriptEnginePrivate() :
			jsEngine( new KJSEmbed::KJSEmbedPart ), contactList( jsEngine ), groupList( jsEngine )

		{}

		~JavaScriptEnginePrivate()
		{
			delete jsEngine;
		}

		KJSEmbed::KJSEmbedPart *jsEngine;
		ObjectList<KopeteMetaContact> contactList;
		ObjectList<KopeteGroup> groupList;
};

class JavaScriptPluginPrivate
{
	public:
		JavaScriptPluginPrivate() :
			config( JavaScriptConfig::instance() )
		{
			engineMap.setAutoDelete( true );
		}

		QPtrDict<JavaScriptEnginePrivate> engineMap;
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

	connect( KopeteAccountManager::manager(), SIGNAL( accountReady( KopeteAccount * ) ),
		this, SLOT( slotAccountCreated( KopeteAccount * ) ) );

	KJSEmbed::JSSecurityPolicy::setDefaultPolicy( KJSEmbed::JSSecurityPolicy::CapabilityAll );

	d = new JavaScriptPluginPrivate();
	connect( d->config, SIGNAL( changed() ), this, SLOT( slotReloadScripts() ) );
	for( QPtrListIterator<KopeteAccount> it( KopeteAccountManager::manager()->accounts() ); it.current(); ++it )
	{
		slotAccountCreated( it.current() );
	}

	KopeteCommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("jsconsole"),
		SLOT( slotShowConsole( const QString &, KopeteMessageManager*) ),
		i18n("USAGE: /jsconsole - Shows the JavaScript console."), 0, 0 );

	KopeteCommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("jsexec"),
		SLOT( slotJsExec( const QString &, KopeteMessageManager*) ),
		i18n("USAGE: /jsexec [-o] <args> - Executes the JavaScript arguments in the current context and displays the results in the chat buffer. If -o is specified, the output is sent to all members of the chat."), 1 );
}

void JavaScriptPlugin::slotReloadScripts()
{
	for( QPtrListIterator<KopeteAccount> it( KopeteAccountManager::manager()->accounts() ); it.current(); ++it )
	{
		execScripts( it.current() );
	}
}

void JavaScriptPlugin::execScripts( KopeteAccount *a )
{
	QValueList<Script*> scripts = d->config->scriptsFor( a );
	if( !scripts.isEmpty() )
	{
		JavaScriptEnginePrivate *ep = d->engineMap[a];
		for( QValueList<Script*>::iterator s = scripts.begin(); s != scripts.end(); ++s )
		{
			kdDebug() << (*s)->script() << endl;
			ep->jsEngine->execute( (*s)->script() );
			QMap<QString,QString>::iterator it = (*s)->functions.find( QString::fromLatin1("Init") );
			if( it != (*s)->functions.end() )
			{
				ep->jsEngine->evaluate( it.data() + QString::fromLatin1("()") );
			}
		}
	}
}

void JavaScriptPlugin::slotAccountCreated( KopeteAccount *a )
{
	JavaScriptEnginePrivate *ep = d->engineMap[a];
	if( !ep )
	{
		ep = new JavaScriptEnginePrivate();
		d->engineMap.insert( a, ep );

		execScripts( a );

		//Register types
		ep->jsEngine->factory()->addType( "Message" );
		ep->jsEngine->factory()->addType( "KopeteAccount" );
		ep->jsEngine->factory()->addType( "KopeteAccountManager" );
		ep->jsEngine->factory()->addType( "KopeteContact" );
		ep->jsEngine->factory()->addType( "KopeteMetaContact" );
		ep->jsEngine->factory()->addType( "KopeteContactList" );
		ep->jsEngine->factory()->addType( "KopeteMessageManager" );
		ep->jsEngine->factory()->addType( "KopeteMessageManagerFactory" );
		ep->jsEngine->factory()->addType( "KopeteView" );

		ep->jsEngine->addObject( KopeteMessageManagerFactory::factory(), "MessageManagerFactory" );
		ep->jsEngine->addObject( KopeteAccountManager::manager(), "AccountManager" );

		KJS::Object account = ep->jsEngine->addObject( a, "Account" );
		ObjectList<KopeteContact> accountContacts( ep->jsEngine, a->contacts() );
		account.put( ep->jsEngine->globalExec(), "contacts", accountContacts.array() );

                JSContact *myself = new JSContact( msg->message()->manager()->account()->myself() );
                ep->jsEngine->interpreter()->globalObject().put( ep->jsEngine->globalExec(),
                    "Myself", KJS::Object( myself ) );

		KJS::Object contactList = ep->jsEngine->addObject( KopeteContactList::contactList(), "ContactList" );
		ep->contactList = ObjectList<KopeteMetaContact>( ep->jsEngine, KopeteContactList::contactList()->metaContacts() );
		ep->groupList = ObjectList<KopeteGroup>( ep->jsEngine, KopeteContactList::contactList()->groups() );
		contactList.put( ep->jsEngine->globalExec(), "metaContacts", ep->contactList.array() );
		contactList.put( ep->jsEngine->globalExec(), "groups", ep->groupList.array() );

		//ep->jsEngine->view()->show();
	}
}

void JavaScriptPlugin::slotAccountDestroyed( KopeteAccount *a )
{
	JavaScriptEnginePrivate *ep = d->engineMap[a];
	if( ep )
	{
		delete ep;
		d->engineMap.remove( a );
	}
}

JavaScriptPlugin::~JavaScriptPlugin()
{
	delete d;
	pluginStatic_ = 0L;
}

JavaScriptPlugin* JavaScriptPlugin::self()
{
	return pluginStatic_ ;
}

void JavaScriptPlugin::slotShowConsole( const QString &, KopeteMessageManager *manager )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ manager->account() ];
	if( !ep )
	{
		//TODO: Init EP and continue
		KopeteMessage msg( manager->user(), manager->members(),
			i18n("There are no scripts currently active for this account."),
			KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat );
		manager->appendMessage( msg );
	}
	else
	{
		ep->jsEngine->view()->show();
	}
}

void JavaScriptPlugin::slotJsExec( const QString &args, KopeteMessageManager *manager )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ manager->account() ];
	if( !ep )
	{
		//TODO: Init EP and continue
		KopeteMessage msg( manager->user(), manager->members(),
			i18n("There are no scripts currently active for this account."),
			KopeteMessage::Internal, KopeteMessage::PlainText, KopeteMessage::Chat );
		manager->appendMessage( msg );
	}
	else
	{
		KopeteMessage::MessageDirection dir = KopeteMessage::Internal;
		QString cmd = args;
		QStringList argsList = KopeteCommandHandler::parseArguments( args );

		if( argsList.front() == QString::fromLatin1("-o") )
		{
			dir = KopeteMessage::Outbound;
			cmd = args.section( QRegExp(QString::fromLatin1("\\s+") ), 1);
		}

		KJS::Value val = ep->jsEngine->evaluate( cmd );
		KopeteMessage msg( manager->user(), manager->members(),
			val.toString( ep->jsEngine->globalExec() ).qstring(), dir,
			KopeteMessage::PlainText, KopeteMessage::Chat );

		if( dir == KopeteMessage::Outbound )
			manager->sendMessage( msg );
		else
			manager->appendMessage( msg );
	}
}

void JavaScriptPlugin::publishMessage( JSMessage *msg, KJSEmbed::KJSEmbedPart *jsEngine )
{
	KJS::ExecState *exec = jsEngine->globalExec();
        KJS::Object message( msg );
        jsEngine->interpreter()->globalObject().put( exec, "Message", message );

	ObjectList<KopeteContact> toContacts( jsEngine, msg->message()->to() );
	message.put( exec, "to", toContacts.array() );
}

void JavaScriptPlugin::slotIncomingMessage( KopeteMessage &msg )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ msg.manager()->account() ];
	if( ep )
	{
		JSMessage msgWrapper( &msg );
		publishMessage( &msgWrapper, ep->jsEngine );
		runScripts( msg.manager()->account(), "Incoming", ep->jsEngine );
		ep->jsEngine->interpreter()->globalObject().deleteProperty(
			ep->jsEngine->globalExec(), "Message"
		);
	}
}

void JavaScriptPlugin::slotOutgoingMessage( KopeteMessage &msg )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ msg.manager()->account() ];
	if( ep )
	{
		JSMessage msgWrapper( &msg );
		publishMessage( &msgWrapper, ep->jsEngine );
		runScripts( msg.manager()->account(), "Outgoing", ep->jsEngine );
		ep->jsEngine->interpreter()->globalObject().deleteProperty(
			ep->jsEngine->globalExec(), "Message"
		);
	}
}

void JavaScriptPlugin::slotDisplayMessage( KopeteMessage &msg )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ msg.manager()->account() ];
	if( ep )
	{
		JSMessage msgWrapper( &msg );
		publishMessage( &msgWrapper, ep->jsEngine );
		runScripts( msg.manager()->account(), "Display", ep->jsEngine );
		ep->jsEngine->interpreter()->globalObject().deleteProperty(
			ep->jsEngine->globalExec(), "Message"
		);
	}
}

void JavaScriptPlugin::slotAccountChangedStatus( KopeteAccount *a, const KopeteOnlineStatus &,
	const KopeteOnlineStatus & )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ a ];
	if( ep )
	{
		runScripts( a, "AccountStatusChange", ep->jsEngine );
	}
}

void JavaScriptPlugin::slotContactChangedStatus( KopeteContact *c, const KopeteOnlineStatus &,
	const KopeteOnlineStatus & )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ c->account() ];
	if( ep )
	{
		ep->jsEngine->addObject( c, "Contact" );
		runScripts( c->account(), "ContactStatusChange", ep->jsEngine );
		ep->jsEngine->interpreter()->globalObject().deleteProperty(
			ep->jsEngine->globalExec(), "Contact"
		);
	}
}

void JavaScriptPlugin::slotContactAdded( KopeteContact *c )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ c->account() ];
	if( ep )
	{
		ep->jsEngine->addObject( c, "Contact" );
		runScripts( c->account(), "ContactAdded", ep->jsEngine );
		ep->jsEngine->interpreter()->globalObject().deleteProperty(
			ep->jsEngine->globalExec(), "Contact"
		);
	}
}

void JavaScriptPlugin::slotContactRemoved( KopeteContact *c )
{
 	JavaScriptEnginePrivate *ep = d->engineMap[ c->account() ];
	if( ep )
	{
		ep->jsEngine->addObject( c, "Contact" );
		runScripts( c->account(), "ContactRemoved", ep->jsEngine );
		ep->jsEngine->interpreter()->globalObject().deleteProperty(
			ep->jsEngine->globalExec(), "Contact"
		);
	}
}

void JavaScriptPlugin::runScripts( KopeteAccount *a, const QString &scriptType, KJSEmbed::KJSEmbedPart *jsEngine )
{
	QValueList<Script*> scripts = d->config->scriptsFor( a );
	kdDebug() << k_funcinfo << "Scripts for " << a->accountId() << ", type " << scriptType << " = " << scripts.count() << endl;
	for( QValueList<Script*>::iterator s = scripts.begin(); s != scripts.end(); ++s )
	{
		QMap<QString,QString>::iterator it = (*s)->functions.find( scriptType );
		if( it != (*s)->functions.end() )
		{
			kdDebug() << k_funcinfo << "Executing " << it.data() << QString::fromLatin1("()") << endl;
			jsEngine->evaluate( it.data() + QString::fromLatin1("()") );
		}
	}
}

#include "javascriptplugin.moc"

