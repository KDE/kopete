/*
    Kopete Oscar Protocol
    oscarmessageplugin.cpp - Oscar Message Plugin

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

#include "oscarmessageplugin.h"

#include <QSharedData>

#include <kdebug.h>

#define GUID_UNKNOWN       "00000000000000000000000000000000"
#define GUID_MESSAGE       "BE6B73050FC2104FA6DE4DB1E3564B0E"
#define GUID_STATUSMSGEXT  "811A18BC0E6C1847A5916F18DCC76F1A"
#define GUID_FILE          "F02D12D93091D3118DD700104B06462E"
#define GUID_WEBURL        "371C5872E987D411A4C100D0B759B1D9"
#define GUID_CONTACTS      "2A0E7D467676D411BCE60004AC961EA6"
#define GUID_GREETINGCARD  "01E53B482AE4D111B679006097E1E294"
#define GUID_CHAT          "BFF720B2378ED411BD280004AC96D905"
#define GUID_XTRAZSCRIPT   "3B60B3EFD82A6C45A4E09C5A5E67E865"

namespace Oscar
{

class MessagePlugin::MessagePluginPrivate : public QSharedData
{
public:
	Types type;
	WORD subType;
	QByteArray subTypeText;
	QByteArray data;
};

MessagePlugin::MessagePlugin()
	: d( new MessagePluginPrivate )
{
	d->type = Unknown;
	d->subType = SubStandardSend;
}

MessagePlugin::MessagePlugin( const MessagePlugin& mp )
	: d( mp.d )
{
}

MessagePlugin& MessagePlugin::operator=( const MessagePlugin& mp )
{
	d = mp.d;
	return *this;
}

MessagePlugin::~MessagePlugin()
{
}

MessagePlugin::Types MessagePlugin::type() const
{
	return d->type;
}

Guid MessagePlugin::guid() const
{
	switch ( d->type )
	{
	case Message:
		return Guid( QLatin1String( GUID_MESSAGE ) );
	case StatusMsgExt:
		return Guid( QLatin1String( GUID_STATUSMSGEXT ) );
	case File:
		return Guid( QLatin1String( GUID_FILE ) );
	case WebUrl:
		return Guid( QLatin1String( GUID_WEBURL ) );
	case Contacts:
		return Guid( QLatin1String( GUID_CONTACTS ) );
	case GreetingCard:
		return Guid( QLatin1String( GUID_GREETINGCARD ) );
	case Chat:
		return Guid( QLatin1String( GUID_CHAT ) );
	case XtrazScript:
		return Guid( QLatin1String( GUID_XTRAZSCRIPT ) );
	default:
		return Guid( QLatin1String( GUID_UNKNOWN ) );
	}
}

void MessagePlugin::setType( MessagePlugin::Types type )
{
	d->type = type;
}


void MessagePlugin::setType( Guid guid )
{
	if ( guid == Guid( QLatin1String( GUID_MESSAGE ) ) )
		d->type = Message;
	else if ( guid == Guid( QLatin1String( GUID_STATUSMSGEXT ) ) )
		d->type = StatusMsgExt;
	else if ( guid == Guid( QLatin1String( GUID_FILE ) ) )
		d->type = File;
	else if ( guid == Guid( QLatin1String( GUID_WEBURL ) ) )
		d->type = WebUrl;
	else if ( guid == Guid( QLatin1String( GUID_CONTACTS ) ) )
		d->type = Contacts;
	else if ( guid == Guid( QLatin1String( GUID_GREETINGCARD ) ) )
		d->type = GreetingCard;
	else if ( guid == Guid( QLatin1String( GUID_CHAT ) ) )
		d->type = Chat;
	else if ( guid == Guid( QLatin1String( GUID_XTRAZSCRIPT ) ) )
		d->type = XtrazScript;
	else
		d->type = Unknown;
}

WORD MessagePlugin::subTypeId() const
{
	return d->subType;
}

void MessagePlugin::setSubTypeId( WORD subType )
{
	d->subType = subType;
}

QByteArray MessagePlugin::subTypeText() const
{
	return d->subTypeText;
}

void MessagePlugin::setSubTypeText( const QByteArray& text )
{
	d->subTypeText = text;
}

QByteArray MessagePlugin::data() const
{
	return d->data;
}

void MessagePlugin::setData( const QByteArray &data )
{
	d->data = data;
}

}
