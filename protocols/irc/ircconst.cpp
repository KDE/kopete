/*
    ircconst.cpp - The IRC constants & enums.

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

#include "ircconst.h"

#include <kaboutdata.h>
#include <klocale.h>

using namespace Kopete;

/* The usage of the namespace, instead of the "using" keyword, is intentional.
 * Not using it introduce compiler confusion, and lead to new symbols declaration.
 */

namespace IRC {
/*
const QString Version = I18N_NOOP("Kopete IRC Plugin %1 [http://kopete.kde.org]",
				KGlobal::mainComponent().aboutData()->version());
*/
namespace Config {

//const QLatin1String AUTOSHOWSERVERWINDOW("AutoShowServerWindow");
const QLatin1String CODECMIB("Codec");
const QLatin1String NETWORKNAME("NetworkName");
const QLatin1String NICKNAME("NickName");
const QLatin1String USERNAME("UserName");
const QLatin1String REALNAME("RealName");

} // IRC::Config

namespace Properties {
/*
const ContactPropertyTmpl ChannelTopic(QLatin1String("channelTopic"), i18n("Topic"), QString(), false, true );
const ContactPropertyTmpl ChannelMembers(QLatin1String("channelMembers"), i18n("Members"));
const ContactPropertyTmpl ChannelHomepage(QLatin1String("homePage"), i18n("Home Page"));

//const ContactPropertyTmpl LastSeen(Kopete::Global::IRC::Properties::self()->lastSeen());
const ContactPropertyTmpl UserInfo(QLatin1String("userInfo"), i18n("IRC User"));
const ContactPropertyTmpl Server(QLatin1String("ircServer"), i18n("IRC Server"));
const ContactPropertyTmpl Channels( QLatin1String("ircChannels"), i18n("IRC Channels"));
const ContactPropertyTmpl Hops(QLatin1String("ircHops"), i18n("IRC Hops"));
const ContactPropertyTmpl FullName(QLatin1String("FormattedName"), i18n("Full Name"));
const ContactPropertyTmpl IsIdentified(QLatin1String("identifiedUser"), i18n("User Is Authenticated"));
*/
} // IRC::Properties

} // IRC
