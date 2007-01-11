/*
    ircconst.h - The IRC constants & enums.

    Copyright (c) 2005-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2005-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCCONST_H
#define IRCCONST_H

#include "kopetecontactproperty.h"

namespace IRC {

enum ChatSessionType
{
	SERVER,
	CHANNEL,
	DCC
};

extern const QString Version;

namespace Config {

//extern const QLatin1String AUTOSHOWSERVERWINDOW;
extern const QLatin1String CODECMIB;
extern const QLatin1String NETWORKNAME;
extern const QLatin1String NICKNAME;
extern const QLatin1String USERNAME;
extern const QLatin1String REALNAME;

} // namespace IRC::Config

namespace Properties {

// irc channnel-contact properties
extern const Kopete::ContactPropertyTmpl ChannelHomepage;
extern const Kopete::ContactPropertyTmpl ChannelMembers;
extern const Kopete::ContactPropertyTmpl ChannelTopic;

// irc user-contact properties
//extern const Kopete::ContactPropertyTmpl LastSeen;
extern const Kopete::ContactPropertyTmpl UserInfo;
extern const Kopete::ContactPropertyTmpl Server;
extern const Kopete::ContactPropertyTmpl Channels;
extern const Kopete::ContactPropertyTmpl Hops;
extern const Kopete::ContactPropertyTmpl FullName;
extern const Kopete::ContactPropertyTmpl IsIdentified;

} // namespace IRC::Properties

} // namespace IRC

#endif

