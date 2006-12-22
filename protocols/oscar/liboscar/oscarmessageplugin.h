/*
    Kopete Oscar Protocol
    oscarmessageplugin.h - Oscar Message Plugin

    Copyright (c) 2006 Roman Jarosz <kedgedev@centrum.cz>

    Kopete (c) 2006 by the Kopete developers <kopete-devel@kde.org>

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

#include "kopete_export.h"
#include "oscartypes.h"

namespace Oscar
{

class KOPETE_EXPORT MessagePlugin
{
public:

	enum Types { Unknown = 0, Message, StatusMsgExt, File, WebUrl,
	             Contacts, GreetingCard, Chat, XtrazScript };

	MessagePlugin();
	~MessagePlugin();

	/** get the message plugin type */
	int type() const;

	/** set the message plugin type */
	void setType( int type );

private:
	class MessagePluginPrivate;
	MessagePluginPrivate* d;

};

}

#endif
