
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
		static QString id( Kopete::Contact *c )
		{
			return c->contactId();
		}

		static QString id( Kopete::MetaContact *c )
		{
			return c->displayName();
		}

		static QString id( Kopete::Group *c )
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

		void addChildObjects( KJS::Object &jsObject, Kopete::MetaContact *object )
		{
			ObjectList<Kopete::Contact> *contacts = new ObjectList<Kopete::Contact>(
				jsEngine, object->contacts()
			);

			wrapperList.insert( JavaScriptIdentifier::id( object ), contacts );
			jsObject.put( exec, "contacts", contacts->array() );

			/*ObjectList<Kopete::Group> *groups = new ObjectList<Kopete::Group>(
				jsEngine, object->groups()
			);

			wrapperList.insert( JavaScriptIdentifier::id( object ), groups );
			jsObject.put( exec, "groups", groups->array() );*/
		}

		void addChildObjects( KJS::Object &jsObject, Kopete::Contact *object )
		{
			JSStatus* status = new JSStatus( object->onlineStatus() );
			wrapperList.insert( JavaScriptIdentifier::id( object ), status );
			jsObject.put( exec, "onlineStatus", KJS::Object( status ) );
		}

		void addChildObjects( KJS::Object &, Kopete::Group * ){}

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
		ObjectList<Kopete::MetaContact> contactList;
		ObjectList<Kopete::Group> groupList;
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
: Kopete::Plugin( JavaScriptPluginFactory::instance(), parent, name )
{
	if( !pluginStatic_ )
		pluginStatic_ = this;

	//Kopete::Message events
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( Kopete::Message & ) ),
		this, SLOT( slotDisplayMessage( Kopete::Message & ) ) );

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToReceive( Kopete::Message & ) ),
		this, SLOT( slotIncomingMessage( Kopete::Message & ) ) );

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToSend( Kopete::Message & ) ),
		this, SLOT( slotOutgoingMessage( Kopete::Message & ) ) );

	connect( Kopete::AccountManager::manager(), SIGNAL( accountReady( Kopete::Account * ) ),
		this, SLOT( slotAccountCreated( Kopete::Account * ) ) );

	KJSEmbed::JSSecurityPolicy::setDefaultPolicy( KJSEmbed::JSSecurityPolicy::CapabilityAll );

	d = new JavaScriptPluginPrivate();
	connect( d->config, SIGNAL( changed() ), this, SLOT( slotReloadScripts() ) );
	for( QPtrListIterator<Kopete::Account> it( Kopete::AccountManager::manager()->accounts() ); it.current(); ++it )
	{
		slotAccountCreated( it.current() );
	}

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("jsconsole"),
		SLOT( slotShowConsole( const QString &, Kopete::MessageManager*) ),
		i18n("USAGE: /jsconsole - Shows the JavaScript console."), 0, 0 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("jsexec"),
		SLOT( slotJsExec( const QString &, Kopete::MessageManager*) ),
		i18n("USAGE: /jsexec [-o] <args> - Executes the JavaScript arguments in the current context and displays the results in the chat buffer. If -o is specified, the output is sent to all members of the chat."), 1 );
}

void JavaScriptPlugin::slotReloadScripts()
{
	for( QPtrListIterator<Kopete::Account> it( Kopete::AccountManager::manager()->accounts() ); it.current(); ++it )
	{
		execScripts( it.current() );
	}
}

void JavaScriptPlugin::execScripts( Kopete::Account *a )
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

void JavaScriptPlugin::slotAccountCreated( Kopete::Account *a )
{
	JavaScriptEnginePrivate *ep = d->engineMap[a];
	if( !ep )
	{
		ep = new JavaScriptEnginePrivate();
		d->engineMap.insert( a, ep );

		execScripts( a );

		//Register types
		ep->jsEngine->factory()->addType( "Message" );
		ep->jsEngine->factory()->addType( "Kopete::Account" );
		ep->jsEngine->factory()->addType( "Kopete::AccountManager" );
		ep->jsEngine->factory()->addType( "Kopete::Contact" );
		ep->jsEngine->factory()->addType( "Kopete::MetaContact" );
		ep->jsEngine->factory()->addType( "Kopete::ContactList" );
		ep->jsEngine->factory()->addType( "Kopete::MessageManager" );
		ep->jsEngine->factory()->addType( "KopeteMessageManagerFactory" );
		ep->jsEngine->factory()->addType( "KopeteView" );

		ep->jsEngine->addObject( KopeteMessageManagerFactory::factory(), "MessageManagerFactory" );
		ep->jsEngine->addObject( Kopete::AccountManager::manager(), "AccountManager" );

		KJS::Object account = ep->jsEngine->addObject( a, "Account" );
		ObjectList<Kopete::Contact> accountContacts( ep->jsEngine, a->contacts() );
		account.put( ep->jsEngine->globalExec(), "contacts", accountContacts.array() );

                JSContact *myself = new JSContact( msg->message()->manager()->account()->myself() );
                ep->jsEngine->interpreter()->globalObject().put( ep->jsEngine->globalExec(),
                    "Myself", KJS::Object( myself ) );

		KJS::Object contactList = ep->jsEngine->addObject( Kopete::ContactList::contactList(), "ContactList" );
		ep->contactList = ObjectList<Kopete::MetaContact>( ep->jsEngine, Kopete::ContactList::contactList()->metaContacts() );
		ep->groupList = ObjectList<Kopete::Group>( ep->jsEngine, Kopete::ContactList::contactList()->groups() );
		contactList.put( ep->jsEngine->globalExec(), "metaContacts", ep->contactList.array() );
		contactList.put( ep->jsEngine->globalExec(), "groups", ep->groupList.array() );

		//ep->jsEngine->view()->show();
	}
}

void JavaScriptPlugin::slotAccountDestroyed( Kopete::Account *a )
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

void JavaScriptPlugin::slotShowConsole( const QString &, Kopete::MessageManager *manager )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ manager->account() ];
	if( !ep )
	{
		//TODO: Init EP and continue
		Kopete::Message msg( manager->user(), manager->members(),
			i18n("There are no scripts currently active for this account."),
			Kopete::Message::Internal, Kopete::Message::PlainText, Kopete::Message::Chat );
		manager->appendMessage( msg );
	}
	else
	{
		ep->jsEngine->view()->show();
	}
}

void JavaScriptPlugin::slotJsExec( const QString &args, Kopete::MessageManager *manager )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ manager->account() ];
	if( !ep )
	{
		//TODO: Init EP and continue
		Kopete::Message msg( manager->user(), manager->members(),
			i18n("There are no scripts currently active for this account."),
			Kopete::Message::Internal, Kopete::Message::PlainText, Kopete::Message::Chat );
		manager->appendMessage( msg );
	}
	else
	{
		Kopete::Message::MessageDirection dir = Kopete::Message::Internal;
		QString cmd = args;
		QStringList argsList = Kopete::CommandHandler::parseArguments( args );

		if( argsList.front() == QString::fromLatin1("-o") )
		{
			dir = Kopete::Message::Outbound;
			cmd = args.section( QRegExp(QString::fromLatin1("\\s+") ), 1);
		}

		KJS::Value val = ep->jsEngine->evaluate( cmd );
		Kopete::Message msg( manager->user(), manager->members(),
			val.toString( ep->jsEngine->globalExec() ).qstring(), dir,
			Kopete::Message::PlainText, Kopete::Message::Chat );

		if( dir == Kopete::Message::Outbound )
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

	ObjectList<Kopete::Contact> toContacts( jsEngine, msg->message()->to() );
	message.put( exec, "to", toContacts.array() );
}

void JavaScriptPlugin::slotIncomingMessage( Kopete::Message &msg )
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

void JavaScriptPlugin::slotOutgoingMessage( Kopete::Message &msg )
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

void JavaScriptPlugin::slotDisplayMessage( Kopete::Message &msg )
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

void JavaScriptPlugin::slotAccountChangedStatus( Kopete::Account *a, const Kopete::OnlineStatus &,
	const Kopete::OnlineStatus & )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ a ];
	if( ep )
	{
		runScripts( a, "AccountStatusChange", ep->jsEngine );
	}
}

void JavaScriptPlugin::slotContactChangedStatus( Kopete::Contact *c, const Kopete::OnlineStatus &,
	const Kopete::OnlineStatus & )
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

void JavaScriptPlugin::slotContactAdded( Kopete::Contact *c )
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

void JavaScriptPlugin::slotContactRemoved( Kopete::Contact *c )
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

void JavaScriptPlugin::runScripts( Kopete::Account *a, const QString &scriptType, KJSEmbed::KJSEmbedPart *jsEngine )
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

