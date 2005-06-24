/*
    ircconst.cpp - The IRC constants & enums. 

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

#include "ircconst.h"

#include "klocale.h"

using namespace Kopete;

/* The usage of the namespace, instead of the "using" keyword, is intentional.
 * Not using it introduce compiler confusion, and lead to new symbols declaration.
 */

namespace IRC {

namespace Config {

//const QString CONFIG_AUTOSHOWSERVERWINDOW = QString::fromLatin1("AutoShowServerWindow");
const QString CODECMIB = QString::fromLatin1("Codec");
const QString NETWORKNAME = QString::fromLatin1("NetworkName");
const QString NICKNAME = QString::fromLatin1("NickName");
const QString USERNAME = QString::fromLatin1("UserName");
const QString REALNAME = QString::fromLatin1("RealName");

} // IRC::Config

namespace Properties {

const ContactPropertyTmpl ChannelTopic(QString::fromLatin1("channelTopic"), i18n("Topic"), QString::null, false, true );
const ContactPropertyTmpl ChannelMembers(QString::fromLatin1("channelMembers"), i18n("Members"));
const ContactPropertyTmpl ChannelHomepage(QString::fromLatin1("homePage"), i18n("Home Page"));

//const ContactPropertyTmpl LastSeen(Kopete::Global::IRC::Properties::self()->lastSeen());
const ContactPropertyTmpl UserInfo(QString::fromLatin1("userInfo"), i18n("IRC User"));
const ContactPropertyTmpl Server(QString::fromLatin1("ircServer"), i18n("IRC Server"));
const ContactPropertyTmpl Channels( QString::fromLatin1("ircChannels"), i18n("IRC Channels"));
const ContactPropertyTmpl Hops(QString::fromLatin1("ircHops"), i18n("IRC Hops"));
const ContactPropertyTmpl FullName(QString::fromLatin1("FormattedName"), i18n("Full Name"));
const ContactPropertyTmpl IsIdentified(QString::fromLatin1("identifiedUser"), i18n("User Is Authenticated"));

} // IRC::Properties

} // IRC
