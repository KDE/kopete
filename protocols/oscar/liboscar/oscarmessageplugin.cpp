/*
    Kopete Oscar Protocol
    oscarmessageplugin.cpp - Oscar Message Plugin

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

#include "oscarmessageplugin.h"

class Oscar::MessagePlugin::MessagePluginPrivate
{
public:
	int type;
};

Oscar::MessagePlugin::MessagePlugin()
: d( new MessagePluginPrivate )
{
	d->type = Unknown;
}

Oscar::MessagePlugin::~MessagePlugin()
{
	delete d;
}

int Oscar::MessagePlugin::type() const
{
	return d->type;
}

void Oscar::MessagePlugin::setType( int type )
{
	d->type = type;
}

