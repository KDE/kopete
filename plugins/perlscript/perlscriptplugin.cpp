/*
    perlplugin.cpp

    Kopete Perl Scriping plugin

    Copyright (c) 2003 by Jason Keirstead   <jason@keirstead.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include <qstringlist.h>
#include <qptrlist.h>
#include <qfile.h>
#include <qsignalmapper.h>

#include <kdebug.h>
#include <knotifyclient.h>
#include <ktempfile.h>
#include <kgenericfactory.h>
#include <kio/netaccess.h>
#include <kaction.h>

#include "kopetemetacontact.h"
#include "kopetemessagemanagerfactory.h"
#include "perlscriptplugin.h"
#include "perlscriptprefs.h"

K_EXPORT_COMPONENT_FACTORY( kopete_perlscript, KGenericFactory<PerlPlugin> );

//This stuff lets the perl engine load external modules if they want to in their scripts
EXTERN_C void xs_init (pTHX);
EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);
EXTERN_C void
xs_init(pTHX)
{
        //This qstrdup stuff is a hack to surpress a few ISO C++ warnings about converting const char* to char*
	char *file = qstrdup(__FILE__);
        dXSUB_SYS;

        /* DynaLoader is a special case */
        newXS( qstrdup("DynaLoader::boot_DynaLoader"), boot_DynaLoader, file);
}

PerlPlugin::PerlPlugin( QObject *parent, const char *name, const QStringList &) : KopetePlugin( parent, name )
{
	kdDebug(14010) << k_funcinfo << "Loaded PerlScript" << endl;
	if ( !pluginStatic_ )
		pluginStatic_ = this;
		
	//Initialize the perl engine	
	my_perl = perl_alloc();
	perl_construct( my_perl );
	
	m_prefs = new PerlScriptPreferences( QString::null );
	
	connect( m_prefs, SIGNAL(scriptAdded( const QString &, const QString &, const QString & )), this, SLOT( slotAddScript( const QString &, const QString &, const QString & ) ));
	connect( m_prefs, SIGNAL(scriptRemoved( const QString &)), this, SLOT(slotRemoveScript( const QString & )) );
	connect( m_prefs, SIGNAL(loaded()), this, SLOT(slotClearScripts()) );
	
	m_prefs->reopen();
	
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ), SLOT( slotIncomingMessage( KopeteMessage & ) ) );
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToSend( KopeteMessage & ) ), SLOT( slotOutgoingMessage( KopeteMessage & ) ) );
}

PerlPlugin::~PerlPlugin()
{
	//Destroy the perl engine
	perl_destruct(my_perl);
	perl_free(my_perl);

	pluginStatic_ = 0L;
}

PerlPlugin* PerlPlugin::plugin()
{
	return pluginStatic_ ;
}

PerlPlugin* PerlPlugin::pluginStatic_ = 0L;

void PerlPlugin::slotAddScript( const QString &scriptPath, const QString &scriptName, const QString &desc )
{
	kdDebug(14010) << k_funcinfo << "Loaded script " << scriptName << endl;
	PerlScript *script = new PerlScript( scriptPath, scriptName, desc );
	m_allScripts.insert( scriptPath, script);
	
	if( script->scriptText.contains( QString::fromLatin1("IncomingMessage") ) )
	{
		kdDebug(14010) << k_funcinfo << "Script has an incoming handler" << endl;
		m_incomingScripts.append( script );
	}
	
	if( script->scriptText.contains( QString::fromLatin1("OutgoingMessage") ) )
	{
		kdDebug(14010) << k_funcinfo << "Script has an outgoing handler" << endl;
		m_outgoingScripts.append( script );
	}
	
	if( script->scriptText.contains( QString::fromLatin1("ContactContextMenu") ) )
	{
		kdDebug(14010) << k_funcinfo << "Script has a contact context menu handler" << endl;
		m_actionScripts.append( script );
	}
}

void PerlPlugin::slotRemoveScript( const QString &scriptPath )
{
	PerlScript *script = m_allScripts[ scriptPath ];
	
	m_allScripts.remove( scriptPath );
	m_incomingScripts.remove( script );
	m_outgoingScripts.remove( script );
	m_actionScripts.remove( script );
	
	delete script;
}

void PerlPlugin::slotClearScripts()
{
	m_allScripts.clear();
	m_incomingScripts.clear();
	m_outgoingScripts.clear();
	m_actionScripts.clear();
}

void PerlPlugin::slotScriptModified( const QString &scriptPath )
{
	PerlScript *script = m_allScripts[ scriptPath ];
	QString scriptName = script->name;
	QString scriptDesc = script->description;
	
	slotRemoveScript( scriptPath );
	slotAddScript( scriptPath, scriptName, scriptDesc );
}

PerlScript *PerlPlugin::script(  const QString &scriptPath )
{
	return m_allScripts[ scriptPath ];
}

KActionCollection *PerlPlugin::customContextMenuActions(KopeteMetaContact *m)
{
	KActionCollection *m_actionCollection = 0L;
	
	if( !m_actionScripts.isEmpty() )
	{
		m_actionCollection = new KActionCollection(this);
		KActionMenu *actionMenu = new KActionMenu( i18n("Perl Scripts") );
		m_actionCollection->insert( actionMenu );
		
		QSignalMapper *actionMapper = new QSignalMapper( this );
 		connect( actionMapper, SIGNAL( mapped( const QString & ) ),this, SLOT( slotContextScript( const QString & ) ) );
		
		for ( PerlScript *script = m_actionScripts.first(); script; script = m_actionScripts.next() )
		{
			script->tempArgs = new QStringList();
			script->tempArgs->append( m->displayName() );
						
			KAction *scriptAction = new KAction( script->name );
			connect( scriptAction, SIGNAL( activated() ), actionMapper, SLOT( map() ) );
			actionMapper->setMapping( scriptAction, script->path );
			actionMenu->insert( scriptAction );
		}
	}
	
	return m_actionCollection;
}

void PerlPlugin::slotContextScript( const QString &scriptPath )
{
	PerlScript *script = m_allScripts[ scriptPath ];
	executeScript( script->scriptText, "ContactContextMenu", *(script->tempArgs) );
}

void PerlPlugin::slotIncomingMessage( KopeteMessage& msg )
{
	if( !m_incomingScripts.isEmpty() )
	{
		kdDebug(14010) << k_funcinfo << "Executing incoming scripts" << endl;
		
		//Get our arguments fo rthe perl script
		QStringList args = getArguments( msg );
		
		//Execute all the incoming scripts			
		for ( PerlScript *script = m_incomingScripts.first(); script; script = m_incomingScripts.next() )
		{
			msg.setBody( executeScript( script->scriptText, QString::fromLatin1("IncomingMessage"), args ) );
			emit( scriptExecuted( script->path, script->name ) );
		}
	}
}

void PerlPlugin::slotOutgoingMessage( KopeteMessage& msg )
{
	if( !m_outgoingScripts.isEmpty() )
	{
		kdDebug(14010) << k_funcinfo << "Executing outgoing scripts" << endl;
		
		//Get our arguments fo rthe perl script
		QStringList args = getArguments( msg );

		//Execute all the outgoing scripts
		for ( PerlScript *script = m_outgoingScripts.first(); script; script = m_outgoingScripts.next() )
		{
			msg.setBody( executeScript( script->scriptText, QString::fromLatin1("OutgoingMessage"), args ) );
			emit( scriptExecuted( script->path, script->name ) );
		}
	}
}

QStringList PerlPlugin::getArguments( KopeteMessage &msg )
{
	QStringList args;
	
	//First argument is the first word in the message
	args.append( msg.plainBody().section( QString::fromLatin1(" "), 0, 0) );
	
	//Second argument is the remainder of the message
	args.append( msg.plainBody().section( QString::fromLatin1(" "), 1) );

	//Third argument is the sender's displayName
	args.append( msg.from()->displayName() );
	
	//All remaining arguments are the message recipients' displayNames
	for( KopeteContact *c = msg.to().first(); c; c = msg.to().next() )
		args.append( c->displayName() );
		
	return args;
}

QString PerlPlugin::executeScript( const QString &scriptText, const QString &subName, QStringList &args )
{
	//This qstrdup stuff is a hack to surpress a few ISO C++ warnings about converting const char* to char*
	char *embedding[] = { qstrdup(""), qstrdup("-e"), qstrdup("0") };

	perl_parse(my_perl, xs_init, 3, embedding, NULL);
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
	perl_run(my_perl);

	dSP;						//initialize stack pointer
	ENTER;						//everything created after here
	SAVETMPS;					//...is a temporary variable.
	PUSHMARK(SP);			 		//remember the stack pointer
	
	//Load our script into the interpereter
	eval_pv(scriptText.latin1(), TRUE);
	
	//Push all our arguments onto the Perl stack
	for ( QStringList::Iterator it = args.begin(); it != args.end(); ++it )
	{
        	XPUSHs(sv_2mortal(newSVpv( (*it).latin1(), 0 )));
	}
		
	PUTBACK;					//make local stack pointer global
	call_pv(subName.local8Bit(), G_SCALAR);		//call the function
	SPAGAIN;					//refresh stack pointer

	QString result = QString::fromLatin1(POPp);	//pop the return value from stack
	
	PUTBACK;
	FREETMPS;					//free that return value
	LEAVE;						//...and the XPUSHed "mortal" args.

	return result;
}

PerlScript::PerlScript( const QString &scriptPath, const QString &scriptName, const QString &desc )
{
	m_localFile = 0L;
	name = scriptName;
	path = scriptPath;
	description = desc;
	load();
}

PerlScript::~PerlScript()
{
	if( m_localFile )
	{
		m_localFile->close();
		m_localFile->unlink();
	}
}

void PerlScript::load()
{
	QString m_localPath;
	
	//If this isn't a local file, make a temp copy so we can read it
	if( !KURL(path).isLocalFile() )
	{
		m_localFile = new KTempFile( QString::null, ".pl" );
		m_localPath = m_localFile->name();
		if ( !KIO::NetAccess::download(KURL(path), m_localPath) )
		{
			KNotifyClient::event("cannotopenfile");
			return;
		}
		m_localFile->close();
	}
	else
	{
		m_localPath = path;
	}
	
	//Read in the file
	QFile f(m_localPath);
	if ( !f.open(IO_ReadOnly) )
	{
		KNotifyClient::event("cannotopenfile");
	}
	else
	{
		scriptText = QString( f.readAll() );
		f.close();
	}
}

