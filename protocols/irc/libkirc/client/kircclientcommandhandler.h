/*
    kircclientcommandhandler.h - IRC Client Command handler.

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

#ifndef KIRCCLIENTCOMMANDHANDLER_H
#define KIRCCLIENTCOMMANDHANDLER_H

#include "kirccommandhandler.h"

namespace KIrc
{

class Message;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@gmail.com>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class ClientCommandHandler
	: public KIrc::CommandHandler
{
	Q_OBJECT

public:
	explicit ClientCommandHandler(QObject *parent = 0);
	~ClientCommandHandler();

public slots:
	void handleMessage(KIrc::Message msg);

private:
	Q_DISABLE_COPY(ClientCommandHandler)

	class Private;
	Private * const d;
};

}

#endif

