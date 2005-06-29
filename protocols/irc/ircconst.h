/*
    ircconst.h - The IRC constants & enums.

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

#ifndef IRCCONST_H
#define IRCCONST_H

#include "kopetecontactproperty.h"

namespace IRC {

extern const QString Version;

namespace Config {

//extern const QString AUTOSHOWSERVERWINDOW;
extern const QString CODECMIB;
extern const QString NETWORKNAME;
extern const QString NICKNAME;
extern const QString USERNAME;
extern const QString REALNAME;

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

