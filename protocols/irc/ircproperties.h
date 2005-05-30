/*
    ircprotocol.h - IRC Protocol

    Copyright (c) 2005      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2005      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCPROPERTIES_H
#define IRCPROPERTIES_H

#include "kopetecontactproperty.h"

namespace IRC
{
namespace Properties
{

// irc channnel-contact properties
const Kopete::ContactPropertyTmpl ChannelHomepage;
const Kopete::ContactPropertyTmpl ChannelMembers;
const Kopete::ContactPropertyTmpl ChannelTopic;

// irc user-contact properties
//const Kopete::ContactPropertyTmpl LastSeen;
const Kopete::ContactPropertyTmpl UserInfo;
const Kopete::ContactPropertyTmpl Server;
const Kopete::ContactPropertyTmpl Channels;
const Kopete::ContactPropertyTmpl Hops;
const Kopete::ContactPropertyTmpl FullName;
const Kopete::ContactPropertyTmpl IsIdentified;

}
}

#endif
