/*
    pipesplugin.cpp

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
#include "pipesplugin.h"
#include "pipesconfig.h"

#include <kpluginfactory.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <KMessageBox>
#include <KLocalizedString>

#include <QProcess>
#include <QDomDocument>

#include "kopetesimplemessagehandler.h"
#include "kopetechatsessionmanager.h"
#include "kopetemessage.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteaccount.h"
#include "kopeteprotocol.h"
#include "kopetepicture.h"
#include "kopeteview.h"
#include "kopeteuiglobal.h"

K_PLUGIN_FACTORY ( PipesPluginFactory, registerPlugin<PipesPlugin>(); )
K_EXPORT_PLUGIN ( PipesPluginFactory ( "kopete_pipes" ) )

PipesPlugin::PipesPlugin ( QObject *parent, const QVariantList &/*args*/ )
		: Kopete::Plugin ( PipesPluginFactory::componentData(), parent )
{
	if( !mPluginStatic )
		mPluginStatic = this;
	
	// intercept inbound messages
	mInboundHandler = new Kopete::SimpleMessageHandlerFactory ( Kopete::Message::Inbound,
	        Kopete::MessageHandlerFactory::InStageToDesired, this, SLOT (slotIncomingMessage(Kopete::Message&)) );
	// intercept outbound messages
	connect ( Kopete::ChatSessionManager::self(),
	          SIGNAL (aboutToSend(Kopete::Message&)),
	          SLOT (slotOutgoingMessage(Kopete::Message&)) );
}

PipesPlugin::~PipesPlugin()
{
	delete mInboundHandler;
	mPluginStatic = 0;
}

PipesPlugin * PipesPlugin::plugin()
{
	return mPluginStatic;
}

PipesPlugin* PipesPlugin::mPluginStatic = 0L;

void PipesPlugin::slotIncomingMessage ( Kopete::Message & msg )
{
	PipesConfig::self()->load();
	// for each pipe, run it if it is enabled and set for this direction
	foreach ( PipeOptions pipeOptions, PipesConfig::pipes() )
	{
		if ( ! pipeOptions.enabled )
			continue;
		if ( ! ( ( pipeOptions.direction & Inbound ) && ( msg.direction() == Kopete::Message::Inbound ) ) )
			continue;
		doPiping ( msg, pipeOptions );
	}

}

void PipesPlugin::slotOutgoingMessage ( Kopete::Message & msg )
{
	PipesConfig::self()->load();
	// for each pipe, run it if it is enabled and set for this direction
	foreach ( PipeOptions pipeOptions, PipesConfig::pipes() )
	{
		if ( ! pipeOptions.enabled )
			continue;
		if ( ! ( ( pipeOptions.direction & Outbound ) && ( msg.direction() == Kopete::Message::Outbound ) ) )
			continue;
		doPiping ( msg, pipeOptions );
	}
}

void PipesPlugin::doPiping ( Kopete::Message & msg, PipeOptions pipeOptions )
{
	kDebug ( 14316 ) << "pipe" << pipeOptions.path;
	
	// start pipe with no arguments
	QProcess pipe;
	pipe.start ( pipeOptions.path, QStringList(), QIODevice::ReadWrite );
	pipe.waitForStarted();

	// poop out appropriate data
	if ( pipeOptions.pipeContents == HtmlBody )
		pipe.write ( msg.escapedBody().toUtf8() );
	else if ( pipeOptions.pipeContents == PlainBody )
		pipe.write ( msg.plainBody().toUtf8() );
	else if ( pipeOptions.pipeContents == Xml )
		pipe.write ( createXml ( msg ) );

	pipe.closeWriteChannel();
	pipe.waitForFinished();

	QByteArray pipeReturn = pipe.readAllStandardOutput();

	// set data in message to output of pipe
	if ( pipeOptions.pipeContents == HtmlBody )
		msg.setHtmlBody ( QString::fromUtf8( pipeReturn ) );
	else if ( pipeOptions.pipeContents == PlainBody )
		msg.setPlainBody ( QString::fromUtf8( pipeReturn ) );
	else if ( pipeOptions.pipeContents == Xml )
		readXml ( pipeOptions, msg, pipeReturn );
}

QByteArray PipesPlugin::createXml ( const Kopete::Message & msg )
{
	/*
	Here's an example of what a pipee will get:

	<?xml version="1.0" encoding="UTF-8"?>
	<message subject=""
		route="outbound"
		importance="1"
		formattedTimestamp="11:06:46 am"
		timestamp="Sat Dec 1 11:06:46 2007"
		type="normal"
		mainContactId="spacemonkey1234"
		time="11:06 am" >
		<from>
			<contact contactId="tdurden"
				protocolIcon="aim_protocol"
				userPhoto="/home/kde-devel/.kde4/share/apps/kopete/oscarpictures/tdurden.jpg" >
				<contactDisplayName dir="ltr" text="tdurden" />
				<metaContactDisplayName dir="ltr"
					text="Tyler" />
			</contact>
		</from>
		<to>
			<contact contactId="spacemonkey1234"
				protocolIcon="aim_protocol"
				userPhoto="/home/kde-devel/.kde4/share/apps/kopete/avatars/User/7e542903e0ac7519a7496e85f5a5b99d.png" >
				<contactDisplayName dir="ltr" text="spacemonkey1234" />
				<metaContactDisplayName dir="ltr" text="Robert Paulson" />
			</contact>
			</to>
		<body dir="ltr"
			bgcolor="#000000"
			color="#000000" >
			&lt;p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">&lt;span style=" color:#000000;">hey6&lt;/span>&lt;/p></body>
	</message>

	This XML is based on the XSD found at http://kopete.kde.org/files/kopetemessage.xsd,
	which is used to create chatstyles.
	Given that our situation here is that of simply passing a message through, we can't conform exactly to this standard.
	Specific notes for *output* only:
	- The <?Kopete Flag:TransformAllMessages> is totally irrevelant and not dealt with.
	- The "document" element is not dealt with. Messages are done on a single basis.
	- The "timestamp" attribute in the "message" element is not exactly as it is shown in the example in the XSD document.
	  It is instead outputted in an ISO 8601 standard format
	- The "color" attribute in the "contact" element is not handled. This is because Pipes plugin is only aware
	  of the message itself, not how it is displayed. That is decided later.
	- The "protocolIcon" attribute in the "contact" element is chosen like this:
	  - If the Contact object has the photo property set, that path is used.
	  - If not, the Contact's MetaContact is asked for a picture path, and that is used regardless of being null or not
	- The "font" attribute in the complexType "messageBody" is not dealt with. This is because, like with "color",
	  the message itself does not have information on how it is displayed.
	- The "color" and "bgcolor" attributes in the "body" element are empty strings if they are not set in Kopete.
	  They are outputed web color format, not named format.
	- The content inside the "body" element has "<" escaped, so it is not confused with the actual XML structure.
	  Obviously, to correctly interpret the body as HTML, one would have to convert the "&lt;"s to "<"s.
	*/

	QDomDocument doc;

	// Set all the elements and attributes that will appear in the XML.
	// These pieces of info do not only come from the Message object, but
	// from other classes that are linked from the message.
	QDomElement message = doc.createElement ( "message" );
	doc.appendChild ( message );
	message.setAttribute ( "time", msg.timestamp().time().toString ( "h:mm ap" ) );
	message.setAttribute ( "timestamp", msg.timestamp().toString ( "ddd MMM d hh:mm:ss yyyy" ) );
	message.setAttribute ( "formattedTimestamp", msg.timestamp().time().toString ( "hh:mm:ss ap" ) );
	message.setAttribute ( "subject", msg.subject() );
	switch ( msg.direction() )
	{
		case Kopete::Message::Inbound: message.setAttribute ( "route", "inbound" ); break;
		case Kopete::Message::Outbound: message.setAttribute ( "route", "outbound" ); break;
		case Kopete::Message::Internal: message.setAttribute ( "route", "internal" ); break;
		default: break;
	}
	switch ( msg.type() )
	{
		case Kopete::Message::TypeAction: message.setAttribute ( "type", "action" ); break;
		case Kopete::Message::TypeNormal: message.setAttribute ( "type", "normal" ); break;
		default: break;
	}
	switch ( msg.importance() )
	{
		case Kopete::Message::Low: message.setAttribute ( "importance", 0 ); break;
		case Kopete::Message::Normal: message.setAttribute ( "importance", 1 ); break;
		case Kopete::Message::Highlight: message.setAttribute ( "importance", 2 ); break;
		default: break;
	}
	message.setAttribute ( "mainContactId", msg.to().first()->contactId() );

	QDomElement from = doc.createElement ( "from" );
	message.appendChild ( from );
	QDomElement fromContact = doc.createElement ( "contact" );
	from.appendChild ( fromContact );
	fromContact.setAttribute ( "contactId", msg.from()->contactId() );
	// if the Contact has a photo, use that, else use metacontact photo
	if ( ! msg.from()->property ( Kopete::Global::Properties::self()->photo().key() ).value().toString().isNull() )
		fromContact.setAttribute ( "userPhoto",
			msg.from()->property ( Kopete::Global::Properties::self()->photo().key() ).value().toString() );
	else
		fromContact.setAttribute ( "userPhoto", msg.from()->metaContact()->picture().path() );
	fromContact.setAttribute ( "protocolIcon", msg.from()->protocol()->pluginIcon() );

	QDomElement contactDisplayName = doc.createElement ( "contactDisplayName" );
	fromContact.appendChild ( contactDisplayName );
	contactDisplayName.setAttribute ( "dir", msg.from()->formattedName().isRightToLeft() ? "rtl" : "ltr" );
	contactDisplayName.setAttribute ( "text", msg.from()->displayName() );

	QDomElement metaContactDisplayName = doc.createElement ( "metaContactDisplayName" );
	fromContact.appendChild ( metaContactDisplayName );
	metaContactDisplayName.setAttribute ( "dir", msg.from()->metaContact()->displayName().isRightToLeft() ? "rtl" : "ltr" );
	metaContactDisplayName.setAttribute ( "text", msg.from()->metaContact()->displayName() );

	QDomElement to = doc.createElement ( "to" );
	message.appendChild ( to );
	foreach ( Kopete::Contact *contact, msg.to() )
	{
		QDomElement toContact = doc.createElement ( "contact" );
		to.appendChild ( toContact );
		toContact.setAttribute ( "contactId", contact->contactId() );
		if ( ! contact->property ( Kopete::Global::Properties::self()->photo().key() ).value().toString().isNull() )
			toContact.setAttribute ( "userPhoto", contact->property ( Kopete::Global::Properties::self()->photo().key() ).value().toString() );
		else
			toContact.setAttribute ( "userPhoto", contact->metaContact()->picture().path() );
		toContact.setAttribute ( "protocolIcon", contact->protocol()->pluginIcon() );

		QDomElement contactDisplayName = doc.createElement ( "contactDisplayName" );
		toContact.appendChild ( contactDisplayName );
		contactDisplayName.setAttribute ( "dir", contact->formattedName().isRightToLeft() ? "rtl" : "ltr" );
		contactDisplayName.setAttribute ( "text", contact->displayName() );

		QDomElement metaContactDisplayName = doc.createElement ( "metaContactDisplayName" );
		toContact.appendChild ( metaContactDisplayName );
		metaContactDisplayName.setAttribute ( "dir", contact->metaContact()->displayName().isRightToLeft() ? "rtl" : "ltr" );
		metaContactDisplayName.setAttribute ( "text", contact->metaContact()->displayName() );
	}

	QDomElement body = doc.createElement ( "body" );
	message.appendChild ( body );
	body.setAttribute ( "color", msg.foregroundColor().isValid() ? msg.foregroundColor().name() : "" );
	body.setAttribute ( "bgcolor", msg.backgroundColor().isValid() ? msg.backgroundColor().name() : "" );
	body.setAttribute ( "dir", msg.plainBody().isRightToLeft() ? "rtl" : "ltr" );
	body.appendChild ( doc.createTextNode ( msg.escapedBody() ) );

	QDomNode xsdInstruction = doc.createProcessingInstruction ( "xml", "version=\"1.0\" encoding=\"UTF-8\"" );
	doc.insertBefore ( xsdInstruction, doc.firstChild() );

	return doc.toByteArray();
}

void PipesPlugin::readXml ( PipeOptions pipeOptions, Kopete::Message & msg, const QByteArray & pipeReturn )
{
	/* modification to the following attributes/elements in the XML will be reflected back into the message:
	- "timestamp" but *not* other time attributes
	- "subject"
	- "type"
	- "importance"
	- "color" in "body"
	  This is usually set to a web color.
	  The names defined at http://www.w3.org/TR/SVG/types.html#ColorKeywords are accepted by
	  QColor in Qt4, so those can be set too.
	- "bgcolor" in "body" - same behaviour as "color"
	- content of "body"
	*/

	QDomDocument doc;
	QString readError;
	int errorLine, errorCol;

	// if the pipe returns bad XML, tell user and do not process.
	if ( ! doc.setContent ( pipeReturn, false/*namespaceProcessing*/, &readError, &errorLine, &errorCol ) )
	{
		KMessageBox::error ( msg.manager()->view()->mainWidget() ?
		                     msg.manager()->view()->mainWidget() : Kopete::UI::Global::mainWidget(),
		                     i18n ( "The pipe %1 returned malformed XML to the Pipes plugin."
		                            "The error is:\n\n%2\n\nLocated at line %3 and column %4",
		                            pipeOptions.path, readError, errorLine, errorCol ) );
	}
	
	// take some choice attributes from XML and put them back in Message
	else
	{
		QDomElement message = doc.firstChildElement ( "message" );
		msg.setTimestamp ( QDateTime::fromString ( message.attribute ( "timestamp" ), "ddd MMM d hh:mm:ss yyyy" ) );
		msg.setSubject ( message.attribute ( "subject" ) );

		if ( message.attribute ( "type" ) == "normal" )
			msg.setType ( Kopete::Message::TypeNormal );
		else if ( message.attribute ( "type" ) == "action" )
			msg.setType ( Kopete::Message::TypeAction );

		if ( message.attribute ( "importance" ) == "0" )
			msg.setImportance ( Kopete::Message::Low );
		else if ( message.attribute ( "importance" ) == "1" )
			msg.setImportance ( Kopete::Message::Normal );
		else if ( message.attribute ( "importance" ) == "2" )
			msg.setImportance ( Kopete::Message::Highlight );

		QDomElement body = message.firstChildElement ( "body" );
		msg.setForegroundColor ( QColor ( body.attribute ( "color" ) ) );
		msg.setBackgroundColor ( QColor ( body.attribute ( "bgcolor" ) ) );
		msg.setHtmlBody ( body.text() );
	}
}


