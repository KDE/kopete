
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

#include "kopetemessage.h"
#include "kopeteplugin.h"

class KopeteContact;
class KopeteMessageManager;
class Message;
class JavaScriptConfig;

namespace KJSEmbed
{
	class JSOpaqueProxy;
	class KJSEmbedPart;
}

class JavaScriptPlugin : public KopetePlugin
{
	Q_OBJECT

	public:
		enum ScriptType
		{
			Incoming = 0,
			Outgoing = 1,
			Display = 2
		};

		static JavaScriptPlugin  *self();

		JavaScriptPlugin( QObject *parent, const char *name, const QStringList &args );
		~JavaScriptPlugin();

	private slots:
		void slotIncomingMessage( KopeteMessage& msg );
		void slotOutgoingMessage( KopeteMessage& msg );
		void slotDisplayMessage( KopeteMessage& msg );

		void slotShowConsole();

	private:
		void runScripts( KopeteMessageManager *manager, ScriptType type );
		void publishMessage( KopeteMessage &msg, ScriptType type );

		static JavaScriptPlugin* pluginStatic_;

		JavaScriptConfig *config;
		KJSEmbed::KJSEmbedPart *jsEngine;
		KJSEmbed::JSOpaqueProxy *messageProxy;
};

#endif


