
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

class KopeteAccount;
class KopeteContact;
class KopeteMessageManager;
class JSMessage;
class JavaScriptConfig;
class JavaScriptPluginPrivate;

namespace KJSEmbed
{
	class JSOpaqueProxy;
	class KJSEmbedPart;
}

class JavaScriptPlugin : public KopetePlugin
{
	Q_OBJECT

	public:
		static JavaScriptPlugin  *self();

		JavaScriptPlugin( QObject *parent, const char *name, const QStringList &args );
		~JavaScriptPlugin();

	private slots:
		void slotReloadScripts();

		void slotAccountCreated( KopeteAccount *a );
		void slotAccountDestroyed( KopeteAccount *a );

		void slotIncomingMessage( KopeteMessage& msg );
		void slotOutgoingMessage( KopeteMessage& msg );
		void slotDisplayMessage( KopeteMessage& msg );

		void slotAccountChangedStatus( KopeteAccount *c, const KopeteOnlineStatus &,
			const KopeteOnlineStatus & );

		void slotContactChangedStatus( KopeteContact *c, const KopeteOnlineStatus &,
			const KopeteOnlineStatus & );

		void slotContactAdded( KopeteContact *c );
		void slotContactRemoved( KopeteContact *c );

		void slotShowConsole( const QString &, KopeteMessageManager *manager );
		void slotJsExec( const QString &, KopeteMessageManager *manager );

	private:
		void execScripts( KopeteAccount *a );
		void runScripts( KopeteAccount *a, const QString &scriptType, KJSEmbed::KJSEmbedPart *engine );
		void publishMessage( JSMessage *msg, KJSEmbed::KJSEmbedPart *engine );

		static JavaScriptPlugin* pluginStatic_;

		JavaScriptPluginPrivate *d;
};

#endif


