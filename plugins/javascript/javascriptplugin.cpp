
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "javascriptplugin.h"

#include "javascriptconfig.h"
#include "javascriptfile.h"

/*
#include "wrappers/kopetemessageimp.h"
#include "wrappers/kopetecontactimp.h"
*/

#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopetecommandhandler.h"
#include "kopetechatsessionmanager.h"
#include "kopetecontact.h"
#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopeteuiglobal.h"
#include "kopeteview.h"
#include "kopetecontactlist.h"

#include <kjsembed/kjsembed.h>
/*
#include <kjsembed/kjsembedpart.h>
#include <kjsembed/jsconsolewidget.h>
#include <kjsembed/jsproxy.h>
#include <kjsembed/jsfactory.h>
#include <kjsembed/jssecuritypolicy.h>
#include <kjsembed/jsopaqueproxy.h>
*/
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kapplication.h>

#include <QtCore/QRegExp>

typedef KGenericFactory<JavaScriptPlugin> JavaScriptPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_javascript, JavaScriptPluginFactory( "kopete_javascript" )  )

JavaScriptPlugin* JavaScriptPlugin::pluginStatic_ = 0L;

class JavaScriptEnginePrivate
{
public:
	JavaScriptEnginePrivate()
		: jsEngine( new KJSEmbed::Engine )
//		, contactList( jsEngine )
//		, groupList( jsEngine )
	{}

	~JavaScriptEnginePrivate()
	{
		delete jsEngine;
	}

	KJSEmbed::Engine *jsEngine;
//	ObjectList<Kopete::MetaContact> contactList;
//	ObjectList<Kopete::Group> groupList;
};

class JavaScriptPluginPrivate
{
public:
	JavaScriptPluginPrivate()
		: config( JavaScriptConfig::instance() )
	{
	}

	~JavaScriptPluginPrivate()
	{
		qDeleteAll(engineMap.values());
	}

	QHash<Kopete::Account *, JavaScriptEnginePrivate *> engineMap;
	JavaScriptConfig *config;
};

JavaScriptPlugin::JavaScriptPlugin( QObject *parent, const QStringList &/*args*/ )
: Kopete::Plugin( JavaScriptPluginFactory::componentData(), parent )
{
	if( !pluginStatic_ )
		pluginStatic_ = this;

	//Kopete::Message events
	connect( Kopete::ChatSessionManager::self(), SIGNAL( aboutToDisplay( Kopete::Message & ) ),
		this, SLOT( slotDisplayMessage( Kopete::Message & ) ) );

	connect( Kopete::ChatSessionManager::self(), SIGNAL( aboutToReceive( Kopete::Message & ) ),
		this, SLOT( slotIncomingMessage( Kopete::Message & ) ) );

	connect( Kopete::ChatSessionManager::self(), SIGNAL( aboutToSend( Kopete::Message & ) ),
		this, SLOT( slotOutgoingMessage( Kopete::Message & ) ) );

	connect( Kopete::AccountManager::self(), SIGNAL( accountRegistered( Kopete::Account * ) ),
		this, SLOT( slotAccountCreated( Kopete::Account * ) ) );

#ifdef __GNUC__
#warning REANABLE POLICIES ?
#endif
//	KJSEmbed::JSSecurityPolicy::setDefaultPolicy( KJSEmbed::JSSecurityPolicy::CapabilityAll );

	d = new JavaScriptPluginPrivate();
	connect( d->config, SIGNAL( changed() ), this, SLOT( slotReloadScripts() ) );
	foreach( Kopete::Account *account, Kopete::AccountManager::self()->accounts() )
	{
//		slotAccountCreated( account );
	}

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("jsconsole"),
		SLOT( slotShowConsole( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /jsconsole - Shows the JavaScript console."), 0, 0 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("jsexec"),
		SLOT( slotJsExec( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /jsexec [-o] <args> - Executes the JavaScript arguments in the current context and displays the results in the chat buffer. If -o is specified, the output is sent to all members of the chat."), 1 );
}

JavaScriptPlugin::~JavaScriptPlugin()
{
	delete d;
	pluginStatic_ = 0;
}

JavaScriptPlugin* JavaScriptPlugin::self()
{
	return pluginStatic_ ;
}

void JavaScriptPlugin::slotReloadScripts()
{
	foreach( Kopete::Account *account, Kopete::AccountManager::self()->accounts() )
	{
		execScripts( account );
	}
}

void JavaScriptPlugin::execScripts( Kopete::Account *account )
{
        JavaScriptEnginePrivate *ep = d->engineMap[account];
	foreach( JavaScriptFile *scriptfile, d->config->scriptsFor( account ) )
	{
		kDebug() << scriptfile->script() << endl;
		ep->jsEngine->execute( scriptfile->script() );
		QMap<QString,QString>::iterator it = scriptfile->functions.find( QLatin1String("Init") );
		if( it != scriptfile->functions.end() )
		{
			ep->jsEngine->execute( it.data() + QLatin1String("()") );
		}
	}
}

#ifdef __GNUC__
#warning Temporary disable account creation code
#endif
/*
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
		ep->jsEngine->factory()->addType( "Kopete::ChatSession" );
		ep->jsEngine->factory()->addType( "Kopete::ChatSessionManager" );
		ep->jsEngine->factory()->addType( "KopeteView" );

		ep->jsEngine->addObject( Kopete::ChatSessionManager::self(), "ChatSessionManager" );
		ep->jsEngine->addObject( Kopete::AccountManager::self(), "AccountManager" );

		KJS::Object account = ep->jsEngine->addObject( a, "Account" );
		ObjectList<Kopete::Contact> accountContacts( ep->jsEngine, a->contacts() );
		account.put( ep->jsEngine->globalExec(), "contacts", accountContacts.array() );

                JSContact *myself = new JSContact( msg->message()->manager()->account()->myself() );
                ep->jsEngine->interpreter()->globalObject().put( ep->jsEngine->globalExec(),
                    "Myself", KJS::Object( myself ) );

		KJS::Object contactList = ep->jsEngine->addObject( Kopete::ContactList::self(), "ContactList" );
		ep->contactList = ObjectList<Kopete::MetaContact>( ep->jsEngine, Kopete::ContactList::self()->metaContacts() );
		ep->groupList = ObjectList<Kopete::Group>( ep->jsEngine, Kopete::ContactList::self()->groups() );
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
*/

void JavaScriptPlugin::slotShowConsole( const QString &, Kopete::ChatSession *manager )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ manager->account() ];
	if( !ep )
	{
		//TODO: Init EP and continue
		Kopete::Message msg( manager->myself(), manager->members(),
			i18n("There are no scripts currently active for this account."),
			Kopete::Message::Internal, Kopete::Message::PlainText );
		manager->appendMessage( msg );
	}
	else
	{
		kDebug() << "No console widget for now" << endl;
//		ep->jsEngine->view()->show();
	}
}

void JavaScriptPlugin::slotJsExec( const QString &args, Kopete::ChatSession *manager )
{
	JavaScriptEnginePrivate *ep = d->engineMap[ manager->account() ];
	if( !ep )
	{
		//TODO: Init EP and continue
		Kopete::Message msg( manager->myself(), manager->members(),
			i18n("There are no scripts currently active for this account."),
			Kopete::Message::Internal, Kopete::Message::PlainText );
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

#ifdef __GNUC__
#warning disable some execution code for now
#endif
/*
		KJS::Value val = ep->jsEngine->evaluate( cmd );
		Kopete::Message msg( manager->myself(), manager->members(),
			val.toString( ep->jsEngine->globalExec() ).qstring(), dir,
			Kopete::Message::PlainText, Kopete::Message::Chat );

		if( dir == Kopete::Message::Outbound )
			manager->sendMessage( msg );
		else
			manager->appendMessage( msg );
*/
	}
}

#ifdef __GNUC__
#warning Disable message management
#endif
/*
void JavaScriptPlugin::publishMessage( JSMessage *msg, KJSEmbed::Engine *jsEngine )
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

void JavaScriptPlugin::slotAccountChangedStatus( Kopete::Account *a,
	const Kopete::OnlineStatus &,
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
*/
void JavaScriptPlugin::runScripts( Kopete::Account *a, const QString &scriptType, KJSEmbed::Engine *jsEngine )
{
//	kDebug() << k_funcinfo << "Scripts for " << a->accountId() << ", type " << scriptType << " = " << scripts.count() << endl;
	foreach( JavaScriptFile *scriptfile, d->config->scriptsFor( a ) )
	{
		QMap<QString,QString>::iterator it = scriptfile->functions.find( scriptType );
		if( it != scriptfile->functions.end() )
		{
			QString functionCall = it.data() + QLatin1String("()");
			kDebug() << k_funcinfo << "Executing " << functionCall << endl;
			jsEngine->execute( functionCall );
		}
	}
}

#include "javascriptplugin.moc"

