/*
    kircclientlisthandler.h - IRC Client List Handler

    Copyright (c) 2008      by Michel Hermier <michel.hermier@wanadoo.fr>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCCLIENTLISTHANDLER_H
#define KIRCCLIENTLISTHANDLER_H

#include "kirchandler.h"
#include "kircmessage.h"

namespace KIrc {
class ClientListHandlerPrivate;

/**
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 */
class KIRCCLIENT_EXPORT ClientListHandler : public KIrc::Handler
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KIrc::ClientListHandler)

public:
    explicit ClientListHandler(Context *context);
    ~ClientListHandler();

private:
//	void LIST(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

    void RPL_LISTSTART(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void RPL_LIST(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void RPL_LISTEND(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void ERR_NOSUCHSERVER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

private:
    Q_DISABLE_COPY(ClientListHandler)

    ClientListHandlerPrivate * const d_ptr;
};
}

#endif
