/*
    ircproperties - The IRC Contact properites

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

#include "ircproperties.h"

#include "klocale.h"

using namespace IRC::Properties;
using namespace Kopete;

ContactPropertyTmpl ChannelTopic(QString::fromLatin1("channelTopic"), i18n("Topic"), QString::null, false, true );

ContactPropertyTmpl ChannelMembers(QString::fromLatin1("channelMembers"), i18n("Members"));

ContactPropertyTmpl ChannelHomepage(QString::fromLatin1("homePage"), i18n("Home Page"));



//ContactPropertyTmpl LastSeen(Kopete::Global::Properties::self()->lastSeen());

ContactPropertyTmpl UserInfo(QString::fromLatin1("userInfo"), i18n("IRC User"));

ContactPropertyTmpl Server(QString::fromLatin1("ircServer"), i18n("IRC Server"));

ContactPropertyTmpl Channels( QString::fromLatin1("ircChannels"), i18n("IRC Channels"));

ContactPropertyTmpl Hops(QString::fromLatin1("ircHops"), i18n("IRC Hops"));

ContactPropertyTmpl FullName(QString::fromLatin1("FormattedName"), i18n("Full Name"));

ContactPropertyTmpl IsIdentified(QString::fromLatin1("identifiedUser"), i18n("User Is Authenticated"));
