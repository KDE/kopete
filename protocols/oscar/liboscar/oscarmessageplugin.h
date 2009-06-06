/*
    Kopete Oscar Protocol
    oscarmessageplugin.h - Oscar Message Plugin

    Copyright (c) 2006-2007 Roman Jarosz <kedgedev@centrum.cz>

    Kopete (c) 2006-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef OSCARMESSAGEPLUGIN_H
#define OSCARMESSAGEPLUGIN_H

#include <QSharedDataPointer>

#include "kopete_export.h"
#include "oscartypes.h"

namespace Oscar
{

class LIBOSCAR_EXPORT MessagePlugin
{
public:

	enum Types { Unknown = 0, Message, StatusMsgExt, File, WebUrl,
	             Contacts, GreetingCard, Chat, XtrazScript };

	enum SubTypes {
		SubStandardSend     = 0x00,
		SubContactsRequest  = 0x02,
		SubScriptInvitation = 0x01,
		SubScriptData       = 0x02,
		SubScriptUserRemove = 0x04,
		SubScriptNotify     = 0x08
	};

	MessagePlugin();
	MessagePlugin( const MessagePlugin& mp );
	MessagePlugin& operator=( const MessagePlugin& mp );
	~MessagePlugin();

	/** get the message plugin type */
	Types type() const;

	/** get the message plugin guid */
	Guid guid() const;

	/** set the message plugin type */
	void setType( Types type );

	/** set the message plugin type from GUID */
	void setType( Guid guid );

	/** get the message plugin subtype */
	WORD subTypeId() const;

	/** set the message plugin subtype */
	void setSubTypeId( WORD subType );

	/** get the message plugin subtype text */
	QByteArray subTypeText() const;

	/** set the message plugin subtype text */
	void setSubTypeText( const QByteArray& text );

	/** get the message plugin data */
	QByteArray data() const;

	/** set the message plugin data */
	void setData( const QByteArray &data );

private:
	class MessagePluginPrivate;
	QSharedDataPointer<MessagePluginPrivate> d;

};

}

#endif
