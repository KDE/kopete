/*
    kircctcpmessages.h - IRC CTCP messages factory.

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCCTCPMESSAGES_H
#define KIRCCTCPMESSAGES_H

#include "kircmessage.h"

namespace KIrc {
class Socket;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@gmail.com>
 * @author Jason Keirstead <jason@keirstead.org>
 */
namespace CtcpMessages {
/* CTCP commands */
Message CtcpRequestCommand(Socket *socket, QString contact, QString command);
Message action(QString contact, QString message);
//	KIrc::Message dcc(QString, QString, unsigned int port, KIrc::Transfer::Type type);
Message ping(QString target);
KIrc version(QString target);
}
}

#endif // KIRCCTCPMESSAGES_H
