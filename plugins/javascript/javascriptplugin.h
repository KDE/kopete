
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _JAVASCRIPT_PLUGIN_H
#define _JAVASCRIPT_PLUGIN_H

#include "kopeteplugin.h"

#include <qptrdict.h>
#include <kjs/object.h>

#include "wrappers/kopeteonlinestatus_imp.h"

namespace Kopete { class Account; }
namespace Kopete { class Contact; }
namespace Kopete { class ChatSession; }
class JSMessage;
class JavaScriptConfig;
class JavaScriptPluginPrivate;

namespace KJSEmbed
{
	class JSOpaqueProxy;
	class KJSEmbedPart;
}

class JavaScriptPlugin : public Kopete::Plugin
{
	Q_OBJECT

	public:
		static JavaScriptPlugin  *self();

		JavaScriptPlugin( QObject *parent, const char *name, const QStringList &args );
		~JavaScriptPlugin();

	private slots:
		void slotReloadScripts();

		void slotAccountCreated( Kopete::Account *a );
		void slotAccountDestroyed( Kopete::Account *a );

		void slotIncomingMessage( Kopete::Message& msg );
		void slotOutgoingMessage( Kopete::Message& msg );
		void slotDisplayMessage( Kopete::Message& msg );

		void slotAccountChangedStatus( Kopete::Account *c, const Kopete::OnlineStatus &,
			const Kopete::OnlineStatus & );

		void slotContactChangedStatus( Kopete::Contact *c, const Kopete::OnlineStatus &,
			const Kopete::OnlineStatus & );

		void slotContactAdded( Kopete::Contact *c );
		void slotContactRemoved( Kopete::Contact *c );

		void slotShowConsole( const QString &, Kopete::ChatSession *manager );
		void slotJsExec( const QString &, Kopete::ChatSession *manager );

	private:
		void execScripts( Kopete::Account *a );
		void runScripts( Kopete::Account *a, const QString &scriptType, KJSEmbed::KJSEmbedPart *engine );
		void publishMessage( JSMessage *msg, KJSEmbed::KJSEmbedPart *engine );

		static JavaScriptPlugin* pluginStatic_;

		JavaScriptPluginPrivate *d;
};

#endif


