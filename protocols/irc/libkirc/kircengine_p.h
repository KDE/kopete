/*
    kircengine_p.h - IRC Client

    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCENGINE_P_H
#define KIRCENGINE_P_H

#include "kircentity.h"

#include <QMap>

class QTextCodec;

namespace KIRC
{

/**
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 */
class EnginePrivate
{
public:

	QTextCodec *defaultCodec;

	QString host;
	Q_UINT16 port;

//	QUrl serverURL;
//	QUrl currentServerURL;
	QString nickname;
	QString username;
	QString realName;
	QString password;
	bool reqsPassword;
	bool failedNickOnLogin;
	bool useSSL;

	KIRC::EntityPtrList m_entities;
	KIRC::EntityPtr m_server;
	KIRC::EntityPtr m_self;

	QString versionString;
	QString userString;
	QString sourceString;
	QString pendingNick;

	QMap<QString, KIRC::MessageRedirector *> commands;
//	QMap<int, KIRC::MessageRedirector *> numericCommands;
	QMap<QString, KIRC::MessageRedirector *> ctcpQueries;
	QMap<QString, KIRC::MessageRedirector *> ctcpReplies;
};

}

#endif

