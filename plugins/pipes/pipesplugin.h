/*
    pipesplugin.h

    Copyright (c) 2007      by Charles Connell        <charles@connells.org>

    Kopete    (c) 2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef PIPESPLUGIN_H
#define PIPESPLUGIN_H

#include "kopeteplugin.h"

#include <QUuid>
#include <QDomDocument>
#include <QVariantList>

namespace Kopete
{
	class SimpleMessageHandlerFactory;
	class Message;
}

/**
 * Core functions of Pipes plugin.
 * @author Charles Connell <charles@connells.org>
 */

class PipesPlugin : public Kopete::Plugin
{
		Q_OBJECT

	public:
		/*
		 * Used to indicate direction that a pipe is used for
		 */
		enum PipeDirection { Inbound = 0x1, Outbound = 0x2, BothDirections = Inbound | Outbound };

		/*
		 * Used to indicate what should be outputted, and what the input should be interpreted as
		 */
		enum PipeContents { HtmlBody = 0, PlainBody = 1, Xml = 2 };

		/*
		 * Stores everything we need to know about a pipe
		 */
		class PipeOptions
		{
			public:
				QUuid uid;
				bool enabled;
				QString path;
				PipeDirection direction;
				PipeContents pipeContents;
		};
		typedef QList<PipeOptions> PipeOptionsList;

	public:
		static PipesPlugin* plugin();
		
		PipesPlugin ( QObject *parent, const QVariantList &args );
		~PipesPlugin();

	private slots:
		/*
		 * Grab incoming message, call doPiping for each
		 * appropriate pipe
		 */
		void slotIncomingMessage ( Kopete::Message & );

		/*
		 * Grab outgoing message, call doPiping for each
		 * appropriate pipe
		 */
		void slotOutgoingMessage ( Kopete::Message & );

	private:
		/*
		 * Fork process, push appropriate output to it,
		 * then read it's output and appropriately put it
		 * back in the message.
		 */
		static void doPiping ( Kopete::Message &, PipeOptions );

		/*
		 * Turn a Message into a QDomDocument, return that XML.
		 * Info for the XML is pulled from all over Kopete.
		 */
		static QByteArray createXml ( const Kopete::Message & );

		/*
		 * Take a QByteArray containing XML, take pertinent info from
		 * that, and put it in the Message.
		 */
		static void readXml ( PipeOptions, Kopete::Message &, const QByteArray & );

	private:
		static PipesPlugin* mPluginStatic;
		PipeOptionsList mPipesList;
		Kopete::SimpleMessageHandlerFactory * mInboundHandler;

};

Q_DECLARE_METATYPE ( PipesPlugin::PipeDirection )
Q_DECLARE_METATYPE ( PipesPlugin::PipeContents )

#endif
