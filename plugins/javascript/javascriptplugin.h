
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOPETE_JAVASCRIPT_PLUGIN_H
#define KOPETE_JAVASCRIPT_PLUGIN_H

#include "kopeteplugin.h"
#include "kopetemessage.h"

namespace Kopete
{
	class Account;
	class Contact;
	class ChatSession;
}

class JSMessage;
class JavaScriptPluginPrivate;

namespace KJSEmbed
{
	class Engine;
}

class JavaScriptPlugin
	: public Kopete::Plugin
{
	Q_OBJECT

public:
	static JavaScriptPlugin  *self();

	JavaScriptPlugin( QObject *parent, const QStringList &args );
	~JavaScriptPlugin();

private slots:
	void slotReloadScripts();
/*
	void slotAccountCreated( Kopete::Account *a );
	void slotAccountDestroyed( Kopete::Account *a );
	void slotAccountChangedStatus( Kopete::Account *c,
		const Kopete::OnlineStatus &,
		const Kopete::OnlineStatus & );

	void slotContactAdded( Kopete::Contact *c );
	void slotContactRemoved( Kopete::Contact *c );
	void slotContactChangedStatus( Kopete::Contact *c,
		const Kopete::OnlineStatus &,
		const Kopete::OnlineStatus & );

	void slotIncomingMessage( Kopete::Message& msg );
	void slotOutgoingMessage( Kopete::Message& msg );
	void slotDisplayMessage( Kopete::Message& msg );
*/
	void slotShowConsole( const QString &, Kopete::ChatSession *manager );
	void slotJsExec( const QString &, Kopete::ChatSession *manager );

private:
	void execScripts( Kopete::Account *a );
	void runScripts( Kopete::Account *a, const QString &scriptType,
		KJSEmbed::Engine *jsEngine );
//	void publishMessage( JSMessage *msg, KJSembed::Engine *jsEngine );

	static JavaScriptPlugin* pluginStatic_;

	JavaScriptPluginPrivate *d;
};

#endif

